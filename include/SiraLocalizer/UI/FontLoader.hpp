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
        void OnSceneLoaded();
        void LoadFontAssets();
        void LoadFallbackFonts();
        void LoadFontAsset(UnityEngine::AssetBundle* assetBundle, std::string_view name);
        void ApplyFallbackFonts();
        void AddFallbacksToFont(TMPro::TMP_FontAsset* fontAsset, std::vector<UnityW<TMPro::TMP_FontAsset>> const& fallbacks);

        std::vector<UnityW<TMPro::TMP_FontAsset>> _fallbackFontAssets;
        // Instance ids, because the address of a destroyed font can be reused.
        std::unordered_set<int> _processedFontAssets;
    };
}
