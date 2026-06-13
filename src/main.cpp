#include "main.hpp"

#include "EmbbedData.hpp"
#include "HMUI/ViewController.hpp"
#include "PolyglotInject.hpp"
#include "SSL10n.hpp"
#include "beatsaber-hook/shared/utils/typedefs-string.hpp"
#include "LangCtrl.hpp"
#include "bsml/shared/BSML/Settings/BSMLSettings.hpp"
#include "scotland2/shared/modloader.h"
#include "modconfig.hpp"
#include "SSL10nGenerated.hpp"
#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSML/Settings/BSMLSettings.hpp"
#include "bsml/shared/BSML/Settings/SettingsMenu.hpp"

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};
// Stores the ID and version of our mod, and is sent to
// the modloader upon startup


// Called at the early stages of game loading
MOD_EXTERN_FUNC void setup(CModInfo *info) noexcept {
  *info = modInfo.to_c();

  getConfig().Init(modInfo);

  // File logging
  Paper::Logger::RegisterFileContextId(PaperLogger.tag);

  PaperLogger.info("Completed setup!");

  LangCtrl::SyncSelectedLanguage();
}

// Called later on in the game loading - a good time to install function hooks
MOD_EXTERN_FUNC void late_load() noexcept {
  il2cpp_functions::Init();

  PaperLogger.info("Install keys");

  SSL10nGen::LoadCsv();

  PaperLogger.info("Init Embbed Data");
  LangCtrl::InitEmbbedData();

  PaperLogger.info("Register BSML Menu");

  BSML::Init();

  static SafePtr<BSML::SettingsMenu> menu = BSML::SettingsMenu::Make_new(SSL10nGen::STR::SETTHING_MENU_TITLE(), LangCtrl::DidActivate, false);
  if (!BSML::BSMLSettings::get_instance()->TryAddSettingsMenu(&*menu)) {
    PaperLogger.info("Menu init failed");
  }else{
    SSL10n::OnLanguageChangeCallback.addCallback([](){
      menu->text = SSL10nGen::STR::SETTHING_MENU_TITLE();
    });
  }


  BSML::Register::RegisterSettingsMenu(SSL10nGen::STR::SETTHING_MENU_TITLE(),LangCtrl::DidActivate, false);

  PaperLogger.info("Init PolyglotInject");

  if(getConfig().EnableGameLocalize.GetValue()){
    LangCtrl::PolyglotInjectInit();
  }else{
    PaperLogger.info("Game Localize not enabled.");
  }

  PaperLogger.info("Done");
}