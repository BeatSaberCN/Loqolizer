#include "PolyglotInject.hpp"
#include "main.hpp"
#include "BGLib/Polyglot/LocalizationAsyncInstaller.hpp"
#include "BGLib/Polyglot/Language.hpp"
#include "BGLib/Polyglot/Localization.hpp"
#include "UnityEngine/TextAsset.hpp"
#include <vector>

std::vector<std::string> pending_assets;

MAKE_HOOK_MATCH(LocalizationInstallerHook, 
                &BGLib::Polyglot::LocalizationAsyncInstaller::LoadResourcesBeforeInstall, 
                void, 
                BGLib::Polyglot::LocalizationAsyncInstaller *self, 
                System::Collections::Generic::IList_1<UnityW<UnityEngine::TextAsset>> * assets, 
                BGLib::AppFlow::Initialization::AsyncInstaller::IInstallerRegistry* registry) {
    PaperLogger.debug("%s triggered!", name());
    for(auto & asset_text:pending_assets){
        assets->i___System__Collections__Generic__ICollection_1_T_()->Add(UnityEngine::TextAsset::New_ctor(asset_text));
    }
    self->_mainPolyglotAsset->supportedLanguages->Add(BGLib::Polyglot::Language::Simplified_Chinese);
    LocalizationInstallerHook(self, assets, registry);
}


void LangCtrl::PolyglotInjectInit(){
    PaperLogger.info("Installing hook for polyglot");
    INSTALL_HOOK(PaperLogger, LocalizationInstallerHook);

    //TODO: add local/remote asset file
}