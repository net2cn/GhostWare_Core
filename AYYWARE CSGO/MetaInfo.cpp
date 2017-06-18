/*
Syn's Apocalypse Framework
*/

#include "MetaInfo.h"
#include "Utilities.h"

void PrintMetaHeader()
{
	Utilities::SetConsoleColor(FOREGROUND_INTENSE_GREEN);
	Utilities::SetConsoleColor(FOREGROUND_WHITE);
	Utilities::Log("构建日期 %s", __DATE__);
	Utilities::Log("正在为%s设置GhostWare", Apocalypse_META_GAME);
}