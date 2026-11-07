#include "PolyglotInject.hpp"
#include "EmbbedData.hpp"
#include "SSL10n.hpp"
#include "main.hpp"

#include "GameCompat.hpp"
#include "BGLib/Polyglot/LocalizationAsyncInstaller.hpp"
#include "BGLib/Polyglot/Localization.hpp"
#include "System/Collections/Generic/IList_1.hpp"
#include "System/Collections/Generic/List_1.hpp"
#include "System/Collections/Generic/ICollection_1.hpp"
#include "UnityEngine/TextAsset.hpp"
#include <vector>
#include <variant>
#include <string>
#include <string_view>

/* this is modified from https://github.com/qe201020335/PolyglotInject */

MAKE_HOOK_MATCH(LocalizationInstallerHook, 
                &BGLib::Polyglot::LocalizationAsyncInstaller::LoadResourcesBeforeInstall, 
                void, 
                BGLib::Polyglot::LocalizationAsyncInstaller *self, 
                System::Collections::Generic::IList_1<UnityW<UnityEngine::TextAsset>> * assets, 
                IInstallerRegistry* registry) {
    PaperLogger.debug("Injecting languages for polyglot");

    std::string_view siraLocalizerRes = LangCtrl::siraLocalizerResource();
    if(siraLocalizerRes.size() > 0){
        assets->i___System__Collections__Generic__ICollection_1_T_()->Add(
            UnityEngine::TextAsset::New_ctor( siraLocalizerRes)
        );
    }

    std::vector<PolyglotLanguageTy> toBeAdded {
        PolyglotLanguageTy::Arabic,
        PolyglotLanguageTy::Czech,
        PolyglotLanguageTy::Danish,
        PolyglotLanguageTy::Dutch,
        // PolyglotLanguageTy::French,
        // PolyglotLanguageTy::German,
        PolyglotLanguageTy::Hebrew,
        PolyglotLanguageTy::Hungarian,
        PolyglotLanguageTy::Italian,
        // PolyglotLanguageTy::Japanese,
        // PolyglotLanguageTy::Korean,
        PolyglotLanguageTy::Polish,
        PolyglotLanguageTy::Portuguese_Brazil,
        PolyglotLanguageTy::Romanian,
        PolyglotLanguageTy::Russian,
        PolyglotLanguageTy::Simplified_Chinese,
        // PolyglotLanguageTy::Spanish,
        PolyglotLanguageTy::Swedish,
        PolyglotLanguageTy::Traditional_Chinese,
    };

    for(auto lang : toBeAdded){
        if(!self->____mainPolyglotAsset->___supportedLanguages->Contains(lang))
            self->_mainPolyglotAsset->supportedLanguages->Add(lang);
    }
    
    LocalizationInstallerHook(self, assets, registry);
}


static bool isSimplifiedChinese(StringW serializedName) {
    if (!serializedName) {
        return false;
    }

    std::string name = serializedName;
    std::ranges::transform(name, name.begin(), [](unsigned char c) -> char {
        return c == '-' ? '_' : std::tolower(c);
    });
    return name == "zh_ch" || name == "zh";
}

#if defined(GAME_VER_1_40_8) || defined(GAME_VER_1_37_0)
#include "BGLib/Polyglot/LanguageExtensions.hpp"
// The game saves both Chinese as "zh" and both Portuguese as "pt", which parse back as Simplified Chinese and
// Portuguese. This make sure they are saved correctly.
MAKE_HOOK_MATCH(SerializedNameHook,
                &BGLib::Polyglot::LanguageExtensions::ToSerializedName,
                StringW,
                BGLib::Polyglot::Language lang) {
    using BGLib::Polyglot::Language;
    if (lang == Language::Traditional_Chinese) {
        return "zh-Hant";
    }
    if (lang == Language::Portuguese_Brazil) {
        return "pt-BR";
    }
    return SerializedNameHook(lang);
}
static void installLangFixHook(){
    INSTALL_HOOK(logger, SerializedNameHook)
}
#else

#include "BGLib/Polyglot/LocalizationLanguageExtensions.hpp"

// The game saves Simplified Chinese as "zh_CH" but expects "zh_CN" when parsing it back.
// Anything else starting with "zh" becomes Traditional Chinese.
// 1.40.8 saves "zh" so we should map this to Simplified Chinese too.
MAKE_HOOK_MATCH(SimplifiedChineseParseHook,
                &BGLib::Polyglot::LocalizationLanguageExtensions::ToLocalizationLanguage,
                BGLib::Polyglot::LocalizationLanguage,
                StringW serializedName) {
    PaperLogger.debug("{} triggered!", name());
    if (isSimplifiedChinese(serializedName)) {
        return BGLib::Polyglot::LocalizationLanguage::Simplified_Chinese;
    }
    return SimplifiedChineseParseHook(serializedName);
}
static void installLangFixHook(){
    INSTALL_HOOK(PaperLogger, SimplifiedChineseParseHook);
}

#endif
void LangCtrl::PolyglotInjectInit(){
    PaperLogger.info("Installing hook for polyglot");
    INSTALL_HOOK(PaperLogger, LocalizationInstallerHook);
    installLangFixHook();
}