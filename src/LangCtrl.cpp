#include "LangCtrl.hpp"
#include "SSL10n.hpp"
#include "System/Object.hpp"
#include "bsml/shared/BSML-Lite/Creation/Lists.hpp"
#include "bsml/shared/BSML-Lite/Creation/Misc.hpp"
#include "bsml/shared/BSML-Lite/Creation/Settings.hpp"
#include "bsml/shared/BSML/Components/CustomListTableData.hpp"
#include "main.hpp"
#include "SSL10nGenerated.hpp"
#include "UnityEngine/Resources.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "bsml/shared/BSML-Lite/Creation/Buttons.hpp"
#include "bsml/shared/BSML-Lite/Creation/Text.hpp"
#include "EmbbedData.hpp"
#include "modconfig.hpp"
#include <fmt/format.h>

#define TEXT_FOLLOW_GAME "FollowGame"

std::set<std::string> LangCtrl::loadedResourceMd5;

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
        
        BSML::Lite::AddHoverHint(BSML::Lite::CreateToggle(container->get_transform(), SSL10nGen::STR::SETTHING_ENABLE_GAME_LOCALIZE(), getConfig().EnableGameLocalize.GetValue(), [](bool v){
           getConfig().EnableGameLocalize.SetValue(v); 
        }), SSL10nGen::STR::SETTHING_ENABLE_GAME_LOCALIZE_HINT());

        int old_config = getConfig().SelectedLanguage.GetValue();
        if(old_config < 0 || old_config >= languages.size())
          old_config = 0;

        auto dropdown = BSML::Lite::CreateDropdown(container->get_transform(), SSL10nGen::STR::SETTHING_LABEL_LANGUAGE(), languages[old_config],  languages, [](StringW str){
          std::string v = str;
          if(v == TEXT_FOLLOW_GAME){
            getConfig().SelectedLanguage.SetValue(LangCtrl::L_FollowGame);
            return;
          }

          #define L(x) if(v == #x) { getConfig().SelectedLanguage.SetValue(LangCtrl::L_##x); return; }
          FOR_EACH_LANGUAGE(L)
          #undef  L
        });

        dropdown->formatter = [](System::Object* obj)->StringW{
            if(!obj) return "null";
            std::string ret = obj->ToString();
            if(ret == "FollowGame")
                return SSL10nGen::STR::FOLLOW_GAME();
            return ret;
        };
        dropdown->UpdateChoices();
        dropdown->UpdateState();

        BSML::Lite::CreateText(container->get_transform(), SSL10nGen::STR::SETTHING_HINT_RESTART_REQUIRED(), {0,0},{0,5});
        BSML::Lite::CreateText(container->get_transform(), SSL10nGen::STR::SETTHING_TEXT_KEPT(), {0,0},{0,5});

        BSML::Lite::CreateUIButton(container->get_transform(), SSL10nGen::STR::SETTHING_APPLY_NOW(), [](){
          LangCtrl::SyncSelectedLanguage();
          UnityEngine::Resources::FindObjectsOfTypeAll<GlobalNamespace::MenuTransitionsHelper*>()->First()->RestartGame(nullptr);
        });

        BSML::Lite::CreateText(container->get_transform(), SSL10nGen::STR::SETTHING_EMBBED_DATA_TITLE(),{0,0},{0,5});
        auto embbedDataList = BSML::Lite::CreateScrollableList(container->get_transform(),{0, 0}, {55.0f, 30.0f});
        for(auto & data : embbedData){
            bool loaded = false;
            for(auto & ver:data.second.datas){
                if(loadedResourceMd5.contains(ver.md5))
                    loaded = true;
            }
            std::string desc = loaded ? SSL10nGen::STR::TRANSLATE_LOADED() : SSL10nGen::STR::TRANSLATE_NOT_LOADED();
            // std::string desc = std::string("(") + data.second.version + ")";
            auto item = BSML::CustomCellInfo::New_ctor();
            item->text = SSL10n::GetOptional(fmt::format("MOD_META_{}_NAME", data.second.modId)).value_or(data.second.modId);
            item->subText = desc;
            embbedDataList->data.push_back(item);
        }
        embbedDataList->tableView->ReloadData();
    }
}



void LangCtrl::SyncSelectedLanguage(){
    int opt = getConfig().SelectedLanguage.GetValue();
    if(opt < 0 || opt >= LangCtrl::L_LASTLANG){
        opt = 0;
    }

    auto lang = (LangCtrl::LanguageOption)opt;
    PaperLogger.info("Synchronize language to {}", opt);
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
