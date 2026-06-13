#include "PolyglotInject.hpp"
#include "BGLib/Polyglot/Language.hpp"
#include "EmbbedData.hpp"
#include "main.hpp"
#include "BGLib/Polyglot/LocalizationAsyncInstaller.hpp"
#include "BGLib/Polyglot/Language.hpp"
#include "BGLib/Polyglot/Localization.hpp"
#include "UnityEngine/TextAsset.hpp"
#include <vector>
#include <variant>
#include <string>
#include <string_view>

#include "asset.hpp"

/* this is modified from https://github.com/qe201020335/PolyglotInject */

MAKE_HOOK_MATCH(LocalizationInstallerHook, 
                &BGLib::Polyglot::LocalizationAsyncInstaller::LoadResourcesBeforeInstall, 
                void, 
                BGLib::Polyglot::LocalizationAsyncInstaller *self, 
                System::Collections::Generic::IList_1<UnityW<UnityEngine::TextAsset>> * assets, 
                BGLib::AppFlow::Initialization::AsyncInstaller::IInstallerRegistry* registry) {
    PaperLogger.debug("Injecting languages for polyglot");

    std::string_view siraLocalizerRes = LangCtrl::siraLocalizerResource();
    if(siraLocalizerRes.size() > 0){
        assets->i___System__Collections__Generic__ICollection_1_T_()->Add(
            UnityEngine::TextAsset::New_ctor( siraLocalizerRes)
        );
    }

    std::vector<BGLib::Polyglot::Language> toBeAdded {
        BGLib::Polyglot::Language::Arabic,
        BGLib::Polyglot::Language::Czech,
        BGLib::Polyglot::Language::Danish,
        BGLib::Polyglot::Language::Dutch,
        BGLib::Polyglot::Language::French,
        BGLib::Polyglot::Language::German,
        BGLib::Polyglot::Language::Hebrew,
        BGLib::Polyglot::Language::Hungarian,
        BGLib::Polyglot::Language::Italian,
        BGLib::Polyglot::Language::Italian,
        BGLib::Polyglot::Language::Japanese,
        BGLib::Polyglot::Language::Korean,
        BGLib::Polyglot::Language::Polish,
        BGLib::Polyglot::Language::Portuguese_Brazil,
        BGLib::Polyglot::Language::Romanian,
        BGLib::Polyglot::Language::Russian,
        BGLib::Polyglot::Language::Simplified_Chinese,
        BGLib::Polyglot::Language::Spanish,
        BGLib::Polyglot::Language::Swedish,
        BGLib::Polyglot::Language::Traditional_Chinese,
    };

    for(auto lang : toBeAdded){
        if(!self->____mainPolyglotAsset->IsLanguageSupported(lang)){}
            self->_mainPolyglotAsset->supportedLanguages->Add(lang);
    }
    
    LocalizationInstallerHook(self, assets, registry);
}


void LangCtrl::PolyglotInjectInit(){
    PaperLogger.info("Installing hook for polyglot");
    INSTALL_HOOK(PaperLogger, LocalizationInstallerHook);
}