#include "main.hpp"

#include "HMUI/ViewController.hpp"
#include "PolyglotInject.hpp"
#include "beatsaber-hook/shared/utils/typedefs-string.hpp"
#include "LangCtrl.hpp"
#include "scotland2/shared/modloader.h"
#include "modconfig.hpp"
#include "sslocalization/shared/SSL10n.hpp"
#include "bsml/shared/BSML.hpp"
#include "EmbbedKeys.hpp"

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

  LangCtrl::InitEmbbedKeys();

  PaperLogger.info("Register BSML Menu");

  BSML::Init();
  BSML::Register::RegisterSettingsMenu(SSL10n::Get("LOQOLIZER_SETTHING_MENU_TITLE"),LangCtrl::DidActivate, false);

  PaperLogger.info("Init PolyglotInject");

  LangCtrl::PolyglotInjectInit();

  PaperLogger.info("Done");
}