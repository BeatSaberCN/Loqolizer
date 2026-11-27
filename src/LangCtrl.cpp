#include "LangCtrl.hpp"
#include "SSL10n.hpp"
#include "System/Object.hpp"
#include "UnityEngine/Color.hpp"
#include "bsml/shared/BSML-Lite/Creation/Lists.hpp"
#include "bsml/shared/BSML-Lite/Creation/Misc.hpp"
#include "bsml/shared/BSML-Lite/Creation/Settings.hpp"
#include "UnityEngine/Resources.hpp"
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
#include <optional>
#include <sstream>
#include <string>
#include "GameCompat.hpp"
#include "BGLib/Polyglot/Localization.hpp"
#include "BGLib/Polyglot/LocalizationModel.hpp"
#include "UnityEngine/UI/ContentSizeFitter.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Component.hpp"
#include "HMUI/CurvedTextMeshPro.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "protobuf/snapshot.pb.h"
#include "Web.hpp"
#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "bsml/shared/BSML-Lite/Creation/Image.hpp"
#include "bsml/shared/Helpers/utilities.hpp"
#include <algorithm>
#include <cmath>
#define TEXT_FOLLOW_GAME "FollowGame"


DECLARE_CLASS_CODEGEN(LangCtrl, MenuUpdater, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_FIELD(HMUI::ViewController*, owner);
    DECLARE_INSTANCE_FIELD(float, elapsed);
    DECLARE_INSTANCE_FIELD(HMUI::CurvedTextMeshPro*, statusText);
    DECLARE_INSTANCE_FIELD(HMUI::CurvedTextMeshPro*, displayMessageText);
    DECLARE_INSTANCE_FIELD(HMUI::CurvedTextMeshPro*, progressText);
    DECLARE_INSTANCE_FIELD(HMUI::CurvedTextMeshPro*, translationsText);
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, progressFill);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, refreshButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, downloadButton);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, Refresh);
};

DEFINE_TYPE(LangCtrl, MenuUpdater);

void LangCtrl::MenuUpdater::Update() {
    if (!owner || !owner->get_isActivated()) {
        elapsed = 0;
        return;
    }
    elapsed += UnityEngine::Time::get_unscaledDeltaTime();
    if (elapsed >= 1.0f) {
        elapsed = 0;
        Refresh();
    }
}

void LangCtrl::MenuUpdater::Refresh() {
    // Leave the scroll layout untouched when the displayed contents are unchanged.
    auto setTextIfChanged = [](HMUI::CurvedTextMeshPro* text, const std::string& value) {
        if (static_cast<std::string>(text->get_text()) != value)
            text->set_text(value);
    };
    auto& manager = Web::NetDataManager::instance;
    float progress = manager.downloadingProgress;
    progress = std::isfinite(progress) ? std::clamp(progress, 0.0f, 1.0f) : 0.0f;
    setTextIfChanged(statusText, "Status: " + (manager.statusString.empty() ? std::string("Idle") : manager.statusString));
    setTextIfChanged(progressText, fmt::format("{:.0f}%", progress * 100));
    auto fillRect = progressFill->get_rectTransform();
    if (fillRect->get_anchorMax().x != progress)
        fillRect->set_anchorMax({progress, 1});
    bool canRefresh = !manager.IsBusy();
    canRefresh = false;
    if (refreshButton->get_interactable() != canRefresh)
        refreshButton->set_interactable(canRefresh);
    bool canDownload = manager.CanDownload();
    canDownload = false;
    if (downloadButton->get_interactable() != canDownload)
        downloadButton->set_interactable(canDownload);

    std::string message;
    if (activated_snapshot && activated_snapshot->has_displaymessage()) {
        const auto& information = activated_snapshot->displaymessage();
        if (information.position() == SnapshotInformation::SI_DT_IN_MENU)
            message = information.message();
    }
    setTextIfChanged(displayMessageText, message);
    auto messageObject = displayMessageText->get_gameObject();
    if (messageObject->get_activeSelf() != !message.empty())
        messageObject->SetActive(!message.empty());

    std::stringstream translations;
    if (activated_snapshot) {
        for (const auto& [id, data] : activated_snapshot->mods()) {
            auto loaded = loadedMods.find(id);
            bool isLoaded = loaded != loadedMods.end();
            auto name = SSL10n::GetOptional(fmt::format("QMOD_META_{}_NAME", data.id())).value_or(data.name());
            translations << (isLoaded ? "<color=#00FF00>" : "<color=#808080>");
            translations << name;
            translations << (isLoaded ? SSL10nGen::STR::TRANSLATE_LOADED() : SSL10nGen::STR::TRANSLATE_NOT_LOADED());
            if (isLoaded) translations << fmt::format("({})", loaded->second);
            translations << "</color>\n";
        }

        for (const auto &pcmod : loadedPcMods){
            translations << "<color=#00FF00>" << pcmod << " (PC)" << "</color>\n";
        }
    }
    setTextIfChanged(translationsText, translations.str());
}

bool LangCtrl::is_initialized = false;

std::optional<SSL10n::Language> LangCtrl::getGameLanguage(PolyglotLanguageTy language){
    switch(language){
        #define CASE(lang) \
            case PolyglotLanguageTy::lang:\
                return SSL10n::Language::L_##lang;
        FOR_EACH_LANGUAGE(CASE)
        #undef CASE

        default:
            break;
    }
    return std::nullopt;
}

std::optional<SSL10n::Language> LangCtrl::getGameLanguage(){
    auto instance = BGLib::Polyglot::Localization::get_Instance();
    if(instance){
        return getGameLanguage(instance->get_SelectedLanguage());
    }
    return std::nullopt;
}

void LangCtrl::DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {    
    static std::vector<std::string_view> languages = {
      TEXT_FOLLOW_GAME,
      #define L(x) #x,
      FOR_EACH_LANGUAGE(L)
      #undef L
    };

    if(firstActivation) {
        auto updater = self->get_gameObject()->AddComponent<LangCtrl::MenuUpdater*>();
        updater->owner = self;
        updater->elapsed = 0;

        // Create a container that has a scroll bar
        UnityEngine::GameObject* container = BSML::Lite::CreateScrollableSettingsContainer(self->get_transform());

        int old_config = getConfig().SelectedLanguage.GetValue();
        if(old_config < 0 || old_config >= languages.size()){
          old_config = 0;
          getConfig().SelectedLanguage.SetValue(0);
        }
        
        auto dropdown = BSML::Lite::CreateDropdown(container->get_transform(), SSL10nGen::STR::SETTHING_LABEL_LANGUAGE(), languages[old_config],  languages, [](StringW str){
          std::string v = str;
          if(v == TEXT_FOLLOW_GAME){
            getConfig().SelectedLanguage.SetValue(LangCtrl::L_FollowGame);
            auto gameLang = getGameLanguage();
            if(gameLang.has_value()){
                getConfig().LastGameLanguage.SetValue(gameLang.value());
            }
            return;
          }

          #define L(x) if(v == #x) { getConfig().LastGameLanguage.SetValue(SSL10n::L_##x); getConfig().SelectedLanguage.SetValue(LangCtrl::L_##x); return; }
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


        auto gameLang = getGameLanguage().value_or(SSL10n::Language::L_English);
        std::optional<std::string> sira_game_translator = SSL10n::GetOptional("LANGUAGE_CONTRIBUTORS", gameLang, false);
        if(sira_game_translator.has_value() && gameLang != SSL10n::L_English){
            auto text = BSML::Lite::CreateText(container->get_transform(),
                SSL10nGen::FMT::GAME_TRANSLATED_BY(sira_game_translator.value()), {0,0}, {70,0});
            text->set_enableWordWrapping(true);
            auto layout = text->get_gameObject()->GetComponent<UnityEngine::UI::LayoutElement*>();
            layout->set_preferredWidth(70);

            auto filter = 
            text->get_gameObject()
                ->AddComponent<UnityEngine::UI::ContentSizeFitter*>();
            filter->set_horizontalFit(UnityEngine::UI::ContentSizeFitter::FitMode::Unconstrained);
            filter->set_verticalFit(UnityEngine::UI::ContentSizeFitter::FitMode::PreferredSize);
        }

        BSML::Lite::CreateText(container->get_transform(), SSL10nGen::STR::SETTHING_TEXT_KEPT(), {0,0},{0,5});

        BSML::Lite::CreateUIButton(container->get_transform(), SSL10nGen::STR::SETTHING_APPLY_NOW(), [](){
          LangCtrl::SyncSelectedLanguage();
          RestartGame();
        });

        updater->displayMessageText = BSML::Lite::CreateText(container->get_transform(), "", {0,0}, {70,0});
        updater->displayMessageText->set_enableWordWrapping(true);
        updater->displayMessageText->get_gameObject()->GetComponent<UnityEngine::UI::LayoutElement*>()
            ->set_preferredWidth(70);
        auto messageFitter = updater->displayMessageText->get_gameObject()
            ->AddComponent<UnityEngine::UI::ContentSizeFitter*>();
        messageFitter->set_horizontalFit(UnityEngine::UI::ContentSizeFitter::FitMode::Unconstrained);
        messageFitter->set_verticalFit(UnityEngine::UI::ContentSizeFitter::FitMode::PreferredSize);
        updater->refreshButton = BSML::Lite::CreateUIButton(container->get_transform(), SSL10nGen::STR::REFRESH_MANIFEST(), [updater](){
            Web::NetDataManager::instance.FetchManifest();
            updater->Refresh();
        });
        updater->downloadButton = BSML::Lite::CreateUIButton(container->get_transform(), SSL10nGen::STR::DOWNLOAD_TRANSLATE(), [updater](){
            Web::NetDataManager::instance.DownloadData();
            updater->Refresh();
        });
        updater->statusText = BSML::Lite::CreateText(container->get_transform(), "Status: Idle", {0,0}, {70,5});
        updater->statusText->get_gameObject()->AddComponent<UnityEngine::UI::ContentSizeFitter*>()
            ->set_verticalFit(UnityEngine::UI::ContentSizeFitter::FitMode::PreferredSize);

        auto barSprite = BSML::Utilities::ImageResources::GetWhitePixel();
        auto background = BSML::Lite::CreateImage(container->get_transform(), barSprite, {0,0}, {70,3});
        background->set_color(UnityEngine::Color(0.2f, 0.2f, 0.2f, 1.0f));
        background->set_raycastTarget(false);
        auto layout = background->get_gameObject()->GetComponent<UnityEngine::UI::LayoutElement*>();
        layout->set_preferredHeight(3);
        layout->set_preferredWidth(70);
        updater->progressFill = BSML::Lite::CreateImage(background->get_transform(), barSprite);
        updater->progressFill->set_color(UnityEngine::Color(0.2f, 0.8f, 0.4f, 1.0f));
        updater->progressFill->set_raycastTarget(false);
        auto fillRect = updater->progressFill->get_rectTransform();
        fillRect->set_anchorMin({0,0});
        fillRect->set_anchorMax({0,1});
        fillRect->set_offsetMin({0,0});
        fillRect->set_offsetMax({0,0});
        updater->progressText = BSML::Lite::CreateText(container->get_transform(), "0%", {0,0}, {70,5});

        BSML::Lite::CreateText(container->get_transform(), SSL10nGen::STR::SETTHING_EMBBED_DATA_TITLE(),{0,0},{0,5});
        updater->translationsText = BSML::Lite::CreateText(container->get_transform(), "", {0,0}, {70,5});
        updater->translationsText->get_gameObject()->AddComponent<UnityEngine::UI::ContentSizeFitter*>()
            ->set_verticalFit(UnityEngine::UI::ContentSizeFitter::FitMode::PreferredSize);

        //Do not display the download button. We don't support that in this version.
        updater->refreshButton->set_enabled(false);
        updater->downloadButton->set_enabled(false);
        updater->statusText->set_enabled(false);
        background->set_enabled(false);
        updater->progressFill->set_enabled(false);
        updater->progressText->set_enabled(false);

    }
    auto updater = self->get_gameObject()->GetComponent<LangCtrl::MenuUpdater*>();
    updater->elapsed = 0;
    updater->Refresh();
}



void LangCtrl::SyncSelectedLanguage(){
    int opt = getConfig().SelectedLanguage.GetValue();
    if(opt < 0 || opt >= LangCtrl::L_LASTLANG){
        opt = 0;
    }

    auto lang = (LangCtrl::LanguageOption)opt;
    PaperLogger.info("Synchronize language to {}", opt);
    if(is_initialized && lang == L_FollowGame){
        // This will not works immediately
        // the sslocalization library only follows language when game set it
        auto gameLang = getGameLanguage();
        if(gameLang.has_value()){
            getConfig().LastGameLanguage.SetValue(gameLang.value());
            SSL10n::LanguageController::SetCurrentLanguage(gameLang.value());
        }
        return;
    }

    switch ((LangCtrl::LanguageOption)opt) {
    #define CASE(lang) \
        case LangCtrl::LanguageOption::L_##lang: \
            getConfig().LastGameLanguage.SetValue(SSL10n::Language::L_##lang); \
            SSL10n::LanguageController::SetCurrentLanguage(SSL10n::Language::L_##lang); \
            break;
        FOR_EACH_LANGUAGE(CASE)
    #undef CASE
        default: 
            break;
    }
}
