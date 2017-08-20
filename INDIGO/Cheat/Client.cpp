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

	Vector2D	g_vCenterScreen = Vector2D( 0.f , 0.f );

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

	void ReadConfigs( LPCTSTR lpszFileName )
	{
		if ( !strstr( lpszFileName , "gui.ini" ) )
		{
			ConfigList.push_back( lpszFileName );
		}
	}

	void RefreshConfigs()
	{
		ConfigList.clear();
		string ConfigDir = BaseDir + "\\*.ini";
		SearchFiles( ConfigDir.c_str() , ReadConfigs , FALSE );
	}

	bool Initialize( IDirect3DDevice9* pDevice )
	{
		g_pPlayers = new CPlayers();
		g_pRender = new CRender( pDevice );
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

		Settings::LoadSettings( IniFile );

		iWeaponSelectSkinIndex = GetWeaponSkinIndexFromPaintKit( g_SkinChangerCfg[iWeaponSelectIndex].nFallbackPaintKit );

		g_pGui->GUI_Init( pDevice );

		RefreshConfigs();

		return true;
	}

	void Shutdown()
	{
		DELETE_MOD( g_pPlayers );
		DELETE_MOD( g_pRender );
		DELETE_MOD( g_pGui );

		DELETE_MOD( g_pAimbot );
		DELETE_MOD( g_pTriggerbot );
		DELETE_MOD( g_pEsp );
		DELETE_MOD( g_pRadar );
		DELETE_MOD( g_pKnifebot );
		DELETE_MOD( g_pSkin );
		DELETE_MOD( g_pMisc );
	}

	void OnRender()
	{
		if ( g_pRender && !Interfaces::Engine()->IsTakingScreenshot() && Interfaces::Engine()->IsActiveApp() )
		{
			g_pRender->BeginRender();

			if ( g_pGui )
				g_pGui->GUI_Draw_Elements();

			Interfaces::Engine()->GetScreenSize( iScreenWidth , iScreenHeight );

			g_vCenterScreen.x = iScreenWidth / 2.f;
			g_vCenterScreen.y = iScreenHeight / 2.f;
			
			if ( !Interfaces::Engine()->IsConnected() )
				g_pRender->Text( 3 , 3 , false , true , Color::Aqua() , HACK_NAME );

			if ( Client::g_pPlayers && Client::g_pPlayers->GetLocal() && Interfaces::Engine()->IsInGame() )
			{
				if ( g_pEsp )
					g_pEsp->OnRender();

				if ( g_pMisc )
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
		if ( g_pRender )
			g_pRender->OnLostDevice();

		if ( g_pGui )
			ImGui_ImplDX9_InvalidateDeviceObjects();
	}

	void OnResetDevice()
	{
		if ( g_pRender )
			g_pRender->OnResetDevice();

		if ( g_pGui )
			ImGui_ImplDX9_CreateDeviceObjects();
	}

	void OnCreateMove( CUserCmd* pCmd )
	{
		if ( g_pPlayers && Interfaces::Engine()->IsInGame() )
		{
			g_pPlayers->Update();

			if ( g_pEsp )
				g_pEsp->OnCreateMove( pCmd );

			if ( IsLocalAlive() )
			{
				if ( !bIsGuiVisible )
				{
					int iWeaponSettingsSelectID = GetWeaponSettingsSelectID();

					if ( iWeaponSettingsSelectID >= 0 )
						iWeaponID = iWeaponSettingsSelectID;
				}

				if ( g_pAimbot )
					g_pAimbot->OnCreateMove( pCmd , g_pPlayers->GetLocal() );

				if ( g_pTriggerbot )
					g_pTriggerbot->OnCreateMove( pCmd , g_pPlayers->GetLocal() );

				if ( g_pKnifebot )
					g_pKnifebot->OnCreateMove( pCmd );

				if ( g_pMisc )
					g_pMisc->OnCreateMove( pCmd );
			}
		}
	}

	void OnFireEventClientSideThink( IGameEvent* pEvent )
	{
		if ( !strcmp( pEvent->GetName() , "player_connect_full" ) ||
			 !strcmp( pEvent->GetName() , "round_start" ) ||
			 !strcmp( pEvent->GetName() , "cs_game_disconnected" ) )
		{
			if ( g_pPlayers )
				g_pPlayers->Clear();

			if ( g_pEsp )
				g_pEsp->OnReset();
		}

		if ( Interfaces::Engine()->IsConnected() )
		{
			if ( g_pEsp )
				g_pEsp->OnEvents( pEvent );

			if ( g_pSkin )
				g_pSkin->OnEvents( pEvent );
		}
	}

	void OnFrameStageNotify( ClientFrameStage_t Stage )
	{
		if ( Interfaces::Engine()->IsInGame() )
		{
			Skin_OnFrameStageNotify( Stage );
			Gloves_OnFrameStageNotify( Stage );
		}
	}

	void OnDrawModelExecute( IMatRenderContext* ctx , const DrawModelState_t &state ,
							 const ModelRenderInfo_t &pInfo , matrix3x4_t *pCustomBoneToWorld )
	{
		if ( Interfaces::Engine()->IsInGame() && ctx && pCustomBoneToWorld )
		{
			if ( g_pEsp )
				g_pEsp->OnDrawModelExecute( ctx , state , pInfo , pCustomBoneToWorld );

			if ( g_pMisc )
				g_pMisc->OnDrawModelExecute();
		}
	}

	void OnPlaySound( const Vector* pOrigin , const char* pszSoundName )
	{
		if ( !pszSoundName || !Interfaces::Engine()->IsInGame() )
			return;

		if ( !strstr( pszSoundName , "bulletLtoR" ) &&
			 !strstr( pszSoundName , "rics/ric" ) &&
			 !strstr( pszSoundName , "impact_bullet") )
		{
			if ( g_pEsp && IsLocalAlive() && Settings::Esp::esp_Sound && pOrigin )
			{
				if ( !GetVisibleOrigin( *pOrigin ) )
					g_pEsp->SoundEsp.AddSound( *pOrigin );
			}
		}
	}

	void OnPlaySound( const char* pszSoundName )
	{
		if ( g_pMisc )
			g_pMisc->OnPlaySound( pszSoundName );
	}

	void OnOverrideView( CViewSetup* pSetup )
	{
		if ( g_pMisc )
			g_pMisc->OnOverrideView( pSetup );
	}

	void OnGetViewModelFOV( float& fov )
	{
		if ( g_pMisc )
			g_pMisc->OnGetViewModelFOV( fov );
	}

	void OnRenderGUI()
	{
		ImGui::SetNextWindowSize( ImVec2( 560.f , 325.f ) );

		if ( ImGui::Begin( HACK_NAME , &bIsGuiVisible , ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize ) )
		{
			if ( Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_FovType > 1 )
				Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_FovType = 1;

			if ( Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_BestHit > 1 )
				Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_BestHit = 1;

			if ( Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Spot > 5 )
				Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Spot = 5;

			const char* tabNames[] = { 
				AIMBOT_TEXT , TRIGGER_TEXT , VISUAL_TEXT , RADAR_TEXT , 
				KNIFEBOT_TEXT , SKIN_TEXT , MISC_TEXT , CONFIG_TEXT };
			//Skin changer is damaged... need futher repair
			
			static int tabOrder[] = { 0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 };
			static int tabSelected = 0;
			const bool tabChanged = ImGui::TabLabels( tabNames ,
													  sizeof( tabNames ) / sizeof( tabNames[0] ) ,
													  tabSelected , tabOrder );

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			float SpaceLineOne = 120.f;
			float SpaceLineTwo = 220.f;
			float SpaceLineThr = 320.f;

			if ( tabSelected == 0 ) // Aimbot
			{
				ImGui::PushItemWidth( 110.f );
				ImGui::Text( "\xe5\xbd\x93\xe5\x89\x8d\xe6\xad\xa6\xe5\x99\xa8: " );//Current Weapon 当前武器
				ImGui::SameLine();
				ImGui::Combo( "##\xE6\xAD\xA6\xE5\x99\xA8\xE8\x87\xAA\xE7\x9E\x84\x0A" , &iWeaponID , pWeaponData , IM_ARRAYSIZE( pWeaponData ) );//不确定!!!!!!!!!!!!!!!AimWeapon 武器自瞄
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox( "\xe6\xad\xbb\xe4\xba\xa1\xe7\xab\x9e\xe8\xb5\x9b\xe6\xa8\xa1\xe5\xbc\x8f" , &Settings::Aimbot::aim_Deathmatch ); //Deathmatch 死亡竞赛
				ImGui::SameLine( SpaceLineOne );
				ImGui::Checkbox( "\xe7\xa9\xbf\xe5\xa2\x99" , &Settings::Aimbot::aim_WallAttack );//WallAttack 穿墙
				ImGui::SameLine( SpaceLineTwo );
				ImGui::Checkbox( "\xe6\xa3\x80\xe6\xb5\x8b\xe7\x83\x9f\xe9\x9b\xbe" , &Settings::Aimbot::aim_CheckSmoke );//CheckSmoke 检测烟雾

				ImGui::Checkbox( "\xe5\x8f\x8d\xe8\xb7\xb3\xe8\xb7\x83" , &Settings::Aimbot::aim_AntiJump );//AntiJump 反跳跃
				ImGui::SameLine( SpaceLineOne );
				ImGui::Checkbox( "\xE7\xBB\x98\xE5\x88\xB6\xE7\x9E\x84\xE5\x87\x86\xE8\x8C\x83\xE5\x9B\xB4" , &Settings::Aimbot::aim_DrawFov );//Draw Fov 绘制瞄准范围
				ImGui::SameLine( SpaceLineTwo );
				ImGui::Checkbox( "\xE7\xBB\x98\xE5\x88\xB6\xE7\x9E\x84\xE5\x87\x86\xE7\x82\xB9" , &Settings::Aimbot::aim_DrawSpot );//DrawSpot 绘制瞄准点

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox( "\xe5\x90\xaf\xe7\x94\xa8" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Active );//aim Active 启用
								
				if ( iWeaponID <= 9 )
				{
					ImGui::SameLine();
					ImGui::Checkbox( "\xE8\x87\xAA\xE5\x8A\xA8\xE6\x89\x8B\xE6\x9E\xAA\x0A" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_AutoPistol );//Autopistol 自动手枪
				}

				ImGui::PushItemWidth( 362.f );
				ImGui::SliderInt( "\xE5\xB9\xB3\xE6\xBB\x91\x0A" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Smooth , 1 , 300 );//Smooth 平滑
				ImGui::SliderInt( "Fov" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Fov , 1 , 300 );
				ImGui::PopItemWidth();

				const char* AimFovType[] = { "\xE5\x8A\xA8\xE6\x80\x81\x0A" , "\xE9\x9D\x99\xE6\x80\x81\x0A" };//Dynamic Static 动态 静态
				ImGui::PushItemWidth( 362.f );
				ImGui::Combo( "\xE8\x8C\x83\xE5\x9B\xB4\xE7\xB1\xBB\xE5\x9E\x8B\x0A" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_FovType , AimFovType , IM_ARRAYSIZE( AimFovType ) ); //不确定!!!!!!!!!!!!!!!Fov Type 范围类型
				ImGui::PopItemWidth();

				const char* BestHit[] = { "\xE5\x85\xB3\xE9\x97\xAD\x0A" , "\xE5\xBC\x80\xE5\x90\xAF\x0A" };//关闭 开启
				ImGui::PushItemWidth( 362.f );
				ImGui::Combo( "\xE7\xB2\xBE\xE5\x87\x86\xE5\xB0\x84\xE5\x87\xBB\x0A" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_BestHit , BestHit , IM_ARRAYSIZE( BestHit ) ); //BestHit 精准射击
				
				if ( ImGui::IsItemHovered() )
					ImGui::SetTooltip( "\xE5\x85\xB3\xE9\x97\xAD\xE4\xBB\xA5\xE4\xBD\xBF\xE7\x94\xA8\xE8\x87\xAA\xE7\x9E\x84\xE7\x82\xB9\x0A" );//if disabled then used Aimspot 关闭以使用自瞄点

				ImGui::PopItemWidth();

				const char* Aimspot[] = { "\xE5\xA4\xB4\x0A" , "\xE8\x84\x96\xE5\xAD\x90\x0A" , "\xE9\x94\x81\xE9\xAA\xA8\x0A" , "\xE8\xBA\xAB\xE4\xBD\x93\x0A" , "\xE8\x83\xB8\xE9\x83\xA8\x0A" , "\xE8\x85\xB9\xE9\x83\xA8\x0A" };//"Head" , "Neck" , "Low Neck" , "Body" , "Thorax" , "Chest" 头 脖子 锁骨 身体 胸部 腹部
				ImGui::PushItemWidth( 362.f );
				ImGui::Combo( "\xE8\x87\xAA\xE7\x9E\x84\xE7\x82\xB9\x0A" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Spot , Aimspot , IM_ARRAYSIZE( Aimspot ) );//Aimspot 自瞄点
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::PushItemWidth( 362.f );
				ImGui::SliderInt( "\xE5\xB0\x84\xE5\x87\xBB\xE5\xBB\xB6\xE6\x97\xB6\x0A" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Delay , 0 , 200 );//ShotDelay 射击延时
				ImGui::SliderInt( "\xE5\x90\x8E\xE5\x9D\x90\xE5\x8A\x9B\xE6\x8E\xA7\xE5\x88\xB6\x0A" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Rcs , 0 , 100 );//Rcs 后坐力控制
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				if ( iWeaponID >= 10 && iWeaponID <= 30 )
				{
					ImGui::PushItemWidth( 362.f );
					ImGui::SliderInt( "\xE5\x90\x8E\xE5\x9D\x90\xE5\x8A\x9B\xE6\x8E\xA7\xE5\x88\xB6\xE8\x8C\x83\xE5\x9B\xB4" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_RcsFov , 1 , 300 );//Rcs Fov 后坐力控制范围
					ImGui::SliderInt( "\xE5\x90\x8E\xE5\x9D\x90\xE5\x8A\x9B\xE6\x8E\xA7\xE5\x88\xB6\xE5\xB9\xB3\xE6\xBB\x91" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_RcsSmooth , 1 , 300 );//Rcs Smooth 后坐力控制平滑
					ImGui::PopItemWidth();

					const char* ClampType[] = { "\xE5\x85\xA8\xE9\x83\xA8\xE6\x95\x8C\xE4\xBA\xBA\x0A" , "Shot" , "Shot + Target" };//"All Target" , "Shot" , "Shot + Target"全部敌人 射击 射击+扳机
					ImGui::PushItemWidth( 362.f );
					ImGui::Combo( "Rcs Clamp" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_RcsClampType , ClampType , IM_ARRAYSIZE( ClampType ) );//不确定!!!!!!!!!!!!!!!后坐力控制 固定类型
					ImGui::PopItemWidth();

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();
				}
			}
			else if ( tabSelected == 1 ) // Trigger
			{
				const char* TriggerEnable[] = { "\xE5\x85\xB3\xE9\x97\xAD\x0A" , "\xE8\x8C\x83\xE5\x9B\xB4\x0A" , "\xE8\xBF\xBD\xE8\xB8\xAA\x0A" };//不确定!!!!!!!!!!!!!!!关闭 范围 跟踪
				ImGui::PushItemWidth( 80.f );
				ImGui::Combo( "\xE5\xBC\x80\xE5\x90\xAF\x0A" , &Settings::Triggerbot::trigger_Enable , TriggerEnable , IM_ARRAYSIZE( TriggerEnable ) );//不确定!!!!!!!!!!!!!!!开启
				ImGui::PopItemWidth();
				ImGui::SameLine();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox( "\xe6\xad\xbb\xe4\xba\xa1\xe7\xab\x9e\xe8\xb5\x9b\xe6\xa8\xa1\xe5\xbc\x8f" , &Settings::Triggerbot::trigger_Deathmatch );//Deathmatch 死亡竞赛
				ImGui::SameLine( SpaceLineOne );
				ImGui::Checkbox( "\xe7\xa9\xbf\xe5\xa2\x99" , &Settings::Triggerbot::trigger_WallAttack );//WallAttack 穿墙
				ImGui::SameLine( SpaceLineTwo );
				ImGui::Checkbox( "\xE5\xBF\xAB\xE9\x80\x9F\xE7\xBC\xA9\xE6\x94\xBE\x0A" , &Settings::Triggerbot::trigger_FastZoom );//FastZoom 快速缩放
				
				ImGui::Checkbox( "\xE7\x83\x9F\xE9\x9B\xBE\xE6\xA3\x80\xE6\xB5\x8B\x0A" , &Settings::Triggerbot::trigger_SmokCheck );//SmokeCheck 烟雾检测
				ImGui::SameLine( SpaceLineOne );
				ImGui::Checkbox( "\xE7\x9E\x84\xE5\x87\x86\xE7\x82\xB9\xE8\x8C\x83\xE5\x9B\xB4\x0A" , &Settings::Triggerbot::trigger_DrawFov );//DrawFov 瞄准点范围
				ImGui::SameLine( SpaceLineTwo );
				ImGui::Checkbox( "\xE7\x9E\x84\xE5\x87\x86\xE7\x82\xB9\x0A" , &Settings::Triggerbot::trigger_DrawSpot );//DrawSpot 瞄准点
				ImGui::SameLine( SpaceLineThr );
				ImGui::Checkbox( "\xE7\x9E\x84\xE5\x87\x86\xE7\x82\xB9\xE5\x8D\x8F\xE5\x8A\xA9\x0A" , &Settings::Triggerbot::trigger_DrawFovAssist );//DrawFovAssist 瞄准点协助

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				
				const char* items1[] = { CVAR_KEY_MOUSE3 , CVAR_KEY_MOUSE4 , CVAR_KEY_MOUSE5 };
				ImGui::PushItemWidth( 80.f );
				ImGui::Combo( "Key" , &Settings::Triggerbot::trigger_Key , items1 , IM_ARRAYSIZE( items1 ) );//不确定!!!!!!!!!!!!!!!key
				ImGui::PopItemWidth();
				ImGui::SameLine();

				const char* items2[] = { "Hold" , "Press" };//不确定!!!!!!!!!!!!!!!
				ImGui::PushItemWidth( 80.f );
				ImGui::Combo( "Key Mode" , &Settings::Triggerbot::trigger_KeyMode , items2 , IM_ARRAYSIZE( items2 ) );//不确定!!!!!!!!!!!!!!!key mode
				ImGui::PopItemWidth();
				ImGui::SameLine();

				ImGui::PushItemWidth( 110.f );
				ImGui::Combo( "\xE6\xAD\xA6\xE5\x99\xA8\x0A" , &iWeaponID , pWeaponData , IM_ARRAYSIZE( pWeaponData ) );//不确定!!!!!!!!!!!!!!!weapon武器
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				
				ImGui::PushItemWidth( 362.f );
				ImGui::SliderInt( "\xE6\x9C\x80\xE5\xB0\x8F\xE8\xB7\x9D\xE7\xA6\xBB\x0A" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_DistanceMin , 0 , 5000 );//不确定!!!!!!!!!!!!!!!Min Disstance最小距离
				ImGui::SliderInt( "\xE6\x9C\x80\xE5\xA4\xA7\xE8\xB7\x9D\xE7\xA6\xBB\x0A" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_DistanceMax , 0 , 5000 );//不确定!!!!!!!!!!!!!!!Max Disstance最大距离
				ImGui::SliderInt( "\xE8\x8C\x83\xE5\x9B\xB4\x0A" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_Fov , 1 , 100 );//不确定!!!!!!!!!!!!!!!Fov范围
				ImGui::SliderInt( "\xE5\xBB\xB6\xE6\x97\xB6\xE5\x89\x8D\x0A" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_DelayBefore , 0 , 200 );//不确定!!!!!!!!!!!!!!!Delay Before延时之前
				ImGui::SliderInt( "\xE5\xBB\xB6\xE6\x97\xB6\xE5\x90\x8E\x0A" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_DelayAfter , 0 , 1000 );//不确定!!!!!!!!!!!!!!!Delay After延时之后
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox( "\xE5\x8F\xAA\xE7\x9E\x84\xE5\xA4\xB4\x0A" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_HeadOnly );//HeadOnly 只瞄头
				ImGui::SameLine();

				const char* AssistMode[] = { "Disable" , "One Shot" , "Auto" };//不确定!!!!!!!!!!!!!!!
				ImGui::PushItemWidth( 80.f );
				ImGui::Combo( "\xE5\x90\xB8\xE9\x99\x84\x0A" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_Assist , AssistMode , IM_ARRAYSIZE( AssistMode ) );//不确定!!!!!!!!!!!!!!!Assist 吸附
				ImGui::PopItemWidth();
				ImGui::SameLine();

				const char* AssistFovType[] = { "Dynamic" , "Static" };//不确定!!!!!!!!!!!!!!!
				ImGui::PushItemWidth( 80.f );
				ImGui::Combo( "\xE5\x90\xB8\xE9\x99\x84\xE7\xB1\xBB\xE5\x9E\x8B\x0A" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_AssistFovType , AssistFovType , IM_ARRAYSIZE( AssistFovType ) );//不确定!!!!!!!!!!!!!!!Assist Fov Type 吸附类型
				ImGui::PopItemWidth();

				const char* HitGroup[] = { "All" , "Head + Body" , "Head" };
				ImGui::Combo( "HitGroup" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_HitGroup , HitGroup , IM_ARRAYSIZE( HitGroup ) );
				//ImGui::PopItemWidth();
				
				ImGui::PushItemWidth( 362.f );
				ImGui::SliderInt( "\xE5\x90\xB8\xE9\x99\x84\xE5\x90\x8E\xE5\x9D\x90\xE5\x8A\x9B\x0A" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_AssistRcs , 0 , 100 );//不确定!!!!!!!!!!!!!!! Assist Rcs 吸附后坐力
				ImGui::SliderInt("\xE5\x90\xB8\xE9\x99\x84\xE8\x8C\x83\xE5\x9B\xB4\x0A", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_AssistFov, 1, 300);//不确定!!!!!!!!!!!!!!! Assist Fov 吸附范围
				ImGui::SliderInt("\xE5\x90\xB8\xE9\x99\x84\xE5\xB9\xB3\xE6\xBB\x91\x0A", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_AssistSmooth, 1, 300);//不确定!!!!!!!!!!!!!!! Assist Smooth 吸附平滑
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
			}
			else if ( tabSelected == 2 ) // Visuals
			{
				string style_1 = "\xE6\x96\xB9\xE6\xA1\x86\x0A";//不确定!!!!!!!!!!!!!!!Box 方框
				string style_2 = "\xE5\xA1\xAB\xE5\x85\x85\xE6\xA1\x86\x0A";//不确定!!!!!!!!!!!!!!!CoalBox 填充框

				const char* items1[] = { style_1.c_str() , style_2.c_str() };

				ImGui::PushItemWidth( 339.f );
				ImGui::Combo( "\xE9\x80\x8F\xE8\xA7\x86\xE7\xB1\xBB\xE5\x9E\x8B\x0A" , &Settings::Esp::esp_Style , items1 , IM_ARRAYSIZE( items1 ) );//不确定!!!!!!!!!!!!!!! Esp Type 透视类型
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox( "\xE9\x98\x9F\xE5\x8F\x8B\x0A" , &Settings::Esp::esp_Team );//Esp Team 队友
				ImGui::SameLine( SpaceLineOne );
				ImGui::Checkbox( "\xE6\x95\x8C\xE4\xBA\xBA\x0A" , &Settings::Esp::esp_Enemy );//Esp Enemy 敌人
				ImGui::SameLine( SpaceLineTwo );
				ImGui::Checkbox( "\xE7\x82\xB8\xE5\xBC\xB9\x0A" , &Settings::Esp::esp_Bomb );//Esp Bomp 炸弹
				ImGui::SameLine( SpaceLineThr );
				ImGui::Checkbox( "\xE8\x84\x9A\xE6\xAD\xA5\xE8\xBD\xA8\xE8\xBF\xB9\x0A" , &Settings::Esp::esp_Sound );//Esp Sound 脚步轨迹

				ImGui::Checkbox( "\xE7\x9B\xB4\xE7\xBA\xBF\x0A" , &Settings::Esp::esp_Line );//Esp Line 直线
				ImGui::SameLine( SpaceLineOne );
				ImGui::Checkbox( "\xE8\xBD\xAE\xE5\xBB\x93\x0A" , &Settings::Esp::esp_Outline );//Esp OutLine 轮廓
				ImGui::SameLine( SpaceLineTwo );
				ImGui::Checkbox( "\xE5\x90\x8D\xE5\xAD\x97\x0A" , &Settings::Esp::esp_Name );//Esp Name 名字
				ImGui::SameLine( SpaceLineThr );
				ImGui::Checkbox( "\xE6\xAE\xB5\xE4\xBD\x8D\x0A" , &Settings::Esp::esp_Rank );//Esp Rank 段位

				ImGui::Checkbox( "\xE6\xAD\xA6\xE5\x99\xA8\x0A" , &Settings::Esp::esp_Weapon );//Esp Weapon 武器
				ImGui::SameLine( SpaceLineOne );
				ImGui::Checkbox( "\xE5\xBC\xB9\xE8\x8D\xAF\x0A" , &Settings::Esp::esp_Ammo );//Esp Ammo 弹药
				ImGui::SameLine( SpaceLineTwo );
				ImGui::Checkbox( "\xE8\xB7\x9D\xE7\xA6\xBB\x0A" , &Settings::Esp::esp_Distance );//Esp Distance 距离
				ImGui::SameLine( SpaceLineThr );
				ImGui::Checkbox( "\xE9\xAA\xA8\xE6\x9E\xB6\x0A" , &Settings::Esp::esp_Skeleton );//Esp Skeleton 骨架

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox( "\xE6\xAD\xA6\xE5\x99\xA8\x0A" , &Settings::Esp::esp_WorldWeapons );//Esp World Weapon 武器
				ImGui::Checkbox( "\xE6\x89\x8B\xE9\x9B\xB7\xE8\xBD\xA8\xE8\xBF\xB9\x0A" , &Settings::Esp::esp_WorldGrenade );//Esp World Grenade 手雷轨迹

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				string visible_1 = "\xE6\x95\x8C\xE4\xBA\xBA\x0A";//不确定!!!!!!!!!!!!!!! Enemy 敌人
				string visible_2 = "\xE9\x98\x9F\xE5\x8F\x8B\x0A";//不确定!!!!!!!!!!!!!!! Team 队友
				string visible_3 = "\xE5\x85\xA8\xE9\x83\xA8\x0A";//不确定!!!!!!!!!!!!!!! All 全部
				string visible_4 = "\xE4\xBB\x85\xE5\x8F\xAF\xE8\xA7\x81\xE6\x97\xB6\x0A";//不确定!!!!!!!!!!!!!!! Only Visible 仅可见时

				const char* items2[] = { visible_1.c_str() , visible_2.c_str() , visible_3.c_str() , visible_4.c_str() };

				ImGui::PushItemWidth( 339.f );
				ImGui::Combo( "\xE5\x8F\xAF\xE8\xA7\x81\xE6\x97\xB6\x0A" , &Settings::Esp::esp_Visible , items2 , IM_ARRAYSIZE( items2 ) );//不确定!!!!!!!!!!!!!!! Esp Visible 透视可见时

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::SliderInt( "\xE9\x80\x8F\xE8\xA7\x86\xE8\x8C\x83\xE5\x9B\xB4\x0A" , &Settings::Esp::esp_Size , 0 , 10 );//不确定!!!!!!!!!!!!!!! Esp Size 透视范围
				ImGui::SliderInt( "\xE8\xAE\xA1\xE6\x97\xB6\xE5\x99\xA8\x0A" , &Settings::Esp::esp_BombTimer , 0 , 65 );//不确定!!!!!!!!!!!!!!! Esp BombTimer 计时器
				ImGui::SliderInt( "\xE7\x9E\x84\xE5\x87\x86\xE7\xBA\xBF\x0A" , &Settings::Esp::esp_BulletTrace , 0 , 3000 );//不确定!!!!!!!!!!!!!!! Esp BulletTrace 瞄准线

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				string hpbar_1 = "\xE6\x97\xA0\x0A";//不确定!!!!!!!!!!!!!!! None 无
				string hpbar_2 = "\xE6\x95\xB0\xE5\xAD\x97\x0A";//不确定!!!!!!!!!!!!!!! Number 数字
				string hpbar_3 = "\xE6\x9D\xA1\xE7\x8A\xB6\x0A";//不确定!!!!!!!!!!!!!!! Bottom 条状
				string hpbar_4 = "\xE5\xB7\xA6\xE4\xBE\xA7\x0A";//不确定!!!!!!!!!!!!!!! Left 左侧

				const char* items3[] = { hpbar_1.c_str() , hpbar_2.c_str() , hpbar_3.c_str() , hpbar_4.c_str() };
				ImGui::Combo( "\xE8\xA1\x80\xE9\x87\x8F\x0A" , &Settings::Esp::esp_Health , items3 , IM_ARRAYSIZE( items3 ) );//不确定!!!!!!!!!!!!!!! Esp Health 血量

				string arbar_1 = "\xE6\x97\xA0\x0A";//不确定!!!!!!!!!!!!!!!None 无
				string arbar_2 = "\xE6\x95\xB0\xE5\xAD\x97\x0A";//不确定!!!!!!!!!!!!!!! Number 数字
				string arbar_3 = "\xE6\x9D\xA1\xE7\x8A\xB6\x0A";//不确定!!!!!!!!!!!!!!! Bottom 条状
				string arbar_4 = "\xE5\x8F\xB3\xE4\xBE\xA7\x0A";//不确定!!!!!!!!!!!!!!! Left 右侧

				const char* items4[] = { arbar_1.c_str() , arbar_2.c_str() , arbar_3.c_str() , arbar_4.c_str() };
				ImGui::Combo( "\xE6\x8A\xA4\xE7\x94\xB2\x0A" , &Settings::Esp::esp_Armor , items4 , IM_ARRAYSIZE( items4 ) );//不确定!!!!!!!!!!!!!!! Esp Armor 透视护甲

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				string chams_1 = "\xE6\x97\xA0\x0A";//不确定!!!!!!!!!!!!!!! None 无
				string chams_2 = "\xE5\xB9\xB3\xE6\xBB\x91 2D \x0A";//不确定!!!!!!!!!!!!!!! Flat 平滑(2D)
				string chams_3 = "\xE5\x8E\x9F\xE7\x89\x88 3D \x0A";//不确定!!!!!!!!!!!!!!! Texture 原版(3D材质)

				const char* items5[] = { chams_1.c_str() , chams_2.c_str() , chams_3.c_str() };
				ImGui::Combo( "\xE5\xBD\xA9\xE4\xBA\xBA\x0A" , &Settings::Esp::esp_Chams , items5 , IM_ARRAYSIZE( items5 ) );//不确定!!!!!!!!!!!!!!!Chams 彩人

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::ColorEdit3( "\xE9\x80\x8F\xE8\xA7\x86 CT \xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Esp::esp_Color_CT );//Esp Color CT 透视CT颜色
				ImGui::ColorEdit3( "\xE9\x80\x8F\xE8\xA7\x86 T \xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Esp::esp_Color_TT );//Esp Color TT 透视T颜色
				ImGui::ColorEdit3( "\xE5\x8F\xAF\xE8\xA7\x81 CT \xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Esp::esp_Color_VCT );//Esp Color Visible CT 可见CT颜色
				ImGui::ColorEdit3( "\xE5\x8F\xAF\xE8\xA7\x81 T \xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Esp::esp_Color_VTT );//Esp Color Visible TT 可见T颜色

				ImGui::ColorEdit3( "\xE5\xBD\xA9\xE4\xBA\xBA CT \xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Esp::chams_Color_CT );//Chams Color CT 彩人CT颜色
				ImGui::ColorEdit3( "\xE5\xBD\xA9\xE4\xBA\xBA T \xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Esp::chams_Color_TT );//Chams Color TT 彩人T颜色
				ImGui::ColorEdit3( "\xE5\xBD\xA9\xE4\xBA\xBA\xE5\x8F\xAF\xE8\xA7\x81 CT \xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Esp::chams_Color_VCT );//Chams Color Visible CT 彩人可见CT颜色
				ImGui::ColorEdit3( "\xE5\xBD\xA9\xE4\xBA\xBA\xE5\x8F\xAF\xE8\xA7\x81 T \xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Esp::chams_Color_VTT );//Chams Color Visible TT 彩人可见T颜色
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
			}
			else if ( tabSelected == 3 ) // Radar
			{
				ImGui::Checkbox( "\xE5\x90\xAF\xE5\x8A\xA8\x0A" , &Settings::Radar::rad_Active );//Active 启动
				ImGui::SameLine();
				ImGui::Checkbox(  "\xE9\x98\x9F\xE5\x8F\x8B\x0A" , &Settings::Radar::rad_Team );//Team 队友
				ImGui::SameLine();
				ImGui::Checkbox( "\xE6\x95\x8C\xE4\xBA\xBA\x0A" , &Settings::Radar::rad_Enemy );//Enemy 敌人
				ImGui::SameLine();
				ImGui::Checkbox( "\xE8\xBD\xA8\xE8\xBF\xB9\x0A" , &Settings::Radar::rad_Sound );//Sound 轨迹
				ImGui::SameLine();
				ImGui::Checkbox( "\xE5\x86\x85\xE7\xBD\xAE\x0A" , &Settings::Radar::rad_InGame );//InGame 内置

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::PushItemWidth( 339.f );
				ImGui::SliderInt( "\xE5\xA4\xA7\xE5\xB0\x8F" , &Settings::Radar::rad_Range , 1 , 5000 );//Range 大小
				ImGui::SliderInt( "\xE9\x80\x8F\xE6\x98\x8E\xE5\xBA\xA6" , &Settings::Radar::rad_Alpha , 1 , 255 );//Alpha 透明度

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::ColorEdit3( "CT\xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Radar::rad_Color_CT );//不确定!!!!!!!!!!!!!!! Color CT
				ImGui::ColorEdit3( "T\xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Radar::rad_Color_TT );//不确定!!!!!!!!!!!!!!! Color TT
				ImGui::ColorEdit3( "CT\xE5\x8F\xAF\xE8\xA7\x81\xE6\x97\xB6\xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Radar::rad_Color_VCT );//不确定!!!!!!!!!!!!!!! Color Visible CT
				ImGui::ColorEdit3( "T\xE5\x8F\xAF\xE8\xA7\x81\xE6\x97\xB6\xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Radar::rad_Color_VTT );//不确定!!!!!!!!!!!!!!! Color Visible TT
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
			}
			else if ( tabSelected == 4 ) // knifebot
			{
				ImGui::Checkbox( "\xE5\x90\xAF\xE5\x8A\xA8\x0A" , &Settings::Knifebot::knf_Active );//Active 启动
				ImGui::Checkbox( "\xE6\x94\xBB\xE5\x87\xBB\xE9\x98\x9F\xE5\x8F\x8B\x0A" , &Settings::Knifebot::knf_Team );//Attack Team 攻击队友

				ImGui::Separator();

				string attack_1 = "\xE8\xBD\xBB\xE5\x87\xBB\x0A";//不确定!!!!!!!!!!!!!!! Attack 1轻击
				string attack_2 = "\xE9\x87\x8D\xE5\x87\xBB\x0A";//不确定!!!!!!!!!!!!!!! Attack 2重击
				string attack_3 = "\xE8\xBD\xBB\xE5\x87\xBB\x2B\xE9\x87\x8D\xE5\x87\xBB\x0A";//不确定!!!!!!!!!!!!!!! Attack 1 + Attack 2 轻击+重击 

				const char* items[] = { attack_1.c_str() , attack_2.c_str() , attack_3.c_str() };
				ImGui::Combo( "\xE5\x88\x80\xE4\xBA\xBA\xE6\xA8\xA1\xE5\xBC\x8F\x0A" , &Settings::Knifebot::knf_Attack , items , IM_ARRAYSIZE( items ) );//不确定!!!!!!!!!!!!!!! Attack type 刀人模式

				ImGui::Separator();

				ImGui::SliderInt( "\xE5\x8C\xBA\xE5\x88\x86\xE8\xBD\xBB\xE5\x87\xBB\x0A" , &Settings::Knifebot::knf_DistAttack , 1 , 100 );//不确定!!!!!!!!!!!!!!!Dist to attack 1 区分轻击
				ImGui::SliderInt( "\xE5\x8C\xBA\xE5\x88\x86\xE9\x87\x8D\xE5\x87\xBB\x0A" , &Settings::Knifebot::knf_DistAttack2 , 1 , 100 );//不确定!!!!!!!!!!!!!!!Dist to attack 2 区分重击
			}
			else if ( tabSelected == 5 ) // Skins
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

				ImGui::Text( "\xE5\xBD\x93\xE5\x89\x8D\xE6\xAD\xA6\xE5\x99\xA8\x0A: %s" , pWeaponData[iWeaponID] );//不确定!!!!!!!!!!!!!!!Current Weapon: %s

				ImGui::PushItemWidth( 362.f );

				ImGui::Separator();

				ImGui::Combo( "CT\xE5\x88\x80\xE6\xA8\xA1\xE5\x9E\x8B\x0A" , &Settings::Skin::knf_ct_model , knife_models_items , IM_ARRAYSIZE( knife_models_items ) );//不确定!!!!!!!!!!!!!!! Knife CT Model CT刀模型
				ImGui::Combo( "T\xE5\x88\x80\xE6\xA8\xA1\xE5\x9E\x8B\x0A" , &Settings::Skin::knf_tt_model , knife_models_items , IM_ARRAYSIZE( knife_models_items ) );//不确定!!!!!!!!!!!!!!! Knife TT Model T刀模型

				ImGui::Separator();

				static int iSelectKnifeCTSkinIndex = -1;
				static int iSelectKnifeTTSkinIndex = -1;

				int iKnifeCTModelIndex = Settings::Skin::knf_ct_model - 1;
				int iKnifeTTModelIndex = Settings::Skin::knf_tt_model - 1;

				static int iOldKnifeCTModelIndex = -1;
				static int iOldKnifeTTModelIndex = -1;

				if ( iOldKnifeCTModelIndex != iKnifeCTModelIndex )
					iSelectKnifeCTSkinIndex = GetKnifeSkinIndexFromPaintKit( Settings::Skin::knf_ct_skin , false );

				if ( iOldKnifeTTModelIndex != iKnifeTTModelIndex )
					iSelectKnifeTTSkinIndex = GetKnifeSkinIndexFromPaintKit( Settings::Skin::knf_ct_skin , true );

				iOldKnifeCTModelIndex = iKnifeCTModelIndex;
				iOldKnifeTTModelIndex = iKnifeTTModelIndex;

				string KnifeCTModel = knife_models_items[Settings::Skin::knf_ct_model];
				string KnifeTTModel = knife_models_items[Settings::Skin::knf_tt_model];

				KnifeCTModel += " Skin##KCT";//不确定!!!!!!!!!!!!!!!
				KnifeTTModel += " Skin##KTT";//不确定!!!!!!!!!!!!!!!

				ImGui::SliderFloat( "Knife CT Wear" , &g_SkinChangerCfg[WEAPON_KNIFE].flFallbackWear , 0.f , 1.f );
				ImGui::Combo( "Knife CT Qality" , &g_SkinChangerCfg[WEAPON_KNIFE].iEntityQuality , quality_items , IM_ARRAYSIZE( quality_items ) );//不确定!!!!!!!!!!!!!!!
				ImGui::ComboBoxArray( KnifeCTModel.c_str() , &iSelectKnifeCTSkinIndex , KnifeSkins[iKnifeCTModelIndex].SkinNames );//不确定!!!!!!!!!!!!!!!

				ImGui::Separator();

				ImGui::SliderFloat( "Knife TT Wear" , &g_SkinChangerCfg[WEAPON_KNIFE_T].flFallbackWear , 0.f , 1.f );
				ImGui::Combo( "Knife TT Qality" , &g_SkinChangerCfg[WEAPON_KNIFE_T].iEntityQuality , quality_items , IM_ARRAYSIZE( quality_items ) );//不确定!!!!!!!!!!!!!!!
				ImGui::ComboBoxArray( KnifeTTModel.c_str() , &iSelectKnifeTTSkinIndex , KnifeSkins[iKnifeTTModelIndex].SkinNames );//不确定!!!!!!!!!!!!!!!

				ImGui::Separator();

				static int iOldWeaponID = -1;

				ImGui::Combo( "Weapon##WeaponSelect" , &iWeaponID , pWeaponData , IM_ARRAYSIZE( pWeaponData ) );//不确定!!!!!!!!!!!!!!!

				iWeaponSelectIndex = pWeaponItemIndexData[iWeaponID];

				if ( iOldWeaponID != iWeaponID )
					iWeaponSelectSkinIndex = GetWeaponSkinIndexFromPaintKit( g_SkinChangerCfg[iWeaponSelectIndex].nFallbackPaintKit );

				iOldWeaponID = iWeaponID;

				string WeaponSkin = pWeaponData[iWeaponID];
				WeaponSkin += " Skin";//不确定!!!!!!!!!!!!!!!

				ImGui::ComboBoxArray( WeaponSkin.c_str() , &iWeaponSelectSkinIndex , WeaponSkins[iWeaponID].SkinNames );

				ImGui::Combo("\xE5\x93\x81\xE8\xB4\xA8\x0A", &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].iEntityQuality, quality_items, IM_ARRAYSIZE(quality_items));//不确定!!!!!!!!!!!!!!!Weapon Qality 武器品质
				ImGui::SliderFloat( "\xE7\xA3\xA8\xE6\x8D\x9F\x0A" , &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].flFallbackWear , 0.f , 1.f );//不确定!!!!!!!!!!!!!!!Weapon Wear 武器磨损
				ImGui::InputInt( "\xE6\x9A\x97\xE9\x87\x91\x0A" , &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].nFallbackStatTrak , 1 , 100 , ImGuiInputTextFlags_CharsDecimal );//不确定!!!!!!!!!!!!!!! Weapon StatTrak 暗金武器

				ImGui::Separator();

				ImGui::Combo( "\xE6\x89\x8B\xE5\xA5\x97\x0A" , &Settings::Skin::gloves_skin , gloves_listbox_items ,//不确定!!!!!!!!!!!!!!!Gloves Skin 手套皮肤
							  IM_ARRAYSIZE( gloves_listbox_items ) );

				ImGui::Separator();

				ImGui::PopItemWidth();

				if ( ImGui::Button( "\xE5\xBA\x94\xE7\x94\xA8##\xE7\x9A\xAE\xE8\x82\xA4" ) )//不确定!!!!!!!!!!!!!!!Apply##Skin 接受皮肤
				{
					g_SkinChangerCfg[iWeaponSelectIndex].nFallbackPaintKit = WeaponSkins[iWeaponID].SkinPaintKit[iWeaponSelectSkinIndex];

					Settings::Skin::knf_ct_skin = KnifeSkins[iKnifeCTModelIndex].SkinPaintKit[iSelectKnifeCTSkinIndex];
					Settings::Skin::knf_tt_skin = KnifeSkins[iKnifeTTModelIndex].SkinPaintKit[iSelectKnifeTTSkinIndex];

					ForceFullUpdate();
				}
			}
			else if ( tabSelected == 6 ) // Misc
			{
				ImGui::Checkbox( "\xE8\xBF\x9E\xE8\xB7\xB3\x0A" , &Settings::Misc::misc_Bhop );//Bhop 连跳
				ImGui::Checkbox( "punch" , &Settings::Misc::misc_Punch );//Punch ?
				ImGui::Checkbox( "\xE7\x8B\x99\xE5\x87\xBB\xE6\x9E\xAA\xE5\x87\x86\xE5\xBF\x83" , &Settings::Misc::misc_SniperAim );//SniperAim 狙击准星
				ImGui::Checkbox( "\xE6\x97\xA0\xE9\x97\xAA\xE5\x85\x89\x0A" , &Settings::Misc::misc_NoFlash );//NoFlash 无闪光
				ImGui::Checkbox( "\xE8\x87\xAA\xE5\x8A\xA8\xE6\x89\xAB\xE5\xB0\x84\x0A" , &Settings::Misc::misc_AutoStrafe );//AutoStrafe 自动扫射
				ImGui::Checkbox( "\xE8\x87\xAA\xE5\x8A\xA8\xE7\xA1\xAE\xE8\xAE\xA4\x0A" , &Settings::Misc::misc_AutoAccept );//AutoAccept 自动确认
				ImGui::Checkbox( "\xE8\xA7\x82\xE5\xAF\x9F\xE8\x80\x85\xE5\x88\x97\xE8\xA1\xA8\x0A" , &Settings::Misc::misc_Spectators );//Spectators 观察者列表
				ImGui::Checkbox( "\xE8\xB7\x9D\xE7\xA6\xBB\x0A" , &Settings::Misc::misc_FovChanger );//Fov Changer 距离开关
				ImGui::PushItemWidth( 362.f );
				ImGui::SliderInt( "\xE8\xA7\x86\xE9\x87\x8E\xE8\x8C\x83\xE5\x9B\xB4\x0A" , &Settings::Misc::misc_FovView , 1 , 190 );//Fov View 视野范围
				ImGui::SliderInt( "\xE6\xA8\xA1\xE5\x9E\x8B\xE8\xB7\x9D\xE7\xA6\xBB\x0A" , &Settings::Misc::misc_FovModelView , 1 , 190 );//Fov Model View 模型距离
				ImGui::Separator();
				ImGui::ColorEdit3( "\xE5\x87\x86\xE5\xBF\x83\xE9\xA2\x9C\xE8\x89\xB2" , Settings::Misc::misc_SniperAimColor );//Awp Aim Color AWP自瞄颜色
				ImGui::PopItemWidth();
			}
			else if ( tabSelected == 7 ) // Config
			{
				static int iConfigSelect = 0;
				static int iMenuSheme = 1;
				static char ConfigName[64] = { 0 };

				ImGui::ComboBoxArray( "\xE9\x80\x89\xE6\x8B\xA9\xE9\x85\x8D\xE7\xBD\xAE\x0A" , &iConfigSelect , ConfigList );//Select Config 选择配置

				ImGui::Separator();

				if ( ImGui::Button( "\xE8\xBD\xBD\xE5\x85\xA5\xE9\x85\x8D\xE7\xBD\xAE\x0A" ) & ConfigName != NULL )//Load Config 载入配置
				{
					Settings::LoadSettings( BaseDir + "\\" + ConfigList[iConfigSelect] );
				}
				ImGui::SameLine();
				if ( ImGui::Button( "\xE4\xBF\x9D\xE5\xAD\x98\xE9\x85\x8D\xE7\xBD\xAE\x0A") & ConfigName != NULL )//Save Config 保存配置
				{
					Settings::SaveSettings( BaseDir + "\\" + ConfigList[iConfigSelect] );
				}
				ImGui::SameLine();
				if ( ImGui::Button( "\xE5\x88\xB7\xE6\x96\xB0\xE9\x85\x8D\xE7\xBD\xAE\xE5\x88\x97\xE8\xA1\xA8\x0A" ) )//Refresh Config List 刷新配置列表
				{
					RefreshConfigs();
				}

				ImGui::Separator();

				ImGui::InputText( "\xE9\x85\x8D\xE7\xBD\xAE\xE5\x90\x8D\xE5\xAD\x97\x0A" , ConfigName , 64 );//Config Name 配置名字

				if ( ImGui::Button( "\xE5\x88\x9B\xE5\xBB\xBA\x26\xE4\xBF\x9D\xE5\xAD\x98\xE6\x96\xB0\xE9\x85\x8D\xE7\xBD\xAE\x0A" ) )//Create & Save new Config 创建&保存新配置
				{
					string ConfigFileName = ConfigName;

					if ( ConfigFileName.size() < 1 )
					{
						ConfigFileName = "\xE8\xAE\xBE\xE7\xBD\xAE\x0A";//Settings 设置
					}

					Settings::SaveSettings( BaseDir + "\\" + ConfigFileName + ".ini" );
					RefreshConfigs();
				}

				ImGui::Separator();

				const char* ThemesList[] = { "Default" , "Red" };

				ImGui::Combo( "\xE7\x95\x8C\xE9\x9D\xA2\xE9\xA2\x9C\xE8\x89\xB2\xE6\x96\xB9\xE6\xA1\x88\x0A" , &iMenuSheme , ThemesList ,  IM_ARRAYSIZE( ThemesList ) );//不确定!!!!!!!!!!!!!!! Menu Color Sheme 界面颜色方案

				ImGui::Separator();

				if ( ImGui::Button( "\xE6\x8E\xA5\xE5\x8F\x97\xE6\x96\xB9\xE6\xA1\x88\x0A" ) )//不确定!!!!!!!!!!!!!!! Apply Sheme 接受方案
				{
					if ( iMenuSheme == 0 )
					{
						g_pGui->DefaultSheme();
					}
					else if ( iMenuSheme == 1 )
					{
						g_pGui->RedSheme();
					}
				}
			}

			ImGui::End();
		}
	}
}