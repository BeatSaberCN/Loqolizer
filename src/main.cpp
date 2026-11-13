#include "main.hpp"

#include "EmbbedData.hpp"
#include "GameHooks.hpp"
#include "HMUI/ViewController.hpp"
#include "GameCompat.hpp"
#include "PolyglotInject.hpp"
#include "SSL10n.hpp"
#include "LangCtrl.hpp"
#include "bsml/shared/BSML/Settings/BSMLSettings.hpp"
#include "scotland2/shared/loader.hpp"
#include "modconfig.hpp"
#include "SSL10nGenerated.hpp"
#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSML/Settings/BSMLSettings.hpp"
#include "bsml/shared/BSML/Settings/SettingsMenu.hpp"

#include "flamingo/shared/installer.hpp"

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};
// Stores the ID and version of our mod, and is sent to
// the modloader upon startup

static void SkipPolyglotInjectLateLoad() noexcept {}

static void SkipsPolyglotInjectMod() {
  for (const auto& mod : modloader::get_loaded()) {
    if (mod.path.filename() != "libPolyglotInject.so" || !mod.late_loadFn) {
      continue;
    }

    auto* target = reinterpret_cast<void*>(*mod.late_loadFn);
    auto* replacement = reinterpret_cast<void*>(&SkipPolyglotInjectLateLoad);
    const bool installed = flamingo::Install(flamingo::HookInfo{replacement, target, nullptr}).has_value();
    if (installed) {
      PaperLogger.info("Disabled standalone PolyglotInject late_load");
    } else {
      PaperLogger.warn("Failed to disable standalone PolyglotInject late_load");
    }
    return;
  }
}

// Called at the early stages of game loading
MOD_EXTERN_FUNC void setup(CModInfo *info) noexcept {
  *info = modInfo.to_c();

  getConfig().Init(modInfo);

  // File logging
  Paper::Logger::RegisterFileContextId(PaperLogger.tag);

  SkipsPolyglotInjectMod();

  SSL10n::LanguageController::SetCurrentLanguage((SSL10n::Language)getConfig().LastGameLanguage.GetValue());

  PaperLogger.info("Completed setup!");
}

// Called later on in the game loading - a good time to install function hooks
MOD_EXTERN_FUNC void late_load() noexcept {
  i2c::functions::initialize();

  PaperLogger.info("Install keys");

  SSL10nGen::LoadCsv();

  PaperLogger.info("Init Embbed Data");
  LangCtrl::InitEmbbedData();

  PaperLogger.info("Register BSML Menu");

  BSML::Init();

  static SafePtrBSMLSettingsMenuTy menu = BSML::SettingsMenu::Make_new(SSL10nGen::STR::SETTHING_MENU_TITLE(), LangCtrl::DidActivate, false);
  if (BSML::BSMLSettings::get_instance()->TryAddSettingsMenu(menu.ptr())) {
    #if defined(GAME_VER_1_40_8)
      SSL10n::OnLanguageChangeCallback.addCallback([](){
    #else
      SSL10n::OnLanguageChangeCallback.add([](){
    #endif
        menu->text = SSL10nGen::STR::SETTHING_MENU_TITLE();
      });
  }else{
    PaperLogger.info("Menu init failed");
  }

  // TODO: figure out why I did this again...
  BSML::Register::RegisterSettingsMenu(SSL10nGen::STR::SETTHING_MENU_TITLE(),LangCtrl::DidActivate, false);

  PaperLogger.info("Init PolyglotInject");

  if(getConfig().EnableGameLocalize.GetValue()){
    LangCtrl::PolyglotInjectInit();
  }else{
    PaperLogger.info("Game Localize not enabled.");
  }

  PaperLogger.info("Init GameHooks...");

  LangCtrl::GameHooks::Init();

  PaperLogger.info("Done");

  LangCtrl::is_initialized = true;
}
