#include "lang_ctrl.hpp"
#include "main.hpp"
#include "sslocalization/shared/SSL10n.hpp"

void SSLCTRL::SyncSelectedLanguage(){
    int opt = getConfig().SelectedLanguage.GetValue();
    if(opt < 0 || opt >= SSLCTRL::L_LASTLANG){
        opt = 0;
    }

    auto lang = (SSLCTRL::LanguageOption)opt;

    if(lang == L_FollowGame){
        // This will not works immediately
        // the sslocalization library only follows language when game set it
        SSL10n::LanguageController::SetFollowGameLanguage(true);
        return;
    }

    SSL10n::LanguageController::SetFollowGameLanguage(false);

    switch ((SSLCTRL::LanguageOption)opt) {
    #define CASE(lang) \
        case SSLCTRL::LanguageOption::L_##lang: \
            SSL10n::LanguageController::SetCurrentLanguage(SSL10n::Language::L_##lang); \
            break;
        FOR_EACH_LANGUAGE(CASE)
    #undef CASE
        default: 
            break;
    }
}
