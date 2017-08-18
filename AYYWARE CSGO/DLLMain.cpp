/*
Rest In Peace GhostWareCheats
*/
//#define AC_DEBUG

// General shit
#include "DLLMain.h"
#include "Utilities.h"

// Injection stuff
#include "INJ/ReflectiveLoader.h"

// Stuff to initialise
#include "Offsets.h"
#include "Interfaces.h"
#include "Hooks.h"
#include "RenderManager.h"
#include "Hacks.h"
#include "Menu.h"
#include "AntiAntiAim.h"

#include "Dumping.h"

// Used as part of the reflective DLL injection
extern HINSTANCE hAppInstance;

// Our DLL Instance
HINSTANCE HThisModule;
bool DoUnload;

// Our thread we use to setup everything we need
// Everything appart from code in hooks get's called from inside 
// here.

int InitialThread()
{
	Utilities::OpenConsole(Utilities::GetTitle() + " 构建日期 " __DATE__);

	Sleep(200);
	if(!Utilities::SocketListenClient())
	{
		Utilities::Log("无法连接到后端！即将终止进程");
		Sleep(1000);
		Hooks::UndoHooks();
		Sleep(2000); // Make sure none of our hooks are running
		FreeLibraryAndExitThread(HThisModule, 0);
	}

	// Simple anti crack - Noob proofed. (Obsolete)
	//if (!Utilities::GetProcessByName("GhostWare_Injector.exe"))
	//{
	//	Utilities::Log("无法连接后端！");
	//	Utilities::Log("注入失败！");
	//	Sleep(5000);
	//	Utilities::CloseConsole();
	//	return -1;
	//}

	// Intro banner with info
	PrintMetaHeader();

	//---------------------------------------------------------
	// Initialise all our shit
	Offsets::Initialise(); // Set our VMT offsets and do any pattern scans
	Interfaces::Initialise(); // Get pointers to the valve classes
	NetVar.RetrieveClasses(); // Setup our NetVar manager (thanks shad0w bby)
	NetvarManager::Instance()->CreateDatabase();
	Render::Initialise();
	Hacks::SetupHacks();
	Menu::SetupMenu();
	Hooks::Initialise();
	ApplyAAAHooks();

	//GUI.LoadWindowState(&Menu::Window, "config.xml");

	// Dumping
	//Dump::DumpClassIds();

	//---------------------------------------------------------
	Utilities::Log("注入成功！");
	Utilities::Log("游戏内按下Insert键打开菜单");
	Utilities::Log("感谢您使用GhostWare。");
	Utilities::Log("请尽情享受您的游戏体验。");

	// While our cheat is running
	while (DoUnload == false)
	{
		Sleep(1000);
	}

	Hooks::UndoHooks();
	Sleep(2000); // Make sure none of our hooks are running
	FreeLibraryAndExitThread(HThisModule, 0);

	return 0;
}

// DllMain
// Entry point for our module
BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
	)
{
	switch (fdwReason)
	{
	case DLL_QUERY_HMODULE:
		if (lpvReserved != NULL)
			*(HMODULE *)lpvReserved = hAppInstance;
		break;
	case DLL_PROCESS_ATTACH:
		HThisModule = hinstDLL;
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)InitialThread, NULL, NULL, NULL);
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}