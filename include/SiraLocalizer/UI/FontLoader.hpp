#pragma once

// Port of SiraLocalizer's FontLoader (SiraLocalizer/UI/FontLoader.cs), from the PolyglotInject mod.

#include "TMPro/TMP_FontAsset.hpp"
#include "UnityEngine/AssetBundle.hpp"

#include <string_view>
#include <unordered_set>
#include <vector>

namespace LangCtrl::SiraLocalizer::UI {
    class FontLoader {
       public:
        void Initialize();

       private:
        void OnMenuInstalled();
        void OnSceneLoaded();
        void LoadFallbackFonts();
        void LoadFontAsset(UnityEngine::AssetBundle* assetBundle, std::string_view name);
        TMPro::TMP_FontAsset* FindFallbackFontAsset(std::string_view name);
        void SetCjkFace(int face);
        void ApplyFallbackFonts();
        void AddFallbacksToFont(TMPro::TMP_FontAsset* fontAsset, std::vector<UnityW<TMPro::TMP_FontAsset>> const& fallbacks);
#if !defined(GAME_VER_1_40_8)
        void ReplaceCjkFallback(TMPro::TMP_FontAsset* fontAsset, TMPro::TMP_FontAsset* cjkFontAsset);
#endif

        bool _sceneLoadedSubscribed = false;
        std::vector<UnityW<TMPro::TMP_FontAsset>> _fallbackFontAssets;
        // Instance ids, because the address of a destroyed font can be reused.
        std::unordered_set<int> _processedFontAssets;
        // The CJK fallback for the language, set when the menu is installed
        UnityW<TMPro::TMP_FontAsset> _cjkFontAsset;
    };
}
