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
				ImGui::Text( "\xe5\xbd\x93\xe5\x89\x8d\xe6\xad\xa6\xe5\x99\xa8: " );//Current Weapon ��ǰ����
				ImGui::SameLine();
				ImGui::Combo( "##\xE6\xAD\xA6\xE5\x99\xA8\xE8\x87\xAA\xE7\x9E\x84\x0A" , &iWeaponID , pWeaponData , IM_ARRAYSIZE( pWeaponData ) );//��ȷ��!!!!!!!!!!!!!!!AimWeapon ��������
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox( "\xe6\xad\xbb\xe4\xba\xa1\xe7\xab\x9e\xe8\xb5\x9b\xe6\xa8\xa1\xe5\xbc\x8f" , &Settings::Aimbot::aim_Deathmatch ); //Deathmatch ��������
				ImGui::SameLine( SpaceLineOne );
				ImGui::Checkbox( "\xe7\xa9\xbf\xe5\xa2\x99" , &Settings::Aimbot::aim_WallAttack );//WallAttack ��ǽ
				ImGui::SameLine( SpaceLineTwo );
				ImGui::Checkbox( "\xe6\xa3\x80\xe6\xb5\x8b\xe7\x83\x9f\xe9\x9b\xbe" , &Settings::Aimbot::aim_CheckSmoke );//CheckSmoke �������

				ImGui::Checkbox( "\xe5\x8f\x8d\xe8\xb7\xb3\xe8\xb7\x83" , &Settings::Aimbot::aim_AntiJump );//AntiJump ����Ծ
				ImGui::SameLine( SpaceLineOne );
				ImGui::Checkbox( "\xE7\xBB\x98\xE5\x88\xB6\xE7\x9E\x84\xE5\x87\x86\xE8\x8C\x83\xE5\x9B\xB4" , &Settings::Aimbot::aim_DrawFov );//Draw Fov ������׼��Χ
				ImGui::SameLine( SpaceLineTwo );
				ImGui::Checkbox( "\xE7\xBB\x98\xE5\x88\xB6\xE7\x9E\x84\xE5\x87\x86\xE7\x82\xB9" , &Settings::Aimbot::aim_DrawSpot );//DrawSpot ������׼��

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox( "\xe5\x90\xaf\xe7\x94\xa8" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Active );//aim Active ����
								
				if ( iWeaponID <= 9 )
				{
					ImGui::SameLine();
					ImGui::Checkbox( "\xE8\x87\xAA\xE5\x8A\xA8\xE6\x89\x8B\xE6\x9E\xAA\x0A" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_AutoPistol );//Autopistol �Զ���ǹ
				}

				ImGui::PushItemWidth( 362.f );
				ImGui::SliderInt( "\xE5\xB9\xB3\xE6\xBB\x91\x0A" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Smooth , 1 , 300 );//Smooth ƽ��
				ImGui::SliderInt( "Fov" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Fov , 1 , 300 );
				ImGui::PopItemWidth();

				const char* AimFovType[] = { "\xE5\x8A\xA8\xE6\x80\x81\x0A" , "\xE9\x9D\x99\xE6\x80\x81\x0A" };//Dynamic Static ��̬ ��̬
				ImGui::PushItemWidth( 362.f );
				ImGui::Combo( "\xE8\x8C\x83\xE5\x9B\xB4\xE7\xB1\xBB\xE5\x9E\x8B\x0A" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_FovType , AimFovType , IM_ARRAYSIZE( AimFovType ) ); //��ȷ��!!!!!!!!!!!!!!!Fov Type ��Χ����
				ImGui::PopItemWidth();

				const char* BestHit[] = { "\xE5\x85\xB3\xE9\x97\xAD\x0A" , "\xE5\xBC\x80\xE5\x90\xAF\x0A" };//�ر� ����
				ImGui::PushItemWidth( 362.f );
				ImGui::Combo( "\xE7\xB2\xBE\xE5\x87\x86\xE5\xB0\x84\xE5\x87\xBB\x0A" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_BestHit , BestHit , IM_ARRAYSIZE( BestHit ) ); //BestHit ��׼���
				
				if ( ImGui::IsItemHovered() )
					ImGui::SetTooltip( "\xE5\x85\xB3\xE9\x97\xAD\xE4\xBB\xA5\xE4\xBD\xBF\xE7\x94\xA8\xE8\x87\xAA\xE7\x9E\x84\xE7\x82\xB9\x0A" );//if disabled then used Aimspot �ر���ʹ�������

				ImGui::PopItemWidth();

				const char* Aimspot[] = { "\xE5\xA4\xB4\x0A" , "\xE8\x84\x96\xE5\xAD\x90\x0A" , "\xE9\x94\x81\xE9\xAA\xA8\x0A" , "\xE8\xBA\xAB\xE4\xBD\x93\x0A" , "\xE8\x83\xB8\xE9\x83\xA8\x0A" , "\xE8\x85\xB9\xE9\x83\xA8\x0A" };//"Head" , "Neck" , "Low Neck" , "Body" , "Thorax" , "Chest" ͷ ���� ���� ���� �ز� ����
				ImGui::PushItemWidth( 362.f );
				ImGui::Combo( "\xE8\x87\xAA\xE7\x9E\x84\xE7\x82\xB9\x0A" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Spot , Aimspot , IM_ARRAYSIZE( Aimspot ) );//Aimspot �����
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::PushItemWidth( 362.f );
				ImGui::SliderInt( "\xE5\xB0\x84\xE5\x87\xBB\xE5\xBB\xB6\xE6\x97\xB6\x0A" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Delay , 0 , 200 );//ShotDelay �����ʱ
				ImGui::SliderInt( "\xE5\x90\x8E\xE5\x9D\x90\xE5\x8A\x9B\xE6\x8E\xA7\xE5\x88\xB6\x0A" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Rcs , 0 , 100 );//Rcs ����������
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				if ( iWeaponID >= 10 && iWeaponID <= 30 )
				{
					ImGui::PushItemWidth( 362.f );
					ImGui::SliderInt( "\xE5\x90\x8E\xE5\x9D\x90\xE5\x8A\x9B\xE6\x8E\xA7\xE5\x88\xB6\xE8\x8C\x83\xE5\x9B\xB4" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_RcsFov , 1 , 300 );//Rcs Fov ���������Ʒ�Χ
					ImGui::SliderInt( "\xE5\x90\x8E\xE5\x9D\x90\xE5\x8A\x9B\xE6\x8E\xA7\xE5\x88\xB6\xE5\xB9\xB3\xE6\xBB\x91" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_RcsSmooth , 1 , 300 );//Rcs Smooth ����������ƽ��
					ImGui::PopItemWidth();

					const char* ClampType[] = { "\xE5\x85\xA8\xE9\x83\xA8\xE6\x95\x8C\xE4\xBA\xBA\x0A" , "Shot" , "Shot + Target" };//"All Target" , "Shot" , "Shot + Target"ȫ������ ��� ���+���
					ImGui::PushItemWidth( 362.f );
					ImGui::Combo( "Rcs Clamp" , &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_RcsClampType , ClampType , IM_ARRAYSIZE( ClampType ) );//��ȷ��!!!!!!!!!!!!!!!���������� �̶�����
					ImGui::PopItemWidth();

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();
				}
			}
			else if ( tabSelected == 1 ) // Trigger
			{
				const char* TriggerEnable[] = { "\xE5\x85\xB3\xE9\x97\xAD\x0A" , "\xE8\x8C\x83\xE5\x9B\xB4\x0A" , "\xE8\xBF\xBD\xE8\xB8\xAA\x0A" };//��ȷ��!!!!!!!!!!!!!!!�ر� ��Χ ����
				ImGui::PushItemWidth( 80.f );
				ImGui::Combo( "\xE5\xBC\x80\xE5\x90\xAF\x0A" , &Settings::Triggerbot::trigger_Enable , TriggerEnable , IM_ARRAYSIZE( TriggerEnable ) );//��ȷ��!!!!!!!!!!!!!!!����
				ImGui::PopItemWidth();
				ImGui::SameLine();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox( "\xe6\xad\xbb\xe4\xba\xa1\xe7\xab\x9e\xe8\xb5\x9b\xe6\xa8\xa1\xe5\xbc\x8f" , &Settings::Triggerbot::trigger_Deathmatch );//Deathmatch ��������
				ImGui::SameLine( SpaceLineOne );
				ImGui::Checkbox( "\xe7\xa9\xbf\xe5\xa2\x99" , &Settings::Triggerbot::trigger_WallAttack );//WallAttack ��ǽ
				ImGui::SameLine( SpaceLineTwo );
				ImGui::Checkbox( "\xE5\xBF\xAB\xE9\x80\x9F\xE7\xBC\xA9\xE6\x94\xBE\x0A" , &Settings::Triggerbot::trigger_FastZoom );//FastZoom ��������
				
				ImGui::Checkbox( "\xE7\x83\x9F\xE9\x9B\xBE\xE6\xA3\x80\xE6\xB5\x8B\x0A" , &Settings::Triggerbot::trigger_SmokCheck );//SmokeCheck ������
				ImGui::SameLine( SpaceLineOne );
				ImGui::Checkbox( "\xE7\x9E\x84\xE5\x87\x86\xE7\x82\xB9\xE8\x8C\x83\xE5\x9B\xB4\x0A" , &Settings::Triggerbot::trigger_DrawFov );//DrawFov ��׼�㷶Χ
				ImGui::SameLine( SpaceLineTwo );
				ImGui::Checkbox( "\xE7\x9E\x84\xE5\x87\x86\xE7\x82\xB9\x0A" , &Settings::Triggerbot::trigger_DrawSpot );//DrawSpot ��׼��
				ImGui::SameLine( SpaceLineThr );
				ImGui::Checkbox( "\xE7\x9E\x84\xE5\x87\x86\xE7\x82\xB9\xE5\x8D\x8F\xE5\x8A\xA9\x0A" , &Settings::Triggerbot::trigger_DrawFovAssist );//DrawFovAssist ��׼��Э��

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				
				const char* items1[] = { CVAR_KEY_MOUSE3 , CVAR_KEY_MOUSE4 , CVAR_KEY_MOUSE5 };
				ImGui::PushItemWidth( 80.f );
				ImGui::Combo( "Key" , &Settings::Triggerbot::trigger_Key , items1 , IM_ARRAYSIZE( items1 ) );//��ȷ��!!!!!!!!!!!!!!!key
				ImGui::PopItemWidth();
				ImGui::SameLine();

				const char* items2[] = { "Hold" , "Press" };//��ȷ��!!!!!!!!!!!!!!!
				ImGui::PushItemWidth( 80.f );
				ImGui::Combo( "Key Mode" , &Settings::Triggerbot::trigger_KeyMode , items2 , IM_ARRAYSIZE( items2 ) );//��ȷ��!!!!!!!!!!!!!!!key mode
				ImGui::PopItemWidth();
				ImGui::SameLine();

				ImGui::PushItemWidth( 110.f );
				ImGui::Combo( "\xE6\xAD\xA6\xE5\x99\xA8\x0A" , &iWeaponID , pWeaponData , IM_ARRAYSIZE( pWeaponData ) );//��ȷ��!!!!!!!!!!!!!!!weapon����
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				
				ImGui::PushItemWidth( 362.f );
				ImGui::SliderInt( "\xE6\x9C\x80\xE5\xB0\x8F\xE8\xB7\x9D\xE7\xA6\xBB\x0A" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_DistanceMin , 0 , 5000 );//��ȷ��!!!!!!!!!!!!!!!Min Disstance��С����
				ImGui::SliderInt( "\xE6\x9C\x80\xE5\xA4\xA7\xE8\xB7\x9D\xE7\xA6\xBB\x0A" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_DistanceMax , 0 , 5000 );//��ȷ��!!!!!!!!!!!!!!!Max Disstance������
				ImGui::SliderInt( "\xE8\x8C\x83\xE5\x9B\xB4\x0A" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_Fov , 1 , 100 );//��ȷ��!!!!!!!!!!!!!!!Fov��Χ
				ImGui::SliderInt( "\xE5\xBB\xB6\xE6\x97\xB6\xE5\x89\x8D\x0A" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_DelayBefore , 0 , 200 );//��ȷ��!!!!!!!!!!!!!!!Delay Before��ʱ֮ǰ
				ImGui::SliderInt( "\xE5\xBB\xB6\xE6\x97\xB6\xE5\x90\x8E\x0A" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_DelayAfter , 0 , 1000 );//��ȷ��!!!!!!!!!!!!!!!Delay After��ʱ֮��
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox( "\xE5\x8F\xAA\xE7\x9E\x84\xE5\xA4\xB4\x0A" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_HeadOnly );//HeadOnly ֻ��ͷ
				ImGui::SameLine();

				const char* AssistMode[] = { "Disable" , "One Shot" , "Auto" };//��ȷ��!!!!!!!!!!!!!!!
				ImGui::PushItemWidth( 80.f );
				ImGui::Combo( "\xE5\x90\xB8\xE9\x99\x84\x0A" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_Assist , AssistMode , IM_ARRAYSIZE( AssistMode ) );//��ȷ��!!!!!!!!!!!!!!!Assist ����
				ImGui::PopItemWidth();
				ImGui::SameLine();

				const char* AssistFovType[] = { "Dynamic" , "Static" };//��ȷ��!!!!!!!!!!!!!!!
				ImGui::PushItemWidth( 80.f );
				ImGui::Combo( "\xE5\x90\xB8\xE9\x99\x84\xE7\xB1\xBB\xE5\x9E\x8B\x0A" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_AssistFovType , AssistFovType , IM_ARRAYSIZE( AssistFovType ) );//��ȷ��!!!!!!!!!!!!!!!Assist Fov Type ��������
				ImGui::PopItemWidth();

				const char* HitGroup[] = { "All" , "Head + Body" , "Head" };
				ImGui::Combo( "HitGroup" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_HitGroup , HitGroup , IM_ARRAYSIZE( HitGroup ) );
				//ImGui::PopItemWidth();
				
				ImGui::PushItemWidth( 362.f );
				ImGui::SliderInt( "\xE5\x90\xB8\xE9\x99\x84\xE5\x90\x8E\xE5\x9D\x90\xE5\x8A\x9B\x0A" , &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_AssistRcs , 0 , 100 );//��ȷ��!!!!!!!!!!!!!!! Assist Rcs ����������
				ImGui::SliderInt("\xE5\x90\xB8\xE9\x99\x84\xE8\x8C\x83\xE5\x9B\xB4\x0A", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_AssistFov, 1, 300);//��ȷ��!!!!!!!!!!!!!!! Assist Fov ������Χ
				ImGui::SliderInt("\xE5\x90\xB8\xE9\x99\x84\xE5\xB9\xB3\xE6\xBB\x91\x0A", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_AssistSmooth, 1, 300);//��ȷ��!!!!!!!!!!!!!!! Assist Smooth ����ƽ��
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
			}
			else if ( tabSelected == 2 ) // Visuals
			{
				string style_1 = "\xE6\x96\xB9\xE6\xA1\x86\x0A";//��ȷ��!!!!!!!!!!!!!!!Box ����
				string style_2 = "\xE5\xA1\xAB\xE5\x85\x85\xE6\xA1\x86\x0A";//��ȷ��!!!!!!!!!!!!!!!CoalBox ����

				const char* items1[] = { style_1.c_str() , style_2.c_str() };

				ImGui::PushItemWidth( 339.f );
				ImGui::Combo( "\xE9\x80\x8F\xE8\xA7\x86\xE7\xB1\xBB\xE5\x9E\x8B\x0A" , &Settings::Esp::esp_Style , items1 , IM_ARRAYSIZE( items1 ) );//��ȷ��!!!!!!!!!!!!!!! Esp Type ͸������
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox( "\xE9\x98\x9F\xE5\x8F\x8B\x0A" , &Settings::Esp::esp_Team );//Esp Team ����
				ImGui::SameLine( SpaceLineOne );
				ImGui::Checkbox( "\xE6\x95\x8C\xE4\xBA\xBA\x0A" , &Settings::Esp::esp_Enemy );//Esp Enemy ����
				ImGui::SameLine( SpaceLineTwo );
				ImGui::Checkbox( "\xE7\x82\xB8\xE5\xBC\xB9\x0A" , &Settings::Esp::esp_Bomb );//Esp Bomp ը��
				ImGui::SameLine( SpaceLineThr );
				ImGui::Checkbox( "\xE8\x84\x9A\xE6\xAD\xA5\xE8\xBD\xA8\xE8\xBF\xB9\x0A" , &Settings::Esp::esp_Sound );//Esp Sound �Ų��켣

				ImGui::Checkbox( "\xE7\x9B\xB4\xE7\xBA\xBF\x0A" , &Settings::Esp::esp_Line );//Esp Line ֱ��
				ImGui::SameLine( SpaceLineOne );
				ImGui::Checkbox( "\xE8\xBD\xAE\xE5\xBB\x93\x0A" , &Settings::Esp::esp_Outline );//Esp OutLine ����
				ImGui::SameLine( SpaceLineTwo );
				ImGui::Checkbox( "\xE5\x90\x8D\xE5\xAD\x97\x0A" , &Settings::Esp::esp_Name );//Esp Name ����
				ImGui::SameLine( SpaceLineThr );
				ImGui::Checkbox( "\xE6\xAE\xB5\xE4\xBD\x8D\x0A" , &Settings::Esp::esp_Rank );//Esp Rank ��λ

				ImGui::Checkbox( "\xE6\xAD\xA6\xE5\x99\xA8\x0A" , &Settings::Esp::esp_Weapon );//Esp Weapon ����
				ImGui::SameLine( SpaceLineOne );
				ImGui::Checkbox( "\xE5\xBC\xB9\xE8\x8D\xAF\x0A" , &Settings::Esp::esp_Ammo );//Esp Ammo ��ҩ
				ImGui::SameLine( SpaceLineTwo );
				ImGui::Checkbox( "\xE8\xB7\x9D\xE7\xA6\xBB\x0A" , &Settings::Esp::esp_Distance );//Esp Distance ����
				ImGui::SameLine( SpaceLineThr );
				ImGui::Checkbox( "\xE9\xAA\xA8\xE6\x9E\xB6\x0A" , &Settings::Esp::esp_Skeleton );//Esp Skeleton �Ǽ�

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox( "\xE6\xAD\xA6\xE5\x99\xA8\x0A" , &Settings::Esp::esp_WorldWeapons );//Esp World Weapon ����
				ImGui::Checkbox( "\xE6\x89\x8B\xE9\x9B\xB7\xE8\xBD\xA8\xE8\xBF\xB9\x0A" , &Settings::Esp::esp_WorldGrenade );//Esp World Grenade ���׹켣

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				string visible_1 = "\xE6\x95\x8C\xE4\xBA\xBA\x0A";//��ȷ��!!!!!!!!!!!!!!! Enemy ����
				string visible_2 = "\xE9\x98\x9F\xE5\x8F\x8B\x0A";//��ȷ��!!!!!!!!!!!!!!! Team ����
				string visible_3 = "\xE5\x85\xA8\xE9\x83\xA8\x0A";//��ȷ��!!!!!!!!!!!!!!! All ȫ��
				string visible_4 = "\xE4\xBB\x85\xE5\x8F\xAF\xE8\xA7\x81\xE6\x97\xB6\x0A";//��ȷ��!!!!!!!!!!!!!!! Only Visible ���ɼ�ʱ

				const char* items2[] = { visible_1.c_str() , visible_2.c_str() , visible_3.c_str() , visible_4.c_str() };

				ImGui::PushItemWidth( 339.f );
				ImGui::Combo( "\xE5\x8F\xAF\xE8\xA7\x81\xE6\x97\xB6\x0A" , &Settings::Esp::esp_Visible , items2 , IM_ARRAYSIZE( items2 ) );//��ȷ��!!!!!!!!!!!!!!! Esp Visible ͸�ӿɼ�ʱ

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::SliderInt( "\xE9\x80\x8F\xE8\xA7\x86\xE8\x8C\x83\xE5\x9B\xB4\x0A" , &Settings::Esp::esp_Size , 0 , 10 );//��ȷ��!!!!!!!!!!!!!!! Esp Size ͸�ӷ�Χ
				ImGui::SliderInt( "\xE8\xAE\xA1\xE6\x97\xB6\xE5\x99\xA8\x0A" , &Settings::Esp::esp_BombTimer , 0 , 65 );//��ȷ��!!!!!!!!!!!!!!! Esp BombTimer ��ʱ��
				ImGui::SliderInt( "\xE7\x9E\x84\xE5\x87\x86\xE7\xBA\xBF\x0A" , &Settings::Esp::esp_BulletTrace , 0 , 3000 );//��ȷ��!!!!!!!!!!!!!!! Esp BulletTrace ��׼��

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				string hpbar_1 = "\xE6\x97\xA0\x0A";//��ȷ��!!!!!!!!!!!!!!! None ��
				string hpbar_2 = "\xE6\x95\xB0\xE5\xAD\x97\x0A";//��ȷ��!!!!!!!!!!!!!!! Number ����
				string hpbar_3 = "\xE6\x9D\xA1\xE7\x8A\xB6\x0A";//��ȷ��!!!!!!!!!!!!!!! Bottom ��״
				string hpbar_4 = "\xE5\xB7\xA6\xE4\xBE\xA7\x0A";//��ȷ��!!!!!!!!!!!!!!! Left ���

				const char* items3[] = { hpbar_1.c_str() , hpbar_2.c_str() , hpbar_3.c_str() , hpbar_4.c_str() };
				ImGui::Combo( "\xE8\xA1\x80\xE9\x87\x8F\x0A" , &Settings::Esp::esp_Health , items3 , IM_ARRAYSIZE( items3 ) );//��ȷ��!!!!!!!!!!!!!!! Esp Health Ѫ��

				string arbar_1 = "\xE6\x97\xA0\x0A";//��ȷ��!!!!!!!!!!!!!!!None ��
				string arbar_2 = "\xE6\x95\xB0\xE5\xAD\x97\x0A";//��ȷ��!!!!!!!!!!!!!!! Number ����
				string arbar_3 = "\xE6\x9D\xA1\xE7\x8A\xB6\x0A";//��ȷ��!!!!!!!!!!!!!!! Bottom ��״
				string arbar_4 = "\xE5\x8F\xB3\xE4\xBE\xA7\x0A";//��ȷ��!!!!!!!!!!!!!!! Left �Ҳ�

				const char* items4[] = { arbar_1.c_str() , arbar_2.c_str() , arbar_3.c_str() , arbar_4.c_str() };
				ImGui::Combo( "\xE6\x8A\xA4\xE7\x94\xB2\x0A" , &Settings::Esp::esp_Armor , items4 , IM_ARRAYSIZE( items4 ) );//��ȷ��!!!!!!!!!!!!!!! Esp Armor ͸�ӻ���

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				string chams_1 = "\xE6\x97\xA0\x0A";//��ȷ��!!!!!!!!!!!!!!! None ��
				string chams_2 = "\xE5\xB9\xB3\xE6\xBB\x91 2D \x0A";//��ȷ��!!!!!!!!!!!!!!! Flat ƽ��(2D)
				string chams_3 = "\xE5\x8E\x9F\xE7\x89\x88 3D \x0A";//��ȷ��!!!!!!!!!!!!!!! Texture ԭ��(3D����)

				const char* items5[] = { chams_1.c_str() , chams_2.c_str() , chams_3.c_str() };
				ImGui::Combo( "\xE5\xBD\xA9\xE4\xBA\xBA\x0A" , &Settings::Esp::esp_Chams , items5 , IM_ARRAYSIZE( items5 ) );//��ȷ��!!!!!!!!!!!!!!!Chams ����

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::ColorEdit3( "\xE9\x80\x8F\xE8\xA7\x86 CT \xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Esp::esp_Color_CT );//Esp Color CT ͸��CT��ɫ
				ImGui::ColorEdit3( "\xE9\x80\x8F\xE8\xA7\x86 T \xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Esp::esp_Color_TT );//Esp Color TT ͸��T��ɫ
				ImGui::ColorEdit3( "\xE5\x8F\xAF\xE8\xA7\x81 CT \xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Esp::esp_Color_VCT );//Esp Color Visible CT �ɼ�CT��ɫ
				ImGui::ColorEdit3( "\xE5\x8F\xAF\xE8\xA7\x81 T \xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Esp::esp_Color_VTT );//Esp Color Visible TT �ɼ�T��ɫ

				ImGui::ColorEdit3( "\xE5\xBD\xA9\xE4\xBA\xBA CT \xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Esp::chams_Color_CT );//Chams Color CT ����CT��ɫ
				ImGui::ColorEdit3( "\xE5\xBD\xA9\xE4\xBA\xBA T \xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Esp::chams_Color_TT );//Chams Color TT ����T��ɫ
				ImGui::ColorEdit3( "\xE5\xBD\xA9\xE4\xBA\xBA\xE5\x8F\xAF\xE8\xA7\x81 CT \xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Esp::chams_Color_VCT );//Chams Color Visible CT ���˿ɼ�CT��ɫ
				ImGui::ColorEdit3( "\xE5\xBD\xA9\xE4\xBA\xBA\xE5\x8F\xAF\xE8\xA7\x81 T \xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Esp::chams_Color_VTT );//Chams Color Visible TT ���˿ɼ�T��ɫ
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
			}
			else if ( tabSelected == 3 ) // Radar
			{
				ImGui::Checkbox( "\xE5\x90\xAF\xE5\x8A\xA8\x0A" , &Settings::Radar::rad_Active );//Active ����
				ImGui::SameLine();
				ImGui::Checkbox(  "\xE9\x98\x9F\xE5\x8F\x8B\x0A" , &Settings::Radar::rad_Team );//Team ����
				ImGui::SameLine();
				ImGui::Checkbox( "\xE6\x95\x8C\xE4\xBA\xBA\x0A" , &Settings::Radar::rad_Enemy );//Enemy ����
				ImGui::SameLine();
				ImGui::Checkbox( "\xE8\xBD\xA8\xE8\xBF\xB9\x0A" , &Settings::Radar::rad_Sound );//Sound �켣
				ImGui::SameLine();
				ImGui::Checkbox( "\xE5\x86\x85\xE7\xBD\xAE\x0A" , &Settings::Radar::rad_InGame );//InGame ����

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::PushItemWidth( 339.f );
				ImGui::SliderInt( "\xE5\xA4\xA7\xE5\xB0\x8F" , &Settings::Radar::rad_Range , 1 , 5000 );//Range ��С
				ImGui::SliderInt( "\xE9\x80\x8F\xE6\x98\x8E\xE5\xBA\xA6" , &Settings::Radar::rad_Alpha , 1 , 255 );//Alpha ͸����

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::ColorEdit3( "CT\xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Radar::rad_Color_CT );//��ȷ��!!!!!!!!!!!!!!! Color CT
				ImGui::ColorEdit3( "T\xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Radar::rad_Color_TT );//��ȷ��!!!!!!!!!!!!!!! Color TT
				ImGui::ColorEdit3( "CT\xE5\x8F\xAF\xE8\xA7\x81\xE6\x97\xB6\xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Radar::rad_Color_VCT );//��ȷ��!!!!!!!!!!!!!!! Color Visible CT
				ImGui::ColorEdit3( "T\xE5\x8F\xAF\xE8\xA7\x81\xE6\x97\xB6\xE9\xA2\x9C\xE8\x89\xB2\x0A" , Settings::Radar::rad_Color_VTT );//��ȷ��!!!!!!!!!!!!!!! Color Visible TT
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
			}
			else if ( tabSelected == 4 ) // knifebot
			{
				ImGui::Checkbox( "\xE5\x90\xAF\xE5\x8A\xA8\x0A" , &Settings::Knifebot::knf_Active );//Active ����
				ImGui::Checkbox( "\xE6\x94\xBB\xE5\x87\xBB\xE9\x98\x9F\xE5\x8F\x8B\x0A" , &Settings::Knifebot::knf_Team );//Attack Team ��������

				ImGui::Separator();

				string attack_1 = "\xE8\xBD\xBB\xE5\x87\xBB\x0A";//��ȷ��!!!!!!!!!!!!!!! Attack 1���
				string attack_2 = "\xE9\x87\x8D\xE5\x87\xBB\x0A";//��ȷ��!!!!!!!!!!!!!!! Attack 2�ػ�
				string attack_3 = "\xE8\xBD\xBB\xE5\x87\xBB\x2B\xE9\x87\x8D\xE5\x87\xBB\x0A";//��ȷ��!!!!!!!!!!!!!!! Attack 1 + Attack 2 ���+�ػ� 

				const char* items[] = { attack_1.c_str() , attack_2.c_str() , attack_3.c_str() };
				ImGui::Combo( "\xE5\x88\x80\xE4\xBA\xBA\xE6\xA8\xA1\xE5\xBC\x8F\x0A" , &Settings::Knifebot::knf_Attack , items , IM_ARRAYSIZE( items ) );//��ȷ��!!!!!!!!!!!!!!! Attack type ����ģʽ

				ImGui::Separator();

				ImGui::SliderInt( "\xE5\x8C\xBA\xE5\x88\x86\xE8\xBD\xBB\xE5\x87\xBB\x0A" , &Settings::Knifebot::knf_DistAttack , 1 , 100 );//��ȷ��!!!!!!!!!!!!!!!Dist to attack 1 �������
				ImGui::SliderInt( "\xE5\x8C\xBA\xE5\x88\x86\xE9\x87\x8D\xE5\x87\xBB\x0A" , &Settings::Knifebot::knf_DistAttack2 , 1 , 100 );//��ȷ��!!!!!!!!!!!!!!!Dist to attack 2 �����ػ�
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

				ImGui::Text( "\xE5\xBD\x93\xE5\x89\x8D\xE6\xAD\xA6\xE5\x99\xA8\x0A: %s" , pWeaponData[iWeaponID] );//��ȷ��!!!!!!!!!!!!!!!Current Weapon: %s

				ImGui::PushItemWidth( 362.f );

				ImGui::Separator();

				ImGui::Combo( "CT\xE5\x88\x80\xE6\xA8\xA1\xE5\x9E\x8B\x0A" , &Settings::Skin::knf_ct_model , knife_models_items , IM_ARRAYSIZE( knife_models_items ) );//��ȷ��!!!!!!!!!!!!!!! Knife CT Model CT��ģ��
				ImGui::Combo( "T\xE5\x88\x80\xE6\xA8\xA1\xE5\x9E\x8B\x0A" , &Settings::Skin::knf_tt_model , knife_models_items , IM_ARRAYSIZE( knife_models_items ) );//��ȷ��!!!!!!!!!!!!!!! Knife TT Model T��ģ��

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

				KnifeCTModel += " Skin##KCT";//��ȷ��!!!!!!!!!!!!!!!
				KnifeTTModel += " Skin##KTT";//��ȷ��!!!!!!!!!!!!!!!

				ImGui::SliderFloat( "Knife CT Wear" , &g_SkinChangerCfg[WEAPON_KNIFE].flFallbackWear , 0.f , 1.f );
				ImGui::Combo( "Knife CT Qality" , &g_SkinChangerCfg[WEAPON_KNIFE].iEntityQuality , quality_items , IM_ARRAYSIZE( quality_items ) );//��ȷ��!!!!!!!!!!!!!!!
				ImGui::ComboBoxArray( KnifeCTModel.c_str() , &iSelectKnifeCTSkinIndex , KnifeSkins[iKnifeCTModelIndex].SkinNames );//��ȷ��!!!!!!!!!!!!!!!

				ImGui::Separator();

				ImGui::SliderFloat( "Knife TT Wear" , &g_SkinChangerCfg[WEAPON_KNIFE_T].flFallbackWear , 0.f , 1.f );
				ImGui::Combo( "Knife TT Qality" , &g_SkinChangerCfg[WEAPON_KNIFE_T].iEntityQuality , quality_items , IM_ARRAYSIZE( quality_items ) );//��ȷ��!!!!!!!!!!!!!!!
				ImGui::ComboBoxArray( KnifeTTModel.c_str() , &iSelectKnifeTTSkinIndex , KnifeSkins[iKnifeTTModelIndex].SkinNames );//��ȷ��!!!!!!!!!!!!!!!

				ImGui::Separator();

				static int iOldWeaponID = -1;

				ImGui::Combo( "Weapon##WeaponSelect" , &iWeaponID , pWeaponData , IM_ARRAYSIZE( pWeaponData ) );//��ȷ��!!!!!!!!!!!!!!!

				iWeaponSelectIndex = pWeaponItemIndexData[iWeaponID];

				if ( iOldWeaponID != iWeaponID )
					iWeaponSelectSkinIndex = GetWeaponSkinIndexFromPaintKit( g_SkinChangerCfg[iWeaponSelectIndex].nFallbackPaintKit );

				iOldWeaponID = iWeaponID;

				string WeaponSkin = pWeaponData[iWeaponID];
				WeaponSkin += " Skin";//��ȷ��!!!!!!!!!!!!!!!

				ImGui::ComboBoxArray( WeaponSkin.c_str() , &iWeaponSelectSkinIndex , WeaponSkins[iWeaponID].SkinNames );

				ImGui::Combo("\xE5\x93\x81\xE8\xB4\xA8\x0A", &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].iEntityQuality, quality_items, IM_ARRAYSIZE(quality_items));//��ȷ��!!!!!!!!!!!!!!!Weapon Qality ����Ʒ��
				ImGui::SliderFloat( "\xE7\xA3\xA8\xE6\x8D\x9F\x0A" , &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].flFallbackWear , 0.f , 1.f );//��ȷ��!!!!!!!!!!!!!!!Weapon Wear ����ĥ��
				ImGui::InputInt( "\xE6\x9A\x97\xE9\x87\x91\x0A" , &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].nFallbackStatTrak , 1 , 100 , ImGuiInputTextFlags_CharsDecimal );//��ȷ��!!!!!!!!!!!!!!! Weapon StatTrak ��������

				ImGui::Separator();

				ImGui::Combo( "\xE6\x89\x8B\xE5\xA5\x97\x0A" , &Settings::Skin::gloves_skin , gloves_listbox_items ,//��ȷ��!!!!!!!!!!!!!!!Gloves Skin ����Ƥ��
							  IM_ARRAYSIZE( gloves_listbox_items ) );

				ImGui::Separator();

				ImGui::PopItemWidth();

				if ( ImGui::Button( "\xE5\xBA\x94\xE7\x94\xA8##\xE7\x9A\xAE\xE8\x82\xA4" ) )//��ȷ��!!!!!!!!!!!!!!!Apply##Skin ����Ƥ��
				{
					g_SkinChangerCfg[iWeaponSelectIndex].nFallbackPaintKit = WeaponSkins[iWeaponID].SkinPaintKit[iWeaponSelectSkinIndex];

					Settings::Skin::knf_ct_skin = KnifeSkins[iKnifeCTModelIndex].SkinPaintKit[iSelectKnifeCTSkinIndex];
					Settings::Skin::knf_tt_skin = KnifeSkins[iKnifeTTModelIndex].SkinPaintKit[iSelectKnifeTTSkinIndex];

					ForceFullUpdate();
				}
			}
			else if ( tabSelected == 6 ) // Misc
			{
				ImGui::Checkbox( "\xE8\xBF\x9E\xE8\xB7\xB3\x0A" , &Settings::Misc::misc_Bhop );//Bhop ����
				ImGui::Checkbox( "punch" , &Settings::Misc::misc_Punch );//Punch ?
				ImGui::Checkbox( "\xE7\x8B\x99\xE5\x87\xBB\xE6\x9E\xAA\xE5\x87\x86\xE5\xBF\x83" , &Settings::Misc::misc_SniperAim );//SniperAim �ѻ�׼��
				ImGui::Checkbox( "\xE6\x97\xA0\xE9\x97\xAA\xE5\x85\x89\x0A" , &Settings::Misc::misc_NoFlash );//NoFlash ������
				ImGui::Checkbox( "\xE8\x87\xAA\xE5\x8A\xA8\xE6\x89\xAB\xE5\xB0\x84\x0A" , &Settings::Misc::misc_AutoStrafe );//AutoStrafe �Զ�ɨ��
				ImGui::Checkbox( "\xE8\x87\xAA\xE5\x8A\xA8\xE7\xA1\xAE\xE8\xAE\xA4\x0A" , &Settings::Misc::misc_AutoAccept );//AutoAccept �Զ�ȷ��
				ImGui::Checkbox( "\xE8\xA7\x82\xE5\xAF\x9F\xE8\x80\x85\xE5\x88\x97\xE8\xA1\xA8\x0A" , &Settings::Misc::misc_Spectators );//Spectators �۲����б�
				ImGui::Checkbox( "\xE8\xB7\x9D\xE7\xA6\xBB\x0A" , &Settings::Misc::misc_FovChanger );//Fov Changer ���뿪��
				ImGui::PushItemWidth( 362.f );
				ImGui::SliderInt( "\xE8\xA7\x86\xE9\x87\x8E\xE8\x8C\x83\xE5\x9B\xB4\x0A" , &Settings::Misc::misc_FovView , 1 , 190 );//Fov View ��Ұ��Χ
				ImGui::SliderInt( "\xE6\xA8\xA1\xE5\x9E\x8B\xE8\xB7\x9D\xE7\xA6\xBB\x0A" , &Settings::Misc::misc_FovModelView , 1 , 190 );//Fov Model View ģ�;���
				ImGui::Separator();
				ImGui::ColorEdit3( "\xE5\x87\x86\xE5\xBF\x83\xE9\xA2\x9C\xE8\x89\xB2" , Settings::Misc::misc_SniperAimColor );//Awp Aim Color AWP������ɫ
				ImGui::PopItemWidth();
			}
			else if ( tabSelected == 7 ) // Config
			{
				static int iConfigSelect = 0;
				static int iMenuSheme = 1;
				static char ConfigName[64] = { 0 };

				ImGui::ComboBoxArray( "\xE9\x80\x89\xE6\x8B\xA9\xE9\x85\x8D\xE7\xBD\xAE\x0A" , &iConfigSelect , ConfigList );//Select Config ѡ������

				ImGui::Separator();

				if ( ImGui::Button( "\xE8\xBD\xBD\xE5\x85\xA5\xE9\x85\x8D\xE7\xBD\xAE\x0A" ) & ConfigName != NULL )//Load Config ��������
				{
					Settings::LoadSettings( BaseDir + "\\" + ConfigList[iConfigSelect] );
				}
				ImGui::SameLine();
				if ( ImGui::Button( "\xE4\xBF\x9D\xE5\xAD\x98\xE9\x85\x8D\xE7\xBD\xAE\x0A") & ConfigName != NULL )//Save Config ��������
				{
					Settings::SaveSettings( BaseDir + "\\" + ConfigList[iConfigSelect] );
				}
				ImGui::SameLine();
				if ( ImGui::Button( "\xE5\x88\xB7\xE6\x96\xB0\xE9\x85\x8D\xE7\xBD\xAE\xE5\x88\x97\xE8\xA1\xA8\x0A" ) )//Refresh Config List ˢ�������б�
				{
					RefreshConfigs();
				}

				ImGui::Separator();

				ImGui::InputText( "\xE9\x85\x8D\xE7\xBD\xAE\xE5\x90\x8D\xE5\xAD\x97\x0A" , ConfigName , 64 );//Config Name ��������

				if ( ImGui::Button( "\xE5\x88\x9B\xE5\xBB\xBA\x26\xE4\xBF\x9D\xE5\xAD\x98\xE6\x96\xB0\xE9\x85\x8D\xE7\xBD\xAE\x0A" ) )//Create & Save new Config ����&����������
				{
					string ConfigFileName = ConfigName;

					if ( ConfigFileName.size() < 1 )
					{
						ConfigFileName = "\xE8\xAE\xBE\xE7\xBD\xAE\x0A";//Settings ����
					}

					Settings::SaveSettings( BaseDir + "\\" + ConfigFileName + ".ini" );
					RefreshConfigs();
				}

				ImGui::Separator();

				const char* ThemesList[] = { "Default" , "Red" };

				ImGui::Combo( "\xE7\x95\x8C\xE9\x9D\xA2\xE9\xA2\x9C\xE8\x89\xB2\xE6\x96\xB9\xE6\xA1\x88\x0A" , &iMenuSheme , ThemesList ,  IM_ARRAYSIZE( ThemesList ) );//��ȷ��!!!!!!!!!!!!!!! Menu Color Sheme ������ɫ����

				ImGui::Separator();

				if ( ImGui::Button( "\xE6\x8E\xA5\xE5\x8F\x97\xE6\x96\xB9\xE6\xA1\x88\x0A" ) )//��ȷ��!!!!!!!!!!!!!!! Apply Sheme ���ܷ���
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