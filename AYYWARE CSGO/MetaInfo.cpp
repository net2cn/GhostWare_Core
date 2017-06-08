/*
Syn's Apocalypse Framework
*/

#include "MetaInfo.h"
#include "Utilities.h"

void PrintMetaHeader()
{
	Utilities::SetConsoleColor(FOREGROUND_INTENSE_GREEN);
	Utilities::SetConsoleColor(FOREGROUND_WHITE);
	Utilities::Log("Build %s", __DATE__);
	Utilities::Log("Setting Up GhostWare for %s", Apocalypse_META_GAME);
}