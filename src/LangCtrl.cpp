#include "LangCtrl.hpp"
#include "main.hpp"
#include "sslocalization/shared/SSL10n.hpp"
#include "UnityEngine/Resources.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "bsml/shared/BSML-Lite/Creation/Buttons.hpp"
#include "bsml/shared/BSML-Lite/Creation/Text.hpp"

#define TEXT_FOLLOW_GAME "FollowGame"

void LangCtrl::DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {    
    static std::vector<std::string_view> languages = {
      TEXT_FOLLOW_GAME,
      #define L(x) #x,
      FOR_EACH_LANGUAGE(L)
      #undef L
    };

    if(firstActivation) {
        // Create a container that has a scroll bar
        UnityEngine::GameObject* container = BSML::Lite::CreateScrollableSettingsContainer(self->get_transform());
        
        int old_config = getConfig().SelectedLanguage.GetValue();
        if(old_config < 0 || old_config >= languages.size())
          old_config = 0;

        BSML::Lite::CreateDropdown(container->get_transform(), SSL10n::Get("LOQOLIZER_SETTHING_LABEL_LANGUAGE"), languages[old_config],  languages, [](StringW str){
          std::string v = str;
          if(v == TEXT_FOLLOW_GAME){
            getConfig().SelectedLanguage.SetValue(LangCtrl::L_FollowGame);
            return;
          }

          #define L(x) if(v == #x) { getConfig().SelectedLanguage.SetValue(LangCtrl::L_##x); return; }
          FOR_EACH_LANGUAGE(L)
          #undef  L
        });

        BSML::Lite::CreateText(container->get_transform(), SSL10n::Get("LOQOLIZER_SETTHING_TEXT_FOLLOW_HINT"), {0,0},{0,5});
        BSML::Lite::CreateText(container->get_transform(), SSL10n::Get("LOQOLIZER_SETTHING_TEXT_KEPT"), {0,0},{0,5});

        BSML::Lite::CreateUIButton(container->get_transform(), SSL10n::Get("LOQOLIZER_SETTHING_APPLY_NOW"), [](){
          LangCtrl::SyncSelectedLanguage();
          UnityEngine::Resources::FindObjectsOfTypeAll<GlobalNamespace::MenuTransitionsHelper*>()->First()->RestartGame(nullptr);
        });
    }
}



void LangCtrl::SyncSelectedLanguage(){
    int opt = getConfig().SelectedLanguage.GetValue();
    if(opt < 0 || opt >= LangCtrl::L_LASTLANG){
        opt = 0;
    }

    auto lang = (LangCtrl::LanguageOption)opt;

    if(lang == L_FollowGame){
        // This will not works immediately
        // the sslocalization library only follows language when game set it
        SSL10n::LanguageController::SetFollowGameLanguage(true);
        return;
    }

    SSL10n::LanguageController::SetFollowGameLanguage(false);

    switch ((LangCtrl::LanguageOption)opt) {
    #define CASE(lang) \
        case LangCtrl::LanguageOption::L_##lang: \
            SSL10n::LanguageController::SetCurrentLanguage(SSL10n::Language::L_##lang); \
            break;
        FOR_EACH_LANGUAGE(CASE)
    #undef CASE
        default: 
            break;
    }
}
