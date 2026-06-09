#include "PolyglotInject.hpp"
#include "BGLib/Polyglot/Language.hpp"
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

std::vector<std::variant<std::string, std::string_view>> pending_assets;

/* this is modified from https://github.com/qe201020335/PolyglotInject */

MAKE_HOOK_MATCH(LocalizationInstallerHook, 
                &BGLib::Polyglot::LocalizationAsyncInstaller::LoadResourcesBeforeInstall, 
                void, 
                BGLib::Polyglot::LocalizationAsyncInstaller *self, 
                System::Collections::Generic::IList_1<UnityW<UnityEngine::TextAsset>> * assets, 
                BGLib::AppFlow::Initialization::AsyncInstaller::IInstallerRegistry* registry) {
    PaperLogger.debug("%s triggered!", name());
    for(auto & asset_text:pending_assets){
        assets->i___System__Collections__Generic__ICollection_1_T_()->Add(UnityEngine::TextAsset::New_ctor(
            std::holds_alternative<std::string_view>(asset_text)?
                std::get<std::string_view>(asset_text):
                std::string_view(std::get<std::string>(asset_text))
        ));
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

    pending_assets.clear();
}


void LangCtrl::PolyglotInjectInit(){
    PaperLogger.info("Installing hook for polyglot");
    INSTALL_HOOK(PaperLogger, LocalizationInstallerHook);

    //TODO: add local/remote asset file
    // pending_assets.push_back(std::string_view(
    //     (const char*)AssetGenerated::polyglot_inject_csv, 
    //     sizeof(AssetGenerated::polyglot_inject_csv)));
}