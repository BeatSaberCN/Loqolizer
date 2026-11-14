#include "SiraLocalizer/UI/FontLoader.hpp"

#include "GameCompat.hpp"
#include "main.hpp"

#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include "System/Collections/Generic/List_1.hpp"
#include "TMPro/TMP_Text.hpp"
#include "UnityEngine/FindObjectsSortMode.hpp"
#include "UnityEngine/HideFlags.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Events/UnityAction_2.hpp"
#include "UnityEngine/SceneManagement/LoadSceneMode.hpp"
#include "UnityEngine/SceneManagement/Scene.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "custom-types/shared/delegate.hpp"

#if !defined(GAME_VER_1_40_8)
#include "System/Collections/Generic/Dictionary_2.hpp"
#endif

#include <algorithm>
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

    static std::vector<std::string_view> const kFontNamesToRemove = {
        "NotoSansJP-Medium SDF",
        "NotoSansKR-Medium SDF",
        "SourceHanSansCN-Bold-SDF-Common-1(2k)",
        "SourceHanSansCN-Bold-SDF-Common-2(2k)",
        "SourceHanSansCN-Bold-SDF-Uncommon(2k)",
    };
    static std::vector<FontReplacementStrategy> const kFontReplacementStrategies = {
        {{"Teko-Medium SDF"},
         {"Teko-Medium SDF Latin-1 Supplement", "Oswald-Medium SDF Cyrillic", "NotoSansHebrew-Medium SDF", "SourceHanSans-Medium SDF"}},
        {{"Teko-Bold SDF"},
         {"Teko-Bold SDF Latin-1 Supplement", "Oswald-Bold SDF Cyrillic", "NotoSansHebrew-Bold SDF", "SourceHanSans-Medium SDF"}},
    };

    static bool Contains(std::vector<std::string_view> const& names, std::string const& name) {
        return std::ranges::find(names, name) != names.end();
    }

    void FontLoader::Initialize() {
        // BSML creates its scheduler in its own late_load, and Unity is ready by the next frame.
        BSML::MainThreadScheduler::ScheduleNextFrame([this]() {
            try {
                // Fonts that load with a later scene only get their fallbacks here
                SceneManager::add_sceneLoaded(custom_types::MakeDelegate<Events::UnityAction_2<Scene, LoadSceneMode>*>(
                    std::function([this](Scene, LoadSceneMode) { OnSceneLoaded(); })));

                LoadFontAssets();
            } catch (std::exception const& e) {
                PaperLogger.error("Failed to load fonts: {}", e.what());
            }
        });
    }

    void FontLoader::OnSceneLoaded() {
        try {
            ApplyFallbackFonts();
        } catch (std::exception const& e) {
            PaperLogger.error("Failed to apply fallback fonts: {}", e.what());
        }
    }

    void FontLoader::LoadFontAssets() {
        PaperLogger.info("Loading fonts");

        LoadFallbackFonts();
        ApplyFallbackFonts();
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

    void FontLoader::ApplyFallbackFonts() {
        if (_fallbackFontAssets.empty()) {
            return;
        }

        auto fontAssets = Resources::FindObjectsOfTypeAll<TMP_FontAsset*>();

        for (auto const& strategy : kFontReplacementStrategies) {
            std::vector<UnityW<TMP_FontAsset>> fallbacks;
            for (auto name : strategy.fontNamesToAdd) {
                auto fallback = std::ranges::find_if(_fallbackFontAssets, [name](auto f) { return std::string(f->name) == name; });
                if (fallback != _fallbackFontAssets.end()) {
                    fallbacks.push_back(*fallback);
                }
            }

            for (auto fontAsset : fontAssets) {
                if (!_processedFontAssets.contains(fontAsset->GetInstanceID()) &&
                    Contains(strategy.targetFontNames, std::string(fontAsset->name))) {
                    AddFallbacksToFont(fontAsset, fallbacks);
                }
            }
        }

        // force update any text that has already rendered
        for (auto text : Object::FindObjectsByType(Compat::TypeOf<TMP_Text>(), FindObjectsSortMode::None)) {
            Compat::Cast<TMP_Text>(text.ptr())->SetAllDirty();
        }
    }

    void FontLoader::AddFallbacksToFont(TMP_FontAsset* fontAsset, std::vector<UnityW<TMP_FontAsset>> const& fallbacks) {
        PaperLogger.info("Adding fallbacks to '{}' ({})", fontAsset->name, fontAsset->GetInstanceID());

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

#if !defined(GAME_VER_1_40_8)
        // 1.45.3's TMP caches fallback lookups per font. 1.40.8's has no such cache and no ClearFallbackCharacterTable.
        if (fontAsset->m_CharacterLookupDictionary) {
            fontAsset->ClearFallbackCharacterTable();
        }
#endif

        _processedFontAssets.insert(fontAsset->GetInstanceID());
    }
}
