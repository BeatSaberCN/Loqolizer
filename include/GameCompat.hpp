#pragma once

#include "BGLib/Polyglot/LocalizationAsyncInstaller.hpp"

#if defined(GAME_VER_1_40_8) || defined(GAME_VER_1_37_0)
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/typedefs-string.hpp"
#include "BGLib/Polyglot/Language.hpp"
#include "bsml/shared/BSML/Settings/SettingsMenu.hpp"
#include "UnityEngine/Resources.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"

using PolyglotLanguageTy = ::BGLib::Polyglot::Language;
using IInstallerRegistry = ::BGLib::AppFlow::Initialization::AsyncInstaller::IInstallerRegistry;

namespace i2c {
    namespace functions {
        inline void initialize() {
            il2cpp_functions::Init();
        }
    }
}

inline void RestartGame(){
    auto lists = UnityEngine::Resources::FindObjectsOfTypeAll<GlobalNamespace::MenuTransitionsHelper*>();
    if(lists.size() > 0){
        auto first = lists->First();
        if(first){
            first->RestartGame(nullptr);
        }
    }
}

using SafePtrBSMLSettingsMenuTy = SafePtr<BSML::SettingsMenu>;


#else
#include "beatsaber-hook/shared/hooking.hpp"
#include "BGLib/Polyglot/LocalizationLanguage.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "bsml/shared/BSML/Settings/BSMLSettings.hpp"
#include "beatsaber-hook/shared/safeptr.hpp"
#include "beatsaber-hook/shared/api.hpp"

using PolyglotLanguageTy = ::BGLib::Polyglot::LocalizationLanguage;
using IInstallerRegistry = ::BGLib::AppFlow::Initialization::IInstallerRegistry;

#include "bsml/shared/Helpers/getters.hpp"

inline void RestartGame(){
    auto* helper = BSML::Helpers::GetDiContainer()->Resolve<GlobalNamespace::MenuTransitionsHelper*>();
    if (!helper) {
        return;
    }
    helper->RestartGame(nullptr);
}

using SafePtrBSMLSettingsMenuTy = safe_ptr<BSML::SettingsMenu*>;

#endif