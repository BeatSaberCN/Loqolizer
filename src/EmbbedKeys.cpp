#include "EmbbedKeys.hpp"
#include "sslocalization/shared/SSL10n.hpp"

void LangCtrl::InitEmbbedKeys(){
  SSL10n::Database::AddKeyValue("LOQOLIZER_SETTHING_LABEL_LANGUAGE", "Language");
  SSL10n::Database::AddKeyValue("LOQOLIZER_SETTHING_LABEL_LANGUAGE", "语言设置/Language", SSL10n::L_Simplified_Chinese);

  SSL10n::Database::AddKeyValue("LOQOLIZER_SETTHING_MENU_TITLE", "Mod Language");
  SSL10n::Database::AddKeyValue("LOQOLIZER_SETTHING_MENU_TITLE", "Mod语言", SSL10n::L_Simplified_Chinese);

  SSL10n::Database::AddKeyValue("LOQOLIZER_SETTHING_APPLY_NOW", "Apply Now");
  SSL10n::Database::AddKeyValue("LOQOLIZER_SETTHING_APPLY_NOW", "立即应用", SSL10n::L_Simplified_Chinese);

  SSL10n::Database::AddKeyValue("LOQOLIZER_SETTHING_TEXT_KEPT", "Some text will not changed until you restart game.");
  SSL10n::Database::AddKeyValue("LOQOLIZER_SETTHING_TEXT_KEPT", "部分内容重启游戏生效", SSL10n::L_Simplified_Chinese);
  
  SSL10n::Database::AddKeyValue("LOQOLIZER_SETTHING_TEXT_FOLLOW_HINT", "FollowGame need restart game.");
  SSL10n::Database::AddKeyValue("LOQOLIZER_SETTHING_TEXT_FOLLOW_HINT", "FollowGame需重启游戏生效", SSL10n::L_Simplified_Chinese);
}