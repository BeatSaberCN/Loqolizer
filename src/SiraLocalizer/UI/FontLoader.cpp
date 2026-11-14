#include "SiraLocalizer/UI/FontLoader.hpp"

#include "GameCompat.hpp"
#include "main.hpp"

#include "lapiz/shared/zenject/Zenjector.hpp"

#include "BGLib/Polyglot/Localization.hpp"
#include "BGLib/Polyglot/LocalizationModel.hpp"
#include "System/Collections/Generic/List_1.hpp"
#include "TMPro/TMP_Text.hpp"
#include "UnityEngine/FindObjectsSortMode.hpp"
#include "UnityEngine/HideFlags.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Events/UnityAction_2.hpp"
#include "UnityEngine/SceneManagement/LoadSceneMode.hpp"
#include "UnityEngine/SceneManagement/Scene.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "Zenject/DiContainer.hpp"
#include "custom-types/shared/delegate.hpp"

#if defined(GAME_VER_1_40_8)
#include "UnityEngine/Font.hpp"
#include "UnityEngine/TextCore/LowLevel/FontEngine.hpp"
#include "UnityEngine/TextCore/LowLevel/FontEngineError.hpp"
#else
#include "System/Collections/Generic/Dictionary_2.hpp"
#endif

#include <algorithm>
#include <array>
#include <set>
#include <string>

using namespace TMPro;
using namespace UnityEngine;
using namespace UnityEngine::SceneManagement;

namespace LangCtrl::SiraLocalizer::UI {
    struct FontReplacementStrategy {
        std::vector<std::string_view> targetFontNames;
        std::vector<std::string_view> fontNamesToAdd;
    };

    // Must match the fileCopies destination in mod.template.json. The bundle can't be embedded in the .so,
    // because AssetBundle.LoadFromMemory and LoadFromStream are stripped from the game.
    static constexpr auto kFontBundlePath = "/sdcard/ModData/com.beatgames.beatsaber/Mods/Loqolizer/fonts.bundle";

    // The faces of SourceHanSans-Medium.ttc cover the same characters, but each draws the Han characters shared between
    // languages (e.g. 关) the way its region does.
    enum CjkFace : int {
        kJapaneseFace = 0,
        kKoreanFace = 1,
        kSimplifiedChineseFace = 2,
        kTraditionalChineseFace = 3,
    };
    // Uses the Japanese face
    static constexpr std::string_view kCjkFontName = "SourceHanSans-Medium SDF";

#if !defined(GAME_VER_1_40_8)
    // 1.45.3's TMP loads the face set in the font asset, so the bundle has one asset per face, indexed by CjkFace.
    static constexpr std::array<std::string_view, 4> kCjkFontNames = {
        kCjkFontName,
        "SourceHanSansK-Medium SDF",
        "SourceHanSansSC-Medium SDF",
        "SourceHanSansTC-Medium SDF",
    };
#endif

    static std::vector<std::string_view> const kFontNamesToRemove = {
        "NotoSansJP-Medium SDF",
        "NotoSansKR-Medium SDF",
        "SourceHanSansCN-Bold-SDF-Common-1(2k)",
        "SourceHanSansCN-Bold-SDF-Common-2(2k)",
        "SourceHanSansCN-Bold-SDF-Uncommon(2k)",
    };
    static std::vector<FontReplacementStrategy> const kFontReplacementStrategies = {
        {{"Teko-Medium SDF"},
         {"Teko-Medium SDF Latin-1 Supplement", "Oswald-Medium SDF Cyrillic", "NotoSansHebrew-Medium SDF", kCjkFontName}},
        {{"Teko-Bold SDF"},
         {"Teko-Bold SDF Latin-1 Supplement", "Oswald-Bold SDF Cyrillic", "NotoSansHebrew-Bold SDF", kCjkFontName}},
    };

    static bool Contains(std::vector<std::string_view> const& names, std::string const& name) {
        return std::ranges::find(names, name) != names.end();
    }

    static CjkFace GetCjkFace() {
        // Polyglot supports Chinese from the start thanks to LocalizationInstallerHook, so this is the language from the settings
        switch (BGLib::Polyglot::Localization::get_Instance()->get_SelectedLanguage()) {
            case PolyglotLanguageTy::Korean:
                return kKoreanFace;
            case PolyglotLanguageTy::Simplified_Chinese:
                return kSimplifiedChineseFace;
            case PolyglotLanguageTy::Traditional_Chinese:
                return kTraditionalChineseFace;
            default:
                return kJapaneseFace;
        }
    }

    static void ClearFallbackCharacterTable(TMP_FontAsset* fontAsset) {
#if !defined(GAME_VER_1_40_8)
        // 1.45.3's TMP caches fallback lookups per font. 1.40.8's has no such cache and no ClearFallbackCharacterTable.
        if (fontAsset->m_CharacterLookupDictionary) {
            fontAsset->ClearFallbackCharacterTable();
        }
#endif
    }
}

#if defined(GAME_VER_1_40_8)
// 1.40.8's TMP always loads the first face of a font file, so this loads the face for the language instead.
static int cjkSourceFontId = 0;
static int cjkFaceIndex = LangCtrl::SiraLocalizer::UI::kJapaneseFace;

MAKE_HOOK_MATCH(LoadFontFaceHook,
                static_cast<UnityEngine::TextCore::LowLevel::FontEngineError (*)(UnityEngine::Font*, int32_t)>(
                    &UnityEngine::TextCore::LowLevel::FontEngine::LoadFontFace),
                UnityEngine::TextCore::LowLevel::FontEngineError,
                UnityEngine::Font* font,
                int32_t pointSize) {
    if (font && cjkSourceFontId != 0 && font->GetInstanceID() == cjkSourceFontId) {
        return UnityEngine::TextCore::LowLevel::FontEngine::LoadFontFace(font, pointSize, cjkFaceIndex);
    }
    return LoadFontFaceHook(font, pointSize);
}
#endif

namespace LangCtrl::SiraLocalizer::UI {
    void FontLoader::Initialize() {
#if defined(GAME_VER_1_40_8)
        INSTALL_HOOK(PaperLogger, LoadFontFaceHook);
#endif

        // The menu is installed after the game has set up Polyglot and its settings, both at startup and after a soft
        // restart, which is how the language changes
        Lapiz::Zenject::Zenjector::Get()->Install(Lapiz::Zenject::Location::Menu, [this](::Zenject::DiContainer*) { OnMenuInstalled(); });
    }

    void FontLoader::OnMenuInstalled() {
        try {
            if (!_sceneLoadedSubscribed) {
                // Fonts that load with a later scene only get their fallbacks here
                SceneManager::add_sceneLoaded(custom_types::MakeDelegate<Events::UnityAction_2<Scene, LoadSceneMode>*>(
                    std::function([this](Scene, LoadSceneMode) { OnSceneLoaded(); })));
                _sceneLoadedSubscribed = true;
            }

            // The fonts stay loaded across soft restarts, like the game's fonts they're added to
            if (_fallbackFontAssets.empty()) {
                PaperLogger.info("Loading fonts");
                LoadFallbackFonts();
            }

            SetCjkFace(GetCjkFace());
            ApplyFallbackFonts();
        } catch (std::exception const& e) {
            PaperLogger.error("Failed to load fonts: {}", e.what());
        }
    }

    void FontLoader::OnSceneLoaded() {
        try {
            ApplyFallbackFonts();
        } catch (std::exception const& e) {
            PaperLogger.error("Failed to apply fallback fonts: {}", e.what());
        }
    }

    void FontLoader::LoadFallbackFonts() {
        UnityW<AssetBundle> assetBundle = AssetBundle::LoadFromFile(kFontBundlePath);

        if (!assetBundle) {
            PaperLogger.error("Failed to load fonts asset bundle; some characters may not display as expected");
            return;
        }

        std::set<std::string_view> fontNames;
        for (auto const& strategy : kFontReplacementStrategies) {
            fontNames.insert(strategy.fontNamesToAdd.begin(), strategy.fontNamesToAdd.end());
        }
#if !defined(GAME_VER_1_40_8)
        fontNames.insert(kCjkFontNames.begin(), kCjkFontNames.end());
#endif

        for (auto fontName : fontNames) {
            LoadFontAsset(assetBundle, fontName);
        }

        assetBundle->Unload(false);
    }

    void FontLoader::LoadFontAsset(AssetBundle* assetBundle, std::string_view name) {
        UnityW<Object> asset = assetBundle->LoadAsset(std::string(name), Compat::TypeOf<TMP_FontAsset>());

        if (!asset) {
            PaperLogger.error("Font '{}' could not be loaded; some characters may not display as expected", name);
            return;
        }

        PaperLogger.info("Font '{}' loaded successfully", name);

        // Resources.UnloadUnusedAssets can't see this list either, and the fonts may not be in a fallback table yet.
        asset->hideFlags = HideFlags::DontUnloadUnusedAsset;
        _fallbackFontAssets.emplace_back(Compat::Cast<TMP_FontAsset>(asset.ptr()));
    }

    TMP_FontAsset* FontLoader::FindFallbackFontAsset(std::string_view name) {
        auto fallback = std::ranges::find_if(_fallbackFontAssets, [name](auto f) { return f && std::string(f->name) == name; });
        return fallback != _fallbackFontAssets.end() ? fallback->ptr() : nullptr;
    }

    void FontLoader::SetCjkFace(int face) {
#if defined(GAME_VER_1_40_8)
        _cjkFontAsset = FindFallbackFontAsset(kCjkFontName);
        if (!_cjkFontAsset || !_cjkFontAsset->sourceFontFile || face == cjkFaceIndex) {
            return;
        }

        PaperLogger.info("Using face {} of '{}'", face, _cjkFontAsset->sourceFontFile->name);

        cjkSourceFontId = _cjkFontAsset->sourceFontFile->GetInstanceID();
        cjkFaceIndex = face;
        // The characters already in the font asset come from the previous face
        _cjkFontAsset->ClearFontAssetData(false);
#else
        _cjkFontAsset = FindFallbackFontAsset(kCjkFontNames[face]);
        if (!_cjkFontAsset) {
            _cjkFontAsset = FindFallbackFontAsset(kCjkFontName);
        }
#endif
    }

    void FontLoader::ApplyFallbackFonts() {
        // Nothing to add before the menu has loaded
        if (_fallbackFontAssets.empty()) {
            return;
        }

        auto fontAssets = Resources::FindObjectsOfTypeAll<TMP_FontAsset*>();

        for (auto const& strategy : kFontReplacementStrategies) {
            std::vector<UnityW<TMP_FontAsset>> fallbacks;
            for (auto name : strategy.fontNamesToAdd) {
                if (auto fallback = name == kCjkFontName ? static_cast<TMP_FontAsset*>(_cjkFontAsset) : FindFallbackFontAsset(name)) {
                    fallbacks.emplace_back(fallback);
                }
            }

            for (auto fontAsset : fontAssets) {
                if (!Contains(strategy.targetFontNames, std::string(fontAsset->name))) {
                    continue;
                }

                if (!_processedFontAssets.contains(fontAsset->GetInstanceID())) {
                    AddFallbacksToFont(fontAsset, fallbacks);
                }
#if !defined(GAME_VER_1_40_8)
                else {
                    // The game's fonts keep their fallbacks across soft restarts, which is how the language changes
                    ReplaceCjkFallback(fontAsset, _cjkFontAsset);
                }
#endif
            }
        }

        // force update any text that has already rendered
        for (auto text : Object::FindObjectsByType(Compat::TypeOf<TMP_Text>(), FindObjectsSortMode::None)) {
            Compat::Cast<TMP_Text>(text.ptr())->SetAllDirty();
        }
    }

    void FontLoader::AddFallbacksToFont(TMP_FontAsset* fontAsset, std::vector<UnityW<TMP_FontAsset>> const& fallbacks) {
        std::string fallbackNames;
        for (auto fallback : fallbacks) {
            fallbackNames += (fallbackNames.empty() ? "" : ", ") + std::string(fallback->name);
        }
        PaperLogger.info("Adding fallbacks to '{}' ({}): {}", fontAsset->name, fontAsset->GetInstanceID(), fallbackNames);

        auto fallbackFontAssetTable = fontAsset->fallbackFontAssetTable;

        for (int i = fallbackFontAssetTable->Count - 1; i >= 0; i--) {
            auto fallback = fallbackFontAssetTable->get_Item(i);
            if (fallback && Contains(kFontNamesToRemove, std::string(fallback->name))) {
                fallbackFontAssetTable->RemoveAt(i);
            }
        }

        for (int i = 0; i < fallbacks.size(); i++) {
            fallbackFontAssetTable->Insert(i, fallbacks[i]);
        }

        ClearFallbackCharacterTable(fontAsset);

        _processedFontAssets.insert(fontAsset->GetInstanceID());
    }

#if !defined(GAME_VER_1_40_8)
    void FontLoader::ReplaceCjkFallback(TMP_FontAsset* fontAsset, TMP_FontAsset* cjkFontAsset) {
        if (!cjkFontAsset) {
            return;
        }

        auto fallbackFontAssetTable = fontAsset->fallbackFontAssetTable;

        for (int i = 0; i < fallbackFontAssetTable->Count; i++) {
            auto fallback = fallbackFontAssetTable->get_Item(i);
            if (!fallback || fallback.ptr() == cjkFontAsset || std::ranges::find(kCjkFontNames, std::string(fallback->name)) == kCjkFontNames.end()) {
                continue;
            }

            PaperLogger.info("Switching CJK fallback of '{}' ({}) from '{}' to '{}'", fontAsset->name, fontAsset->GetInstanceID(), fallback->name, cjkFontAsset->name);

            fallbackFontAssetTable->set_Item(i, cjkFontAsset);
            ClearFallbackCharacterTable(fontAsset);
        }
    }
#endif
}
