/*
Syn's Apocalypse Framework
*/

#include "MetaInfo.h"
#include "Utilities.h"

void PrintMetaHeader()
{
	Utilities::SetConsoleColor(FOREGROUND_INTENSE_GREEN);
	Utilities::SetConsoleColor(FOREGROUND_WHITE);
	Utilities::Log("�������� %s", __DATE__);
	Utilities::Log("����Ϊ%s����GhostWare", Apocalypse_META_GAME);
}