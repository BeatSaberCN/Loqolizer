#include "EmbbedKeys.hpp"
#include "SSL10n.hpp"
#include "SSL10nGenerated.hpp"
void LangCtrl::InitEmbbedKeys(){
  SSL10nGen::LoadCsv();
  /* We can also add more language in the previous asset, it's polyglot's format */
  SSL10n::Database::Helper(SSL10n::L_Simplified_Chinese)
    .v("LOQOLIZER_SETTHING_LABEL_LANGUAGE", "语言设置/Language")
    .v("LOQOLIZER_SETTHING_MENU_TITLE", "Mod语言")
    .v("LOQOLIZER_SETTHING_APPLY_NOW", "立即应用")
    .v("LOQOLIZER_SETTHING_TEXT_KEPT", "部分内容重启游戏生效")
    .v("LOQOLIZER_SETTHING_TEXT_FOLLOW_HINT", "FollowGame需重启游戏生效")
    ;
}