#pragma once

#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(Config) {
    // LangCtrl::LanguageOption
    CONFIG_VALUE(SelectedLanguage, int, "SelectedLanguage", 0);

    // SSL10n::Language this is last selected game value, use for early mod load
    CONFIG_VALUE(LastGameLanguage, int, "LastGameLanguage", 0);
    CONFIG_VALUE(EnableGameLocalize, bool, "EnableGameLocalize", true);
};