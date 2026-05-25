#pragma once

#include "sslocalization/shared/SSL10n.hpp"
#include "modconfig.hpp"

namespace LangCtrl {
    void DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    enum LanguageOption{
        L_FollowGame,
        L_English,
        L_French,
        L_Spanish,
        L_German,
        L_Italian,
        L_Portuguese_Brazil,
        L_Portuguese,
        L_Russian,
        L_Greek,
        L_Turkish,
        L_Danish,
        L_Norwegian,
        L_Swedish,
        L_Dutch,
        L_Polish,
        L_Finnish,
        L_Japanese,
        L_Simplified_Chinese,
        L_Traditional_Chinese,
        L_Korean,
        L_Czech,
        L_Hungarian,
        L_Romanian,
        L_Thai,
        L_Bulgarian,
        L_Hebrew,
        L_Arabic,
        L_Bosnian,
        L_LASTLANG
    };
    void SyncSelectedLanguage();
}