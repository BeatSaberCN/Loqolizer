#pragma once

#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(Config) {
    CONFIG_VALUE(SelectedLanguage, int, "SelectedLanguage", 0);
    CONFIG_VALUE(EnableGameLocalize, bool, "EnableGameLocalize", true);
};