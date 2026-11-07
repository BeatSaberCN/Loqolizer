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


void LangCtrl::PolyglotInjectInit(){
    PaperLogger.info("Installing hook for polyglot");
    INSTALL_HOOK(PaperLogger, LocalizationInstallerHook);
}