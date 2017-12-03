#include "Client.h"

//[enc_string_enable /]
//[junk_enable /]

namespace Client
{
	//[swap_lines]
	int	iScreenWidth = 0;
	int	iScreenHeight = 0;

	string BaseDir = "";
	string LogFile = "";
	string GuiFile = "";
	string IniFile = "";

	vector<string> ConfigList;

	Vector2D	g_vCenterScreen = Vector2D(0.f, 0.f);

	CPlayers*	g_pPlayers = nullptr;
	CRender*	g_pRender = nullptr;
	CGui*		g_pGui = nullptr;

	CAimbot*	g_pAimbot = nullptr;
	CTriggerbot* g_pTriggerbot = nullptr;
	CEsp*		g_pEsp = nullptr;
	CRadar*		g_pRadar = nullptr;
	CKnifebot*	g_pKnifebot = nullptr;
	CSkin*		g_pSkin = nullptr;
	CMisc*		g_pMisc = nullptr;

	bool		bC4Timer = false;
	int			iC4Timer = 40;

	int			iWeaponID = 0;
	int			iWeaponSelectIndex = WEAPON_DEAGLE;
	int			iWeaponSelectSkinIndex = -1;
	//[/swap_lines]

	void ReadConfigs(LPCTSTR lpszFileName)
	{
		if (!strstr(lpszFileName, "gui.ini"))
		{
			ConfigList.push_back(lpszFileName);
		}
	}

	void RefreshConfigs()
	{
		ConfigList.clear();
		string ConfigDir = BaseDir + "\\*.ini";
		SearchFiles(ConfigDir.c_str(), ReadConfigs, FALSE);
	}

	bool Initialize(IDirect3DDevice9* pDevice)
	{
		g_pPlayers = new CPlayers();
		g_pRender = new CRender(pDevice);
		g_pGui = new CGui();

		g_pAimbot = new CAimbot();
		g_pTriggerbot = new CTriggerbot();
		g_pEsp = new CEsp();
		g_pRadar = new CRadar();
		g_pKnifebot = new CKnifebot();
		g_pSkin = new CSkin();
		g_pMisc = new CMisc();

		GuiFile = BaseDir + "\\" + "gui.ini";
		IniFile = BaseDir + "\\" + "settings.ini";

		g_pSkin->InitalizeSkins();

		Settings::LoadSettings(IniFile);

		iWeaponSelectSkinIndex = GetWeaponSkinIndexFromPaintKit(g_SkinChangerCfg[iWeaponSelectIndex].nFallbackPaintKit);

		g_pGui->GUI_Init(pDevice);

		RefreshConfigs();

		return true;
	}

	void Shutdown()
	{
		DELETE_MOD(g_pPlayers);
		DELETE_MOD(g_pRender);
		DELETE_MOD(g_pGui);

		DELETE_MOD(g_pAimbot);
		DELETE_MOD(g_pTriggerbot);
		DELETE_MOD(g_pEsp);
		DELETE_MOD(g_pRadar);
		DELETE_MOD(g_pKnifebot);
		DELETE_MOD(g_pSkin);
		DELETE_MOD(g_pMisc);
	}

	void OnRender()
	{
		if (g_pRender && !Interfaces::Engine()->IsTakingScreenshot() && Interfaces::Engine()->IsActiveApp())
		{
			g_pRender->BeginRender();

			if (g_pGui)
				g_pGui->GUI_Draw_Elements();

			Interfaces::Engine()->GetScreenSize(iScreenWidth, iScreenHeight);

			g_vCenterScreen.x = iScreenWidth / 2.f;
			g_vCenterScreen.y = iScreenHeight / 2.f;

			if (!Interfaces::Engine()->IsConnected())
				g_pRender->Text(3, 3, false, true, Color::Aqua(), HACK_NAME);
			{
				if (g_pEsp)
					g_pEsp->OnRender();

				if (g_pMisc)
				{
					g_pMisc->OnRender();
					g_pMisc->OnRenderSpectatorList();
				}
			}

			g_pRender->EndRender();
		}
	}

	void OnLostDevice()
	{
		if (g_pRender)
			g_pRender->OnLostDevice();

		if (g_pGui)
			ImGui_ImplDX9_InvalidateDeviceObjects();
	}

	void OnResetDevice()
	{
		if (g_pRender)
			g_pRender->OnResetDevice();

		if (g_pGui)
			ImGui_ImplDX9_CreateDeviceObjects();
	}

	void OnCreateMove(CUserCmd* pCmd)
	{
		if (g_pPlayers && Interfaces::Engine()->IsInGame())
		{
			g_pPlayers->Update();

			if (g_pEsp)
				g_pEsp->OnCreateMove(pCmd);

			if (IsLocalAlive())
			{
				if (!bIsGuiVisible)
				{
					int iWeaponSettingsSelectID = GetWeaponSettingsSelectID();

					if (iWeaponSettingsSelectID >= 0)
						iWeaponID = iWeaponSettingsSelectID;
				}

				if (g_pAimbot)
					g_pAimbot->OnCreateMove(pCmd, g_pPlayers->GetLocal());

				if (g_pTriggerbot)
					g_pTriggerbot->OnCreateMove(pCmd, g_pPlayers->GetLocal());

				if (g_pKnifebot)
					g_pKnifebot->OnCreateMove(pCmd);

				if (g_pMisc)
					g_pMisc->OnCreateMove(pCmd);
			}
		}
	}

	void OnFireEventClientSideThink(IGameEvent* pEvent)
	{
		if (!strcmp(pEvent->GetName(), "player_connect_full") ||
			!strcmp(pEvent->GetName(), "round_start") ||
			!strcmp(pEvent->GetName(), "cs_game_disconnected"))
		{
			if (g_pPlayers)
				g_pPlayers->Clear();

			if (g_pEsp)
				g_pEsp->OnReset();
		}

		if (Interfaces::Engine()->IsConnected())
		{
			if (g_pEsp)
				g_pEsp->OnEvents(pEvent);

			if (g_pSkin)
				g_pSkin->OnEvents(pEvent);
		}
	}

	void OnFrameStageNotify(ClientFrameStage_t Stage)
	{
		if (Interfaces::Engine()->IsInGame())
		{
			Skin_OnFrameStageNotify(Stage);
			Gloves_OnFrameStageNotify(Stage);
		}
	}

	void OnDrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t &state,
		const ModelRenderInfo_t &pInfo, matrix3x4_t *pCustomBoneToWorld)
	{
		if (Interfaces::Engine()->IsInGame() && ctx && pCustomBoneToWorld)
		{
			if (g_pEsp)
				g_pEsp->OnDrawModelExecute(ctx, state, pInfo, pCustomBoneToWorld);

			if (g_pMisc)
				g_pMisc->OnDrawModelExecute();
		}
	}

	void OnPlaySound(const Vector* pOrigin, const char* pszSoundName)
	{
		if (!pszSoundName || !Interfaces::Engine()->IsInGame())
			return;

		if (!strstr(pszSoundName, "bulletLtoR") &&
			!strstr(pszSoundName, "rics/ric") &&
			!strstr(pszSoundName, "impact_bullet"))
		{
			if (g_pEsp && IsLocalAlive() && Settings::Esp::esp_Sound && pOrigin)
			{
				if (!GetVisibleOrigin(*pOrigin))
					g_pEsp->SoundEsp.AddSound(*pOrigin);
			}
		}
	}

	void OnPlaySound(const char* pszSoundName)
	{
		if (g_pMisc)
			g_pMisc->OnPlaySound(pszSoundName);
	}

	void OnOverrideView(CViewSetup* pSetup)
	{
		if (g_pMisc)
			g_pMisc->OnOverrideView(pSetup);
	}

	void OnGetViewModelFOV(float& fov)
	{
		if (g_pMisc)
			g_pMisc->OnGetViewModelFOV(fov);
	}

	void OnRenderGUI()
	{
		ImGui::SetNextWindowSize(ImVec2(560.f, 325.f));

		if (ImGui::Begin(HACK_NAME, &bIsGuiVisible, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			if (Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_FovType > 1)
				Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_FovType = 1;

			if (Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_BestHit > 1)
				Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_BestHit = 1;

			if (Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Spot > 5)
				Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Spot = 5;

			const char* tabNames[] = {
				AIMBOT_TEXT , TRIGGER_TEXT , VISUAL_TEXT , RADAR_TEXT ,
				KNIFEBOT_TEXT , SKIN_TEXT , MISC_TEXT , CONFIG_TEXT };

			static int tabOrder[] = { 0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 };
			static int tabSelected = 0;
			const bool tabChanged = ImGui::TabLabels(tabNames,
				sizeof(tabNames) / sizeof(tabNames[0]),
				tabSelected, tabOrder);

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			float SpaceLineOne = 120.f;
			float SpaceLineTwo = 220.f;
			float SpaceLineThr = 320.f;

			if (tabSelected == 0) // Aimbot
			{
				ImGui::PushItemWidth(110.f);
				ImGui::Text("\xe5\xbd\x93\xe5\x89\x8d\xe6\xad\xa6\xe5\x99\xa8: ");//Current Weapon 当前武器
				ImGui::SameLine();
				ImGui::Combo("##\xE6\xAD\xA6\xE5\x99\xA8\xE8\x87\xAA\xE7\x9E\x84\x0A", &iWeaponID, pWeaponData, IM_ARRAYSIZE(pWeaponData));//AimWeapon 武器自瞄
				ImGui::SameLine();
				ImGui::Checkbox("\xE5\x85\xA8\xE5\xB1\x80\xE5\x90\xAF\xE7\x94\xA8", &Settings::Aimbot::aim_ActiveAll);//Active All 全局启用
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("\xE8\xBF\x99\xE4\xB8\xAA\xE8\xAE\xBE\xE7\xBD\xAE\xE5\xB0\x86\xE8\xA6\x86\xE7\x9B\x96\xE4\xB8\x8B\xE9\x9D\xA2\xE7\x9A\x84\x22\xE5\x90\xAF\xE7\x94\xA8\x22\xE9\x80\x89\xE6\x8B\xA9\xE6\xA1\x86\xE3\x80\x82");//This will override Active Checkbox below. 这个设置将覆盖下面的“启用”选择框。
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox("\xe6\xad\xbb\xe4\xba\xa1\xe7\xab\x9e\xe8\xb5\x9b\xe6\xa8\xa1\xe5\xbc\x8f", &Settings::Aimbot::aim_Deathmatch); //Deathmatch 死亡竞赛
				ImGui::SameLine(SpaceLineOne);
				ImGui::Checkbox("\xe7\xa9\xbf\xe5\xa2\x99", &Settings::Aimbot::aim_WallAttack);//WallAttack 穿墙
				ImGui::SameLine(SpaceLineTwo);
				ImGui::Checkbox("\xE6\x98\xAF\xE5\x90\xA6\xE7\xA9\xBF\xE7\x83\x9F\x0A", &Settings::Aimbot::aim_CheckSmoke);//CheckSmoke 是否穿烟

				ImGui::Checkbox("\xe5\x8f\x8d\xe8\xb7\xb3\xe8\xb7\x83", &Settings::Aimbot::aim_AntiJump);//AntiJump 反跳跃
				ImGui::SameLine(SpaceLineOne);
				ImGui::Checkbox("\xE7\xBB\x98\xE5\x88\xB6\xE7\x9E\x84\xE5\x87\x86\xE8\x8C\x83\xE5\x9B\xB4", &Settings::Aimbot::aim_DrawFov);//Draw Fov 绘制瞄准范围
				ImGui::SameLine(SpaceLineTwo);
				ImGui::Checkbox("\xE7\xBB\x98\xE5\x88\xB6\xE7\x9E\x84\xE5\x87\x86\xE7\x82\xB9", &Settings::Aimbot::aim_DrawSpot);//DrawSpot 绘制瞄准点

				ImGui::Checkbox("Backtrack", &Settings::Aimbot::aim_Backtrack);//我也不知道是什么啊
				ImGui::SameLine(SpaceLineOne);
				ImGui::SliderInt("Ticks", &Settings::Aimbot::aim_Backtracktickrate, 1, 16);//我也不知道是什么啊

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox("\xE5\x90\xAF\xE5\x8A\xA8", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Active);//Active 启动

				if (iWeaponID <= 9)
				{
					ImGui::SameLine();
					ImGui::Checkbox("\xE8\x87\xAA\xE5\x8A\xA8\xE6\x89\x8B\xE6\x9E\xAA\x0A", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_AutoPistol);//Autopistol 自动手枪
				}
				ImGui::PushItemWidth(362.f);
				ImGui::SliderInt("\xE5\xB9\xB3\xE6\xBB\x91\x0A", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Smooth, 1, 300);//Smooth 平滑
				ImGui::SliderInt("\xE8\x87\xAA\xE7\x9E\x84\xE8\x8C\x83\xE5\x9B\xB4", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Fov, 1, 300);//Fov 范围
				ImGui::PopItemWidth();

				const char* AimFovType[] = { "\xE5\x8A\xA8\xE6\x80\x81\x0A" , "\xE9\x9D\x99\xE6\x80\x81\x0A" };//Dynamic Static 动态 静态
				ImGui::PushItemWidth(362.f);
				ImGui::Combo("\xE8\x8C\x83\xE5\x9B\xB4\xE7\xB1\xBB\xE5\x9E\x8B\x0A", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_FovType, AimFovType, IM_ARRAYSIZE(AimFovType));//Fov Type 范围类型
				ImGui::PopItemWidth();

				const char* BestHit[] = { "\xE5\x85\xB3\xE9\x97\xAD\x0A" , "\xE5\xBC\x80\xE5\x90\xAF\x0A" };//Disable Enable 关闭 开启
				ImGui::PushItemWidth(362.f);
				ImGui::Combo("\xE7\xB2\xBE\xE5\x87\x86\xE5\xB0\x84\xE5\x87\xBB\x0A", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_BestHit, BestHit, IM_ARRAYSIZE(BestHit));//BestHit 精准射击

				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("\xE5\x85\xB3\xE9\x97\xAD\xE4\xBB\xA5\xE4\xBD\xBF\xE7\x94\xA8\xE8\x87\xAA\xE7\x9E\x84\xE7\x82\xB9\x0A");//if disabled then used Aimspot 关闭以使用自瞄点

				ImGui::PopItemWidth();

				const char* Aimspot[] = { "\xE5\xA4\xB4\x0A" , "\xE8\x84\x96\xE5\xAD\x90\x0A" , "\xE9\x94\x81\xE9\xAA\xA8\x0A" , "\xE8\xBA\xAB\xE4\xBD\x93\x0A" , "\xE8\x83\xB8\xE9\x83\xA8\x0A" , "\xE8\x85\xB9\xE9\x83\xA8\x0A" };//"Head" , "Neck" , "Low Neck" , "Body" , "Thorax" , "Chest" 头 脖子 锁骨 身体 胸部 腹部
				ImGui::PushItemWidth(362.f);
				ImGui::Combo("\xE7\x9E\x84\xE5\x87\x86\xE9\x83\xA8\xE4\xBD\x8D\x0A", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Spot, Aimspot, IM_ARRAYSIZE(Aimspot));//Aimspot 瞄准部位
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::PushItemWidth(362.f);
				ImGui::SliderInt("\xE5\xB0\x84\xE5\x87\xBB\xE5\xBB\xB6\xE6\x97\xB6\x0A", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Delay, 0, 200);//ShotDelay 射击延时
				ImGui::SliderInt("\xE5\x90\x8E\xE5\x9D\x90\xE5\x8A\x9B\xE6\x8E\xA7\xE5\x88\xB6\x0A", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Rcs, 0, 100);//Rcs 后坐力控制
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				if (iWeaponID >= 10 && iWeaponID <= 30)
				{
					ImGui::PushItemWidth(362.f);
					ImGui::SliderInt("\xE5\x90\x8E\xE5\x9D\x90\xE5\x8A\x9B\xE6\x8E\xA7\xE5\x88\xB6\xE8\x8C\x83\xE5\x9B\xB4", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_RcsFov, 1, 300);//Rcs Fov 后坐力控制范围
					ImGui::SliderInt("\xE5\x90\x8E\xE5\x9D\x90\xE5\x8A\x9B\xE6\x8E\xA7\xE5\x88\xB6\xE5\xB9\xB3\xE6\xBB\x91", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_RcsSmooth, 1, 300);//Rcs Smooth 后坐力控制平滑
					ImGui::PopItemWidth();

					const char* ClampType[] = { "\xE5\x85\xA8\xE9\x83\xA8\xE6\x95\x8C\xE4\xBA\xBA\x0A" , "Shot" , "Shot + Target" };//"All Target" , "Shot" , "Shot + Target"全部敌人 射击 射击+扳机
					ImGui::PushItemWidth(362.f);
					ImGui::Combo("\xE5\x90\x8E\xE5\x9D\x90\xE5\x8A\x9B\xE9\x99\x90\xE5\x88\xB6", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_RcsClampType, ClampType, IM_ARRAYSIZE(ClampType));//Rcs Clamp 后坐力锁定
					ImGui::PopItemWidth();

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();
				}
			}
			else if (tabSelected == 1) // Trigger
			{
				const char* TriggerEnable[] = { "\xE5\x85\xB3\xE9\x97\xAD\x0A" , "\xE8\x8C\x83\xE5\x9B\xB4\x0A" , "\xE8\xBF\xBD\xE8\xB8\xAA\x0A" };//关闭 范围 跟踪
				ImGui::PushItemWidth(80.f);
				ImGui::Combo("\xE5\xBC\x80\xE5\x90\xAF\x0A", &Settings::Triggerbot::trigger_Enable, TriggerEnable, IM_ARRAYSIZE(TriggerEnable));//Enalbe 开启
				ImGui::PopItemWidth();
				ImGui::SameLine();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox("\xe6\xad\xbb\xe4\xba\xa1\xe7\xab\x9e\xe8\xb5\x9b\xe6\xa8\xa1\xe5\xbc\x8f", &Settings::Triggerbot::trigger_Deathmatch);//Deathmatch 死亡竞赛
				ImGui::SameLine(SpaceLineOne);
				ImGui::Checkbox("\xe7\xa9\xbf\xe5\xa2\x99", &Settings::Triggerbot::trigger_WallAttack);//WallAttack 穿墙
				ImGui::SameLine(SpaceLineTwo);
				ImGui::Checkbox("\xE8\x87\xAA\xE5\x8A\xA8\xE5\xBC\x80\xE9\x95\x9C\x0A", &Settings::Triggerbot::trigger_FastZoom);//FastZoom 自动开镜

				ImGui::Checkbox("\xE6\x98\xAF\xE5\x90\xA6\xE7\xA9\xBF\xE7\x83\x9F\x0A", &Settings::Triggerbot::trigger_SmokCheck);//SmokeCheck 是否穿烟
				ImGui::SameLine(SpaceLineOne);
				ImGui::Checkbox("\xE7\x9E\x84\xE5\x87\x86\xE7\x82\xB9\xE8\x8C\x83\xE5\x9B\xB4\x0A", &Settings::Triggerbot::trigger_DrawFov);//DrawFov 瞄准点范围
				ImGui::SameLine(SpaceLineTwo);
				ImGui::Checkbox("\xE7\x9E\x84\xE5\x87\x86\xE7\x82\xB9\x0A", &Settings::Triggerbot::trigger_DrawSpot);//DrawSpot 瞄准点
				ImGui::SameLine(SpaceLineThr);
				ImGui::Checkbox("\xE7\x9E\x84\xE5\x87\x86\xE7\x82\xB9\xE5\x8D\x8F\xE5\x8A\xA9\x0A", &Settings::Triggerbot::trigger_DrawFovAssist);//DrawFovAssist 瞄准点协助

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				const char* items1[] = { CVAR_KEY_MOUSE3 , CVAR_KEY_MOUSE4 , CVAR_KEY_MOUSE5 };
				ImGui::PushItemWidth(80.f);
				ImGui::Combo("\xE6\x8C\x89\xE9\x94\xAE\x0A", &Settings::Triggerbot::trigger_Key, items1, IM_ARRAYSIZE(items1));//key 按键
				ImGui::PopItemWidth();
				ImGui::SameLine();

				const char* items2[] = { "\xE6\x8C\x89\xE4\xBD\x8F\x0A" , "\xE6\x8C\x89\xE9\x94\xAE\x0A" };//Hold Press按住 按键
				ImGui::PushItemWidth(80.f);
				ImGui::Combo("\xE6\x8C\x89\xE9\x94\xAE\xE6\xA8\xA1\xE5\xBC\x8F\x0A", &Settings::Triggerbot::trigger_KeyMode, items2, IM_ARRAYSIZE(items2));//key mode 按键模式
				ImGui::PopItemWidth();
				ImGui::SameLine();

				ImGui::PushItemWidth(110.f);
				ImGui::Combo("\xE6\xAD\xA6\xE5\x99\xA8\x0A", &iWeaponID, pWeaponData, IM_ARRAYSIZE(pWeaponData));//weapon 武器
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::PushItemWidth(362.f);
				ImGui::SliderInt("\xE6\x9C\x80\xE5\xB0\x8F\xE8\xB7\x9D\xE7\xA6\xBB\x0A", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_DistanceMin, 0, 5000);//Min Disstance最小距离
				ImGui::SliderInt("\xE6\x9C\x80\xE5\xA4\xA7\xE8\xB7\x9D\xE7\xA6\xBB\x0A", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_DistanceMax, 0, 5000);//Max Disstance最大距离
				ImGui::SliderInt("\xE8\x8C\x83\xE5\x9B\xB4\x0A", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_Fov, 1, 100);//Fov范围
				ImGui::SliderInt("\xE5\xBB\xB6\xE6\x97\xB6\xE5\x89\x8D\x0A", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_DelayBefore, 0, 200);//Delay Before延时之前
				ImGui::SliderInt("\xE5\xBB\xB6\xE6\x97\xB6\xE5\x90\x8E\x0A", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_DelayAfter, 0, 1000);//Delay After延时之后
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox("\xE5\x8F\xAA\xE7\x9E\x84\xE5\xA4\xB4\x0A", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_HeadOnly);//HeadOnly 只瞄头
				ImGui::SameLine();

				const char* AssistMode[] = { "\xE5\x85\xB3\xE9\x97\xAD\x0A" , "One Shot" , "\xE8\x87\xAA\xE5\x8A\xA8\x0A" };//Disable One Shot Auto关闭 一枪 自动
				ImGui::PushItemWidth(80.f);
				ImGui::Combo("\xE5\x90\xB8\xE9\x99\x84\x0A", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_Assist, AssistMode, IM_ARRAYSIZE(AssistMode));//Assist 吸附
				ImGui::PopItemWidth();
				ImGui::SameLine();

				const char* AssistFovType[] = { "\xE5\x8A\xA8\xE6\x80\x81\x0A" , "\xE9\x9D\x99\xE6\x80\x81\x0A" };//Dynamic Static 动态 静态
				ImGui::PushItemWidth(80.f);
				ImGui::Combo("\xE5\x90\xB8\xE9\x99\x84\xE8\x8C\x83\xE5\x9B\xB4\xE7\xB1\xBB\xE5\x9E\x8B", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_AssistFovType, AssistFovType, IM_ARRAYSIZE(AssistFovType));//Assist Fov Type 吸附范围类型
				ImGui::PopItemWidth();

				const char* HitGroup[] = { "\xE5\x85\xA8\xE9\x83\xA8\x0A" , "\xE5\xA4\xB4\x2B\xE8\xBA\xAB\xE4\xBD\x93\x0A" , "\xE5\xA4\xB4\x0A" };//all head+body Head 全部 头+身子 头
				ImGui::Combo("\xE7\x9B\xAE\xE6\xA0\x87\xE9\x83\xA8\xE4\xBD\x8D\x0A", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_HitGroup, HitGroup, IM_ARRAYSIZE(HitGroup));//hitgroup 目标部位
																																																   //ImGui::PopItemWidth();

				ImGui::PushItemWidth(362.f);
				ImGui::SliderInt("\xE5\x90\xB8\xE9\x99\x84\xE5\x90\x8E\xE5\x9D\x90\xE5\x8A\x9B\x0A", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_AssistRcs, 0, 100);// Assist Rcs 吸附后坐力
				ImGui::SliderInt("\xE5\x90\xB8\xE9\x99\x84\xE8\x8C\x83\xE5\x9B\xB4\x0A", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_AssistFov, 1, 300);// Assist Fov 吸附范围
				ImGui::SliderInt("\xE5\x90\xB8\xE9\x99\x84\xE9\x80\x9F\xE5\xBA\xA6\x0A", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_AssistSmooth, 1, 300);//Assist Smooth 吸附速度
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
			}
			else if (tabSelected == 2) // Visuals
			{
				string style_1 = "\xE6\x96\xB9\xE6\xA1\x86\x0A";//Box 方框
				string style_2 = "\xE8\xBE\xB9\xE8\xA7\x92\xE6\xA1\x86";//CoalBox 边角框

				const char* items1[] = { style_1.c_str() , style_2.c_str() };

				ImGui::PushItemWidth(339.f);
				ImGui::Combo("\xE9\x80\x8F\xE8\xA7\x86\xE7\xB1\xBB\xE5\x9E\x8B\x0A", &Settings::Esp::esp_Style, items1, IM_ARRAYSIZE(items1));//Esp Type 透视类型
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox("\xE9\x98\x9F\xE5\x8F\x8B\x0A", &Settings::Esp::esp_Team);//Esp Team 队友
				ImGui::SameLine(SpaceLineOne);
				ImGui::Checkbox("\xE6\x95\x8C\xE4\xBA\xBA\x0A", &Settings::Esp::esp_Enemy);//Esp Enemy 敌人
				ImGui::SameLine(SpaceLineTwo);
				ImGui::Checkbox("\xE7\x82\xB8\xE5\xBC\xB9\x0A", &Settings::Esp::esp_Bomb);//Esp Bomp 炸弹
				ImGui::SameLine(SpaceLineThr);
				ImGui::Checkbox("\xE8\x84\x9A\xE6\xAD\xA5\xE8\xBD\xA8\xE8\xBF\xB9\x0A", &Settings::Esp::esp_Sound);//Esp Sound 脚步轨迹

				ImGui::Checkbox("\xE7\x9B\xB4\xE7\xBA\xBF\x0A", &Settings::Esp::esp_Line);//Esp Line 直线
				ImGui::SameLine(SpaceLineOne);
				ImGui::Checkbox("\xE8\xBD\xAE\xE5\xBB\x93\x0A", &Settings::Esp::esp_Outline);//Esp OutLine 轮廓
				ImGui::SameLine(SpaceLineTwo);
				ImGui::Checkbox("\xE5\x90\x8D\xE5\xAD\x97\x0A", &Settings::Esp::esp_Name);//Esp Name 名字
				ImGui::SameLine(SpaceLineThr);
				ImGui::Checkbox("\xE6\xAE\xB5\xE4\xBD\x8D\x0A", &Settings::Esp::esp_Rank);//Esp Rank 段位

				ImGui::Checkbox("\xE6\xAD\xA6\xE5\x99\xA8\xE4\xBF\xA1\xE6\x81\xAF\x0A", &Settings::Esp::esp_Weapon);//Esp Weapon 武器信息
				ImGui::SameLine(SpaceLineOne);
				ImGui::Checkbox("\xE5\xBC\xB9\xE8\x8D\xAF\x0A", &Settings::Esp::esp_Ammo);//Esp Ammo 弹药
				ImGui::SameLine(SpaceLineTwo);
				ImGui::Checkbox("\xE8\xB7\x9D\xE7\xA6\xBB\x0A", &Settings::Esp::esp_Distance);//Esp Distance 距离
				ImGui::SameLine(SpaceLineThr);
				ImGui::Checkbox("\xE9\xAA\xA8\xE6\x9E\xB6\x0A", &Settings::Esp::esp_Skeleton);//Esp Skeleton 骨架

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox("\xE5\x85\xA8\xE5\xB1\x80\xE6\xAD\xA6\xE5\x99\xA8\x0A", &Settings::Esp::esp_WorldWeapons);//Esp World Weapon 全局武器
				ImGui::Checkbox("\xE6\x89\x8B\xE9\x9B\xB7\xE8\xBD\xA8\xE8\xBF\xB9\x0A", &Settings::Esp::esp_WorldGrenade);//Esp World Grenade 手雷轨迹

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				string visible_1 = "\xE6\x95\x8C\xE4\xBA\xBA\x0A";//Enemy 敌人
				string visible_2 = "\xE9\x98\x9F\xE5\x8F\x8B\x0A";//Team 队友
				string visible_3 = "\xE5\x85\xA8\xE9\x83\xA8\x0A";//All 全部
				string visible_4 = "\xE4\xBB\x85\xE5\x8F\xAF\xE8\xA7\x81\xE6\x97\xB6\x0A";//Only Visible 仅可见时

				const char* items2[] = { visible_1.c_str() , visible_2.c_str() , visible_3.c_str() , visible_4.c_str() };

				ImGui::PushItemWidth(339.f);
				ImGui::Combo("\xE5\x8F\xAF\xE8\xA7\x81\xE6\x97\xB6\x0A", &Settings::Esp::esp_Visible, items2, IM_ARRAYSIZE(items2));//不确定!!!!!!!!!!!!!!! Esp Visible 透视可见时

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::SliderInt("\xE6\x96\xB9\xE6\xA1\x86\xE5\xA4\xA7\xE5\xB0\x8F", &Settings::Esp::esp_Size, 0, 10);//Esp Size 方框大小
				ImGui::SliderInt("C4\xE7\x88\x86\xE7\x82\xB8\xE6\x97\xB6\xE9\x97\xB4", &Settings::Esp::esp_BombTimer, 0, 65);//Esp BombTimer C4爆炸时间
				ImGui::SliderInt("\xE7\x9E\x84\xE5\x87\x86\xE7\xBA\xBF\x0A", &Settings::Esp::esp_BulletTrace, 0, 3000);//不确定!!!!!!!!!!!!!!! Esp BulletTrace 瞄准线

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				string hpbar_1 = "\xE6\x97\xA0\x0A";//None 无
				string hpbar_2 = "\xE6\x95\xB0\xE5\xAD\x97\x0A";//Number 数字
				string hpbar_3 = "\xE5\xBA\x95\xE9\x83\xA8";//Bottom 底部
				string hpbar_4 = "\xE5\xB7\xA6\xE4\xBE\xA7";//Left 左侧

				const char* items3[] = { hpbar_1.c_str() , hpbar_2.c_str() , hpbar_3.c_str() , hpbar_4.c_str() };
				ImGui::Combo("\xE8\xA1\x80\xE9\x87\x8F\xE6\xA0\xB7\xE5\xBC\x8F", &Settings::Esp::esp_Health, items3, IM_ARRAYSIZE(items3));// Esp Health 血量样式

				string arbar_1 = "\xE6\x97\xA0\x0A";//None 无
				string arbar_2 = "\xE6\x95\xB0\xE5\xAD\x97\x0A";//Number 数字
				string arbar_3 = "\xE5\xBA\x95\xE9\x83\xA8";//Bottom 底部
				string arbar_4 = "\xE5\xB7\xA6\xE4\xBE\xA7";//Left 左侧

				const char* items4[] = { arbar_1.c_str() , arbar_2.c_str() , arbar_3.c_str() , arbar_4.c_str() };
				ImGui::Combo("\xE6\x8A\xA4\xE7\x94\xB2\xE6\xA0\xB7\xE5\xBC\x8F", &Settings::Esp::esp_Armor, items4, IM_ARRAYSIZE(items4));//Esp Armor 护甲样式

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				string chams_1 = "\xE6\x97\xA0\x0A";//None 无
				string chams_2 = "\xE6\x89\x81\xE5\xB9\xB3\x28 2D \x29";//Flat 扁平(2D)
				string chams_3 = "\xE5\x8E\x9F\xE7\x89\x88\x28 3D \x29";//Texture 原版(3D)

				const char* items5[] = { chams_1.c_str() , chams_2.c_str() , chams_3.c_str() };
				ImGui::Combo("\xE5\xBD\xA9\xE4\xBA\xBA\x0A", &Settings::Esp::esp_Chams, items5, IM_ARRAYSIZE(items5));//Chams 彩人

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::ColorEdit3("\xE9\x80\x8F\xE8\xA7\x86 CT \xE9\xA2\x9C\xE8\x89\xB2\x0A", Settings::Esp::esp_Color_CT);//Esp Color CT 透视CT颜色
				ImGui::ColorEdit3("\xE9\x80\x8F\xE8\xA7\x86 T \xE9\xA2\x9C\xE8\x89\xB2\x0A", Settings::Esp::esp_Color_TT);//Esp Color TT 透视T颜色
				ImGui::ColorEdit3("\xE5\x8F\xAF\xE8\xA7\x81 CT \xE9\xA2\x9C\xE8\x89\xB2\x0A", Settings::Esp::esp_Color_VCT);//Esp Color Visible CT 可见CT颜色
				ImGui::ColorEdit3("\xE5\x8F\xAF\xE8\xA7\x81 T \xE9\xA2\x9C\xE8\x89\xB2\x0A", Settings::Esp::esp_Color_VTT);//Esp Color Visible TT 可见T颜色

				ImGui::ColorEdit3("\xE5\xBD\xA9\xE4\xBA\xBA CT \xE9\xA2\x9C\xE8\x89\xB2\x0A", Settings::Esp::chams_Color_CT);//Chams Color CT 彩人CT颜色
				ImGui::ColorEdit3("\xE5\xBD\xA9\xE4\xBA\xBA T \xE9\xA2\x9C\xE8\x89\xB2\x0A", Settings::Esp::chams_Color_TT);//Chams Color TT 彩人T颜色
				ImGui::ColorEdit3("\xE5\xBD\xA9\xE4\xBA\xBA\xE5\x8F\xAF\xE8\xA7\x81 CT \xE9\xA2\x9C\xE8\x89\xB2\x0A", Settings::Esp::chams_Color_VCT);//Chams Color Visible CT 彩人可见CT颜色
				ImGui::ColorEdit3("\xE5\xBD\xA9\xE4\xBA\xBA\xE5\x8F\xAF\xE8\xA7\x81 T \xE9\xA2\x9C\xE8\x89\xB2\x0A", Settings::Esp::chams_Color_VTT);//Chams Color Visible TT 彩人可见T颜色
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
			}
			else if (tabSelected == 3) // Radar
			{
				ImGui::Checkbox("\xE5\x90\xAF\xE5\x8A\xA8\x0A", &Settings::Radar::rad_Active);//Active 启动
				ImGui::SameLine();
				ImGui::Checkbox("\xE9\x98\x9F\xE5\x8F\x8B\x0A", &Settings::Radar::rad_Team);//Team 队友
				ImGui::SameLine();
				ImGui::Checkbox("\xE6\x95\x8C\xE4\xBA\xBA\x0A", &Settings::Radar::rad_Enemy);//Enemy 敌人
				ImGui::SameLine();
				ImGui::Checkbox("\xE8\xBD\xA8\xE8\xBF\xB9\x0A", &Settings::Radar::rad_Sound);//Sound 轨迹
				ImGui::SameLine();
				ImGui::Checkbox("\xE5\x86\x85\xE7\xBD\xAE\x0A", &Settings::Radar::rad_InGame);//InGame 内置

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::PushItemWidth(339.f);
				ImGui::SliderInt("\xE5\xA4\xA7\xE5\xB0\x8F", &Settings::Radar::rad_Range, 1, 5000);//Range 大小
				ImGui::SliderInt("\xE9\x80\x8F\xE6\x98\x8E\xE5\xBA\xA6", &Settings::Radar::rad_Alpha, 1, 255);//Alpha 透明度

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::ColorEdit3("CT\xE9\xA2\x9C\xE8\x89\xB2\x0A", Settings::Radar::rad_Color_CT);//不确定!!!!!!!!!!!!!!! Color CT
				ImGui::ColorEdit3("T\xE9\xA2\x9C\xE8\x89\xB2\x0A", Settings::Radar::rad_Color_TT);//不确定!!!!!!!!!!!!!!! Color TT
				ImGui::ColorEdit3("CT\xE5\x8F\xAF\xE8\xA7\x81\xE6\x97\xB6\xE9\xA2\x9C\xE8\x89\xB2\x0A", Settings::Radar::rad_Color_VCT);//不确定!!!!!!!!!!!!!!! Color Visible CT
				ImGui::ColorEdit3("T\xE5\x8F\xAF\xE8\xA7\x81\xE6\x97\xB6\xE9\xA2\x9C\xE8\x89\xB2\x0A", Settings::Radar::rad_Color_VTT);//不确定!!!!!!!!!!!!!!! Color Visible TT
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
			}
			else if (tabSelected == 4) // knifebot
			{
				ImGui::Checkbox("\xE5\x90\xAF\xE5\x8A\xA8\x0A", &Settings::Knifebot::knf_Active);//Active 启动
				ImGui::Checkbox("\xE6\x94\xBB\xE5\x87\xBB\xE9\x98\x9F\xE5\x8F\x8B\x0A", &Settings::Knifebot::knf_Team);//Attack Team 攻击队友

				ImGui::Separator();

				string attack_1 = "\xE8\xBD\xBB\xE5\x87\xBB\x0A";//不确定!!!!!!!!!!!!!!! Attack 1轻击
				string attack_2 = "\xE9\x87\x8D\xE5\x87\xBB\x0A";//不确定!!!!!!!!!!!!!!! Attack 2重击
				string attack_3 = "\xE8\xBD\xBB\xE5\x87\xBB\x2B\xE9\x87\x8D\xE5\x87\xBB\x0A";//不确定!!!!!!!!!!!!!!! Attack 1 + Attack 2 轻击+重击 

				const char* items[] = { attack_1.c_str() , attack_2.c_str() , attack_3.c_str() };
				ImGui::Combo("\xE5\x88\x80\xE4\xBA\xBA\xE6\xA8\xA1\xE5\xBC\x8F\x0A", &Settings::Knifebot::knf_Attack, items, IM_ARRAYSIZE(items));//不确定!!!!!!!!!!!!!!! Attack type 刀人模式

				ImGui::Separator();

				ImGui::SliderInt("\xE5\x8C\xBA\xE5\x88\x86\xE8\xBD\xBB\xE5\x87\xBB\x0A", &Settings::Knifebot::knf_DistAttack, 1, 100);//不确定!!!!!!!!!!!!!!!Dist to attack 1 区分轻击
				ImGui::SliderInt("\xE5\x8C\xBA\xE5\x88\x86\xE9\x87\x8D\xE5\x87\xBB\x0A", &Settings::Knifebot::knf_DistAttack2, 1, 100);//不确定!!!!!!!!!!!!!!!Dist to attack 2 区分重击
			}
			//Fixed by smef#2433 and Troll Face#1136 on discord
			else if (tabSelected == 5) // Skins
			{
				//[enc_string_disable /]
				const char* knife_models_items[] =
				{
					"Default","Bayonet","Flip","Gut","Karambit" ,"M9 Bayonet",
					"Huntsman","Falchion","Bowie","Butterfly","Shadow Daggers"
				};

				const char* quality_items[] =
				{
					"Normal","Genuine","Vintage","Unusual","Community","Developer",
					"Self-Made","Customized","Strange","Completed","Tournament"
				};

				const char* skins_items[] =
				{
					"", "",  "2: Groundwater","3: Candy Apple", "",
					"5: Forest DDPAT","6: Arctic Camo",  "",
					"8: Desert Storm","9: Bengal Tiger","10: Copperhead",
					"11: Skulls","12: Crimson Web","13: Blue Streak","14: Red Laminate"
					,"15: Gunsmoke","16: Jungle Tiger","17: Urban DDPAT",  "",
					"", "20: Virus","21: Granite Marbleized","22: Contrast Spray",
					"",  "", "25: Forest Leaves","26: Lichen Dashed",
					"27: Bone Mask","28: Anodized Navy",  "",  "30: Snake Camo",
					"",  "32: Silver","33: Hot Rod","34: Metallic DDPAT",  "",
					"36: Ossified","37: Blaze","38: Fade","39: Bulldozer","40: Night",
					"41: Copper","42: Blue Steel","43: Stained","44: Case Hardened",
					"",  "46: Contractor","47: Colony","48: Dragon Tattoo",
					"",   "",  "51: Lightning Strike",  "",
					"",   "",   "",   "",   "",
					"",    "59: Slaughter","60: Dark Water"
					,"61: Hypnotic","62: Bloomstick",  "",   ""
					,   "",   "",  "67: Cold Blooded",  "",   ""
					,  "70: Carbon Fiber","71: Scorpion","72: Safari Mesh","73: Wings","74: Polar Camo"
					,"75: Blizzard Marbleized","76: Winter Forest","77: Boreal Forest","78: Forest Night"
					,  "",   "",   "",   "",  "83: Orange DDPAT","84: Pink DDPAT"
					,  "",   "",   "",   "",  "",  "90: Mudder",  ""
					,  "92: Cyanospatter","93: Caramel",  "", "95: Grassland","96: Blue Spruce",  ""
					,  "98: Ultraviolet","99: Sand Dune","100: Storm","101: Tornado","102: Whiteout",  ""
					,  "104: Grassland Leaves",  "",   "", "107: Polar Mesh",  "",   ""
					, "110: Condemned","111: Glacier Mesh",  "",   "",   "",   "",  "116: Sand Mesh",  "",   "", "119: Sage Spray",  "",   "", "122: Jungle Spray",  "",  "124: Sand Spray",  ""
					,  "",  "",  "",  "",  "",  "",  "",  "",  "", "135: Urban Perforated"
					,"136: Waves Perforated",  "",   "",   "",  "", "141: Orange Peel",  "",  "143: Urban Masked", "", "", "", "147: Jungle Dashed"
					,"148: Sand Dashed","149: Urban Dashed", "", "151: Jungle", "", "153: Demolition","154: Afterimage","155: Bullet Rain","156: Death by Kitty","157: Palm","158: Walnut","159: Brass", "", "", "162: Splash", "","164: Modern Hunter","165: Splash Jam","166: Blaze Orange","167: Radiation Hazard","168: Nuclear Threat","169: Fallout Warning","170: Predator","171: Irradiated Alert","172: Black Laminate", "","174: BOOM","175: Scorched","176: Faded Zebra","177: Memento","178: Doomkitty","179: Nuclear Threat","180: Fire Serpent","181: Corticera","182: Emerald Dragon","183: Overgrowth","184: Corticera","185: Golden Koi","186: Wave Spray","187: Zirka","188: Graven","189: Bright Water","190: Black Limba","191: Tempest","192: Shattered","193: Bone Pile","194: Spitfire","195: Demeter","196: Emerald","197: Anodized Navy","198: Hazard","199: Dry Season","200: Mayan Dreams","201: Palm","202: Jungle DDPAT","203: Rust Coat","204: Mosaico","205: Jungle","206: Tornado","207: Facets","208: Sand Dune","209: Groundwater","210: Anodized Gunmetal","211: Ocean Foam","212: Graphite","213: Ocean Foam","214: Graphite","215: X-Ray","216: Blue Titanium","217: Blood Tiger","218: Hexane","219: Hive","220: Hemoglobin","221: Serum","222: Blood in the Water","223: Nightshade","224: Water Sigil","225: Ghost Camo","226: Blue Laminate","227: Electric Hive","228: Blind Spot","229: Azure Zebra","230: Steel Disruption","231: Cobalt Disruption","232: Crimson Web","233: Tropical Storm","234: Ash Wood","235: VariCamo","236: Night Ops","237: Urban Rubble","238: VariCamo Blue", "", "240: CaliCamo","241: Hunting Blind","242: Army Mesh","243: Gator Mesh","244: Teardown","245: Army Recon","246: Amber Fade","247: Damascus Steel","248: Red Quartz","249: Cobalt Quartz","250: Full Stop","251: Pit Viper","252: Silver Quartz","253: Acid Fade","254: Nitro","255: Asiimov","256: The Kraken","257: Guardian","258: Mehndi","259: Redline","260: Pulse","261: Marina","262: Rose Iron","263: Rising Skull","264: Sandstorm","265: Kami","266: Magma","267: Cobalt Halftone","268: Tread Plate","269: The Fuschia Is Now","270: Victoria","271: Undertow","272: Titanium Bit","273: Heirloom","274: Copper Galaxy","275: Red FragCam","276: Panther","277: Stainless","278: Blue Fissure","279: Asiimov","280: Chameleon","281: Corporal","282: Redline","283: Trigon","284: Heat","285: Terrain","286: Antique","287: Pulse","288: Sergeant","289: Sandstorm","290: Guardian","291: Heaven Guard", "", "293: Death Rattle","294: Green Apple","295: Franklin","296: Meteorite","297: Tuxedo","298: Army Sheen","299: Caged Steel","300: Emerald Pinstripe","301: Atomic Alloy","302: Vulcan","303: Isaac","304: Slashed","305: Torque","306: Antique","307: Retribution","308: Kami","309: Howl","310: Curse","311: Desert Warfare","312: Cyrex","313: Orion","314: Heaven Guard","315: Poison Dart","316: Jaguar","317: Bratatat","318: Road Rash","319: Detour","320: Red Python","321: Master Piece","322: Nitro","323: Rust Coat", "", "325: Chalice","326: Knight","327: Chainmail","328: Hand Cannon","329: Dark Age","330: Briar", "", "332: Royal Blue","333: Indigo","334: Twist","335: Module","336: Desert-Strike","337: Tatter","338: Pulse","339: Caiman","340: Jet Set","341: First Class","342: Leather","343: Commuter","344: Dragon Lore","345: First Class","346: Coach Class","347: Pilot","348: Red Leather","349: Osiris","350: Tigris","351: Conspiracy","352: Fowl Play","353: Water Elemental","354: Urban Hazard","355: Desert-Strike","356: Koi","357: Ivory","358: Supernova","359: Asiimov","360: Cyrex","361: Abyss","362: Labyrinth","363: Traveler","364: Business Class","365: Olive Plaid","366: Green Plaid","367: Reactor","368: Setting Sun","369: Nuclear Waste","370: Bone Machine","371: Styx","372: Nuclear Garden","373: Contamination","374: Toxic","375: Radiation Hazard","376: Chemical Green","377: Hot Shot","378: Fallout Warning","379: Cerberus","380: Wasteland Rebel","381: Grinder","382: Murky","383: Basilisk","384: Griffin","385: Firestarter","386: Dart","387: Urban Hazard","388: Cartel","389: Fire Elemental","390: Highwayman","391: Cardiac","392: Delusion","393: Tranquility","394: Cartel","395: Man-o'-war","396: Urban Shock","397: Naga","398: Chatterbox","399: Catacombs","400: 龍王 (Dragon King)","401: System Lock","402: Malachite","403: Deadly Poison","404: Muertos","405: Serenity","406: Grotto","407: Quicksilver","", "409: Tiger Tooth","410: Damascus Steel","411: Damascus Steel", "", "413: Marble Fade","414: Rust Coat","415: Doppler","416: Doppler","417: Doppler","418: Doppler","419: Doppler","420: Doppler","421: Doppler","422: Elite Build","423: Armor Core","424: Worm God","425: Bronze Deco","426: Valence","427: Monkey Business","428: Eco","429: Djinn","430: Hyper Beast","431: Heat","432: Man-o'-war","433: Neon Rider","434: Origami","435: Pole Position","436: Grand Prix","437: Twilight Galaxy","438: Chronos","439: Hades","440: Icarus Fell","441: Minotaur's Labyrinth","442: Asterion","443: Pathfinder","444: Daedalus","445: Hot Rod","446: Medusa","447: Duelist","448: Pandora's Box","449: Poseidon","450: Moon in Libra","451: Sun in Leo","452: Shipping Forecast","453: Emerald","454: Para Green","455: Akihabara Accept","456: Hydroponic","457: Bamboo Print","458: Bamboo Shadow","459: Bamboo Forest","460: Aqua Terrace", "", "462: Counter Terrace","463: Terrace","464: Neon Kimono","465: Orange Kimono","466: Crimson Kimono","467: Mint Kimono","468: Midnight Storm","469: Sunset Storm 壱","470: Sunset Storm 弐","471: Daybreak","472: Impact Drill","473: Seabird","474: Aquamarine Revenge","475: Hyper Beast","476: Yellow Jacket","477: Neural Net","478: Rocket Pop","479: Bunsen Burner","480: Evil Daimyo","481: Nemesis","482: Ruby Poison Dart","483: Loudmouth","484: Ranger","485: Handgun","486: Elite Build","487: Cyrex","488: Riot","489: Torque","490: Frontside Misty","491: Dualing Dragons","492: Survivor Z","493: Flux","494: Stone Cold","495: Wraiths","496: Nebula Crusader","497: Golden Coil","498: Rangeen","499: Cobalt Core","500: Special Delivery","501: Wingshot","502: Green Marine","503: Big Iron","504: Kill Confirmed","505: Scumbria","506: Point Disarray","507: Ricochet","508: Fuel Rod","509: Corinthian","510: Retrobution","511: The Executioner","512: Royal Paladin", "", "514: Power Loader","515: Imperial","516: Shapewood","517: Yorick","518: Outbreak","519: Tiger Moth","520: Avalanche","521: Teclu Burner","522: Fade","523: Amber Fade","524: Fuel Injector","525: Elite Build","526: Photic Zone","527: Kumicho Dragon","528: Cartel","529: Valence","530: Triumvirate", "", "532: Royal Legion","533: The Battlestar","534: Lapis Gator","535: Praetorian","536: Impire","537: Hyper Beast","538: Necropos","539: Jambiya","540: Lead Conduit","541: Fleet Flock","542: Judgement of Anubis","543: Red Astor","544: Ventilators","545: Orange Crash","546: Firefight","547: Spectre","548: Chantico's Fire","549: Bioleak","550: Oceanic","551: Asiimov","552: Fubar","553: Atlas","554: Ghost Crusader","555: Re-Entry","556: Primal Saber","557: Black Tie","558: Lore","559: Lore","560: Lore","561: Lore","562: Lore","563: Black Laminate","564: Black Laminate","565: Black Laminate","566: Black Laminate","567: Black Laminate","568: Gamma Doppler","569: Gamma Doppler","570: Gamma Doppler","571: Gamma Doppler","572: Gamma Doppler","573: Autotronic","574: Autotronic","575: Autotronic","576: Autotronic","577: Autotronic","578: Bright Water","579: Bright Water","580: Freehand","581: Freehand","582: Freehand","583: Aristocrat","584: Phobos","585: Violent Daimyo","586: Wasteland Rebel","587: Mecha Industries","588: Desolate Space","589: Carnivore","590: Exo","591: Imperial Dragon","592: Iron Clad","593: Chopper","594: Harvester","595: Reboot","596: Limelight","597: Bloodsport","598: Aerial","599: Ice Cap","600: Neon Revolution","601: Syd Mead","602: Imprint","603: Directive","604: Roll Cage","605: Scumbria","606: Ventilator","607: Weasel","608: Petroglyph","609: Airlock","610: Dazzle","611: Grim","612: Powercore","613: Triarch","614: Fuel Injector","615: Briefing","616: Slipstream","617: Doppler","618: Doppler","619: Doppler","620: Ultraviolet","621: Ultraviolet","622: Polymer","623: Ironwork","624: Dragonfire","625: Royal Consorts","626: Mecha Industries","627: Cirrus","628: Stinger","629: Black Sand","630: Sand Scale","631: Flashback","632: Buzz Kill","633: Sonar","634: Gila","635: Turf","636: Shallow Grave","637: Cyrex","638: Wasteland Princess","639: Bloodsport","640: Fever Dream","641: Jungle Slipstream","642: Blueprint","643: Xiangliu","644: Decimator","645: Oxide Blaze","646: Capillary","647: Crimson Tsunami","648: Emerald Poison Dart","649: Akoben","650: Ripple","651: Last Dive","652: Scaffold","653: Neo-Noir","654: Seasons","655: Zander","656: Orbit Mk01","657: Blueprint","658: Cobra Strike","659: Macabre","660: Hyper Beast","661: Sugar Rush","662: Oni Taiji","663: Briefing","664: Hellfire","665: Aloha","666: Hard Water","667: Woodsman","668: Red Rock","669: Death Grip","670: Death's Head","671: Cut Out","672: Metal Flowers","673: Morris","674: Triqua","675: The Empress","676: High Roller","677: Hunter","678: See Ya Later","679: Goo","680: Off World","681: Leaded Glass","682: Oceanic","683: Llama Cannon","684: Cracked Opal","685: Jungle Slipstream","686: Phantom","687: Tacticat","688: Exposure","689: Ziggy"
				};

				const char* gloves_listbox_items[25] =
				{
					"default",
					"bloodhound_black_silver","bloodhound_snakeskin_brass","bloodhound_metallic","handwrap_leathery",
					"handwrap_camo_grey","slick_black","slick_military","slick_red","sporty_light_blue","sporty_military",
					"handwrap_red_slaughter","motorcycle_basic_black","motorcycle_mint_triangle","motorcycle_mono_boom",
					"motorcycle_triangle_blue","specialist_ddpat_green_camo","specialist_kimono_diamonds_red",
					"specialist_emerald_web","specialist_orange_white","handwrap_fabric_orange_camo","sporty_purple",
					"sporty_green","bloodhound_guerrilla","slick_snakeskin_yellow"
				};
				//[enc_string_enable /]


				ImGui::Text("\xE5\xBD\x93\xE5\x89\x8D\xE6\xAD\xA6\xE5\x99\xA8: %s", pWeaponData[iWeaponID]);//Current Weapon 当前武器

				ImGui::PushItemWidth(362.f);

				ImGui::Separator();

				ImGui::Combo("Knife CT Model", &Settings::Skin::knf_ct_model, knife_models_items, IM_ARRAYSIZE(knife_models_items));
				ImGui::Combo("Knife TT Model", &Settings::Skin::knf_tt_model, knife_models_items, IM_ARRAYSIZE(knife_models_items));

				ImGui::Combo("Custom CT Knife", &Settings::Skin::knf_ct_skin, skins_items, IM_ARRAYSIZE(skins_items));
				ImGui::Combo("Custom TT Knife", &Settings::Skin::knf_tt_skin, skins_items, IM_ARRAYSIZE(skins_items));

				ImGui::Separator();

				static int iSelectKnifeCTSkinIndex = -1;
				static int iSelectKnifeTTSkinIndex = -1;

				int iKnifeCTModelIndex = Settings::Skin::knf_ct_model;
				int iKnifeTTModelIndex = Settings::Skin::knf_tt_model;

				static int iOldKnifeCTModelIndex = -1;
				static int iOldKnifeTTModelIndex = -1;

				if (iOldKnifeCTModelIndex != iKnifeCTModelIndex && Settings::Skin::knf_ct_model)
					iSelectKnifeCTSkinIndex = GetKnifeSkinIndexFromPaintKit(Settings::Skin::knf_ct_skin, false);

				if (iOldKnifeTTModelIndex != iKnifeTTModelIndex && Settings::Skin::knf_tt_model)
					iSelectKnifeTTSkinIndex = GetKnifeSkinIndexFromPaintKit(Settings::Skin::knf_ct_skin, true);

				iOldKnifeCTModelIndex = iKnifeCTModelIndex;
				iOldKnifeTTModelIndex = iKnifeTTModelIndex;

				string KnifeCTModel = knife_models_items[Settings::Skin::knf_ct_model];
				string KnifeTTModel = knife_models_items[Settings::Skin::knf_tt_model];

				KnifeCTModel += " Skin##KCT";
				KnifeTTModel += " Skin##KTT";

				ImGui::SliderFloat("CT\xE9\x98\xB5\xE8\x90\xA5\xE5\x88\x80\xE7\xA3\xA8\xE6\x8D\x9F", &g_SkinChangerCfg[WEAPON_KNIFE].flFallbackWear, 0.001f, 1.f);//CT阵营刀磨损
				ImGui::Combo("CT\xE9\x98\xB5\xE8\x90\xA5\xE5\x88\x80\xE5\x93\x81\xE8\xB4\xA8", &g_SkinChangerCfg[WEAPON_KNIFE].iEntityQuality, quality_items, IM_ARRAYSIZE(quality_items));//CT阵营刀品质
				ImGui::ComboBoxArray(KnifeCTModel.c_str(), &iSelectKnifeCTSkinIndex, KnifeSkins[iKnifeCTModelIndex].SkinNames);

				ImGui::Separator();

				ImGui::SliderFloat("T\xE9\x98\xB5\xE8\x90\xA5\xE5\x88\x80\xE7\xA3\xA8\xE6\x8D\x9F", &g_SkinChangerCfg[WEAPON_KNIFE_T].flFallbackWear, 0.001f, 1.f);//T阵营刀磨损
				ImGui::Combo("T\xE9\x98\xB5\xE8\x90\xA5\xE5\x88\x80\xE5\x93\x81\xE8\xB4\xA8", &g_SkinChangerCfg[WEAPON_KNIFE_T].iEntityQuality, quality_items, IM_ARRAYSIZE(quality_items));//T阵营刀品质
				ImGui::ComboBoxArray(KnifeTTModel.c_str(), &iSelectKnifeTTSkinIndex, KnifeSkins[iKnifeTTModelIndex].SkinNames);

				ImGui::Separator();

				static int iOldWeaponID = -1;

				ImGui::Combo("\xE6\xAD\xA6\xE5\x99\xA8\xE9\x80\x89\xE6\x8B\xA9", &iWeaponID, pWeaponData, IM_ARRAYSIZE(pWeaponData));//武器选择Weapon##WeaponSelect

				iWeaponSelectIndex = pWeaponItemIndexData[iWeaponID];

				if (iOldWeaponID != iWeaponID)
					iWeaponSelectSkinIndex = GetWeaponSkinIndexFromPaintKit(g_SkinChangerCfg[iWeaponSelectIndex].nFallbackPaintKit);

				iOldWeaponID = iWeaponID;

				string WeaponSkin = pWeaponData[iWeaponID];
				WeaponSkin += " Skin";

				ImGui::ComboBoxArray(WeaponSkin.c_str(), &iWeaponSelectSkinIndex, WeaponSkins[iWeaponID].SkinNames);

				ImGui::Combo("\xE6\xAD\xA6\xE5\x99\xA8\xE5\x93\x81\xE8\xB4\xA8", &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].iEntityQuality, quality_items, IM_ARRAYSIZE(quality_items));//Weapon Quality武器品质
				ImGui::SliderFloat("\xE6\xAD\xA6\xE5\x99\xA8\xE7\xA3\xA8\xE6\x8D\x9F", &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].flFallbackWear, 0.001f, 1.f);//Weapon Wear武器磨损
				ImGui::InputInt("StatTrak", &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].nFallbackStatTrak, 1, 100, ImGuiInputTextFlags_CharsDecimal);//StatTrak
				ImGui::Combo("\xE7\x9A\xAE\xE8\x82\xA4", &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].nFallbackPaintKit, skins_items, IM_ARRAYSIZE(skins_items));//皮肤

				ImGui::Separator();

				ImGui::Combo("\xE6\x89\x8B\xE5\xA5\x97\xE7\x9A\xAE\xE8\x82\xA4", &Settings::Skin::gloves_skin, gloves_listbox_items,//Gloves Skin手套皮肤
					IM_ARRAYSIZE(gloves_listbox_items));

				ImGui::Separator();

				ImGui::PopItemWidth();

				if (ImGui::Button("Apply##Skin"))
				{
					if (iWeaponSelectSkinIndex >= 0) {
						g_SkinChangerCfg[iWeaponSelectIndex].nFallbackPaintKit = WeaponSkins[iWeaponID].SkinPaintKit[iWeaponSelectSkinIndex];
					}
					if (iSelectKnifeCTSkinIndex > 0) {
						Settings::Skin::knf_ct_skin = KnifeSkins[iKnifeCTModelIndex].SkinPaintKit[iSelectKnifeCTSkinIndex];
					}
					if (iSelectKnifeTTSkinIndex > 0) {
						Settings::Skin::knf_tt_skin = KnifeSkins[iKnifeTTModelIndex].SkinPaintKit[iSelectKnifeTTSkinIndex];
					}
					ForceFullUpdate();
				}
			}
			else if (tabSelected == 6)
			{
				ImGui::InputText("Skyname", Settings::Misc::misc_SkyName, 64);//
				ImGui::Checkbox("\xE5\x85\xB3\xE9\x97\xAD\xE5\x90\x8E\xE6\x9C\x9F\xE5\xA4\x84\xE7\x90\x86", &Settings::Misc::misc_Postprocess);//disnable PostProess关闭后期处理
				ImGui::Checkbox("\xE5\xBC\x80\xE5\x90\xAF\xE5\x90\x8E\xE6\x9C\x9F\xE5\xA4\x84\xE7\x90\x86", &Settings::Misc::misc_EPostprocess);//Enable PostProess开启后期处理
				ImGui::Checkbox("\xE8\xBF\x9E\xE8\xB7\xB3\x0A", &Settings::Misc::misc_Bhop);//Bhop 连跳
				ImGui::Checkbox("\xE5\x87\xBB\xE4\xB8\xAD\xE8\xA1\xA5\xE5\x81\xBF\x0A", &Settings::Misc::misc_Punch);//Punch 击中补偿
				ImGui::Checkbox("\xE7\x8B\x99\xE5\x87\xBB\xE6\x9E\xAA\xE5\x87\x86\xE5\xBF\x83", &Settings::Misc::misc_SniperAim);//SniperAim 狙击准星
				ImGui::Checkbox("\xE6\x97\xA0\xE9\x97\xAA\xE5\x85\x89\x0A", &Settings::Misc::misc_NoFlash);//NoFlash 无闪光
				ImGui::Checkbox("\xE6\x97\xA0\xE7\x83\x9F\xE9\x9B\xBE", &Settings::Misc::misc_NoSmoke);//NoSmoke 无烟雾
				ImGui::Checkbox("\xE8\x87\xAA\xE5\x8A\xA8\xE6\x89\xAB\xE5\xB0\x84\x0A", &Settings::Misc::misc_AutoStrafe);//AutoStrafe 自动扫射
				ImGui::Checkbox("\xE8\x87\xAA\xE5\x8A\xA8\xE7\xA1\xAE\xE8\xAE\xA4\x0A", &Settings::Misc::misc_AutoAccept);//AutoAccept 自动确认
				ImGui::Checkbox("\xE8\xA7\x82\xE5\xAF\x9F\xE8\x80\x85\xE5\x88\x97\xE8\xA1\xA8\x0A", &Settings::Misc::misc_Spectators);//Spectators 观察者列表
				ImGui::Checkbox("\xE8\xB7\x9D\xE7\xA6\xBB\x0A", &Settings::Misc::misc_FovChanger);//Fov Changer 距离开关
				ImGui::PushItemWidth(362.f);
				ImGui::SliderInt("\xE8\xA7\x86\xE9\x87\x8E\xE8\x8C\x83\xE5\x9B\xB4\x0A", &Settings::Misc::misc_FovView, 1, 190);//Fov View 视野范围
				ImGui::SliderInt("\xE6\xA8\xA1\xE5\x9E\x8B\xE8\xB7\x9D\xE7\xA6\xBB\x0A", &Settings::Misc::misc_FovModelView, 1, 190);//Fov Model View 模型距离
				ImGui::Separator();
				ImGui::ColorEdit3("\xE5\x87\x86\xE5\xBF\x83\xE9\xA2\x9C\xE8\x89\xB2", Settings::Misc::misc_AwpAimColor);//Awp Aim Color AWP自瞄颜色
				ImGui::PopItemWidth();
			}
			else if (tabSelected == 7) // Config
			{
				static int iConfigSelect = 0;
				static int iMenuSheme = 1;
				static char ConfigName[64] = { 0 };

				ImGui::ComboBoxArray("\xE9\x80\x89\xE6\x8B\xA9\xE9\x85\x8D\xE7\xBD\xAE\x0A", &iConfigSelect, ConfigList);//Select Config 选择配置

				ImGui::Separator();

				if (ImGui::Button("\xE8\xBD\xBD\xE5\x85\xA5\xE9\x85\x8D\xE7\xBD\xAE\x0A") && ConfigList[iConfigSelect] != "")//Load Config 载入配置
				{
					Settings::LoadSettings(BaseDir + "\\" + ConfigList[iConfigSelect]);
				}
				ImGui::SameLine();
				if (ImGui::Button("\xE4\xBF\x9D\xE5\xAD\x98\xE9\x85\x8D\xE7\xBD\xAE\x0A") && ConfigList[iConfigSelect] != "")//Save Config 保存配置
				{
					Settings::SaveSettings(BaseDir + "\\" + ConfigList[iConfigSelect]);
				}
				ImGui::SameLine();
				if (ImGui::Button("\xE5\x88\xB7\xE6\x96\xB0\xE9\x85\x8D\xE7\xBD\xAE\xE5\x88\x97\xE8\xA1\xA8\x0A"))//Refresh Config List 刷新配置列表
				{
					RefreshConfigs();
				}

				ImGui::Separator();

				ImGui::InputText("\xE9\x85\x8D\xE7\xBD\xAE\xE5\x90\x8D\xE5\xAD\x97\x0A", ConfigName, 64);//Config Name 配置名字

				if (ImGui::Button("\xE5\x88\x9B\xE5\xBB\xBA\x26\xE4\xBF\x9D\xE5\xAD\x98\xE6\x96\xB0\xE9\x85\x8D\xE7\xBD\xAE\x0A"))//Create & Save new Config 创建&保存新配置
				{
					string ConfigFileName = ConfigName;

					if (ConfigFileName.size() < 1)
					{
						ConfigFileName = "settings";
					}

					Settings::SaveSettings(BaseDir + "\\" + ConfigFileName + ".ini");
					RefreshConfigs();
				}

				ImGui::Separator();

				const char* ThemesList[] = { "Purple" , "Default" , "Light Pink" , "Dark Blue" , "MidNight" , "Night" , "Dunno" , "Blue"  , "Black" , "Green" , "Yellow" , "Light Blue" , "Light Grey" , "pHooK" };

				ImGui::Combo("\xE6\x8E\xA5\xE5\x8F\x97\xE6\x96\xB9\xE6\xA1\x88\x0A", &iMenuSheme, ThemesList, IM_ARRAYSIZE(ThemesList));//Apply Sheme 接受方案

				ImGui::Separator();

				if (ImGui::Button("Apply Color"))
				{
					if (iMenuSheme == 0)
					{
						g_pGui->RedSheme();
					}
				}
			}

			ImGui::End();
		}
	}
}