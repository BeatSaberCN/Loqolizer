#pragma once

// Include the modloader header, which allows us to tell the modloader which mod
// this is, and the version etc.
#include "scotland2/shared/modloader.h"

// beatsaber-hook is a modding framework that lets us call functions and fetch
// field values from in the game It also allows creating objects, configuration,
// and importantly, hooking methods to modify their values
#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"


#include "paper2_scotland2/shared/logger.hpp"

#include "_config.hpp"

constexpr auto PaperLogger = Paper::ConstLoggerContext("Loqolizer");

#define FOR_EACH_LANGUAGE(A)        \
        A(English)                  \
        A(French)                   \
        A(Spanish)                  \
        A(German)                   \
        A(Italian)                  \
        A(Portuguese_Brazil)        \
        A(Portuguese)               \
        A(Russian)                  \
        A(Greek)                    \
        A(Turkish)                  \
        A(Danish)                   \
        A(Norwegian)                \
        A(Swedish)                  \
        A(Dutch)                    \
        A(Polish)                   \
        A(Finnish)                  \
        A(Japanese)                 \
        A(Simplified_Chinese)       \
        A(Traditional_Chinese)      \
        A(Korean)                   \
        A(Czech)                    \
        A(Hungarian)                \
        A(Romanian)                 \
        A(Thai)                     \
        A(Bulgarian)                \
        A(Hebrew)                   \
        A(Arabic)                   \
        A(Bosnian)
