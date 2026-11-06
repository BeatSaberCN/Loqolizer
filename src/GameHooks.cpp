#include "GameHooks.hpp"
#include "LangCtrl.hpp"
#include "SSL10n.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "main.hpp"

#include "BGLib/Polyglot/Language.hpp"
#include "BGLib/Polyglot/ILocalize.hpp"
#include "BGLib/Polyglot/Localization.hpp"
#include "BGLib/Polyglot/LocalizationModel.hpp"
#include "BGLib/Polyglot/LocalizationImporter.hpp"
#include "modconfig.hpp"

static bool SyncPolyglotLanguageFromPolyglot(::BGLib::Polyglot::Language lang) {
    auto currentSelected = LangCtrl::getGameLanguage(lang);
    if(currentSelected.has_value()){
        SSL10n::LanguageController::SetCurrentLanguage(currentSelected.value());
        getConfig().LastGameLanguage.SetValue(currentSelected.value());
        return true;
    }
    return false;
}

// sync game language settings
MAKE_HOOK_MATCH(Localization_SetSelectLang, &BGLib::Polyglot::LocalizationModel::set_SelectedLanguage, void,
                ::BGLib::Polyglot::LocalizationModel *self, ::BGLib::Polyglot::Language lang) {
    Localization_SetSelectLang(self, lang);

    if (getConfig().SelectedLanguage.GetValue() == LangCtrl::L_FollowGame) {
        PaperLogger.info("On localize executed");
        if (SyncPolyglotLanguageFromPolyglot(lang)) {
            PaperLogger.info("Language has been updated to {}.", (int)SSL10n::GetCurrentLanguage());
        }
    } else {
        PaperLogger.info("Not follow game language");
    }
}

void LangCtrl::GameHooks::Init() {
    INSTALL_HOOK(PaperLogger, Localization_SetSelectLang);
}