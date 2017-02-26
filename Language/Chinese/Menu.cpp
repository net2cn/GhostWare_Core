

#include "Menu.h"
#include "Controls.h"
#include "Hooks.h" // for the unload meme
#include "Interfaces.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 645

AyyWareWindow Menu::Window;

void SaveCallbk()
{
	GUI.SaveWindowState(&Menu::Window, "config.xml");
}

void LoadCallbk()
{
	GUI.LoadWindowState(&Menu::Window, "config.xml");
}

void UnLoadCallbk()
{
	DoUnload = true;
}

void KnifeApplyCallbk()
{
	static ConVar* Meme = Interfaces::CVar->FindVar("cl_fullupdate");
	Meme->nFlags &= ~FCVAR_CHEAT;
	Interfaces::Engine->ClientCmd_Unrestricted("cl_fullupdate");
}

void AyyWareWindow::Setup()
{
	SetPosition(12, 49);
	SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	SetTitle("Csgo���鸨��by.Emo129");

	RegisterTab(&LegitBotTab);
	RegisterTab(&RageBotTab);
	RegisterTab(&VisualsTab);
	RegisterTab(&MiscTab);
	RegisterTab(&ColorsTab);
	RECT Client = GetClientArea();
	Client.bottom -= 29;

	LegitBotTab.Setup();
	RageBotTab.Setup();
	VisualsTab.Setup();
	MiscTab.Setup();
	ColorsTab.Setup();

#pragma region Bottom Buttons
	SaveButton.SetText("��������");
	SaveButton.SetCallback(SaveCallbk);
	SaveButton.SetPosition(16, Client.bottom - 42);

	LoadButton.SetText("��������");
	LoadButton.SetCallback(LoadCallbk);
	LoadButton.SetPosition(203, Client.bottom - 42);
	
	UnloadButton.SetText("ж������");
	UnloadButton.SetCallback(UnLoadCallbk);
	UnloadButton.SetPosition(396, Client.bottom - 42);

	MiscTab.RegisterControl(&SaveButton);
	MiscTab.RegisterControl(&LoadButton);
	MiscTab.RegisterControl(&UnloadButton);

#pragma endregion Setting up the settings buttons
}

void CRageBotTab::Setup()
{
	SetTitle("����");

	ActiveLabel.SetPosition(16, 16);
	ActiveLabel.SetText("����");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetPosition(66, 16);
	RegisterControl(&Active);

#pragma region Aimbot

	AimbotGroup.SetPosition(16, 48);
	AimbotGroup.SetText("��������");
	AimbotGroup.SetSize(376, 290);
	RegisterControl(&AimbotGroup);

	AimbotEnable.SetFileId("aim_enable");
	AimbotGroup.PlaceLabledControl("����", this, &AimbotEnable);

	AimbotAutoFire.SetFileId("aim_autofire");
	AimbotGroup.PlaceLabledControl("�Զ�����", this, &AimbotAutoFire);

	AimbotFov.SetFileId("aim_fov");
	AimbotFov.SetBoundaries(0.f, 180.f);
	AimbotFov.SetValue(0.f);
	AimbotGroup.PlaceLabledControl("���鷶Χ", this, &AimbotFov);

	AimbotSilentAim.SetFileId("aim_silent");
	AimbotGroup.PlaceLabledControl("��Ĭ����", this, &AimbotSilentAim);

	AimbotPSilent.SetFileId("aim_psilent");
	AimbotGroup.PlaceLabledControl("������Ĭ", this, &AimbotPSilent);

	AimbotAutoPistol.SetFileId("aim_autopistol");
	AimbotGroup.PlaceLabledControl("�Զ���ǹ", this, &AimbotAutoPistol);

	AimbotAimStep.SetFileId("aim_aimstep");
	AimbotGroup.PlaceLabledControl("Speed Limit", this, &AimbotAimStep);


	//AimbotAimStep.SetFileId("aim_aimstep");
	//AimbotAimStep.SetBoundaries(0.f, 180.f);
	//AimbotAimStep.SetValue(39.f);
	//AimbotGroup.PlaceLabledControl("Speed Limit", this, &AimbotAimStep);

	AimbotKeyPress.SetFileId("aim_usekey");
	AimbotGroup.PlaceLabledControl("��������", this, &AimbotKeyPress);

	AimbotKeyBind.SetFileId("aim_key");
	AimbotGroup.PlaceLabledControl("���鰴��", this, &AimbotKeyBind);

	AimbotChicken.SetFileId("aim_chicken");
	AimbotGroup.PlaceLabledControl("���鼦", this, &AimbotChicken);

#pragma endregion Aimbot Controls Get Setup in here

#pragma region Target
	TargetGroup.SetPosition(16, 354);
	TargetGroup.SetText("Ŀ��ѡ��");
	TargetGroup.SetSize(376, 220);
	RegisterControl(&TargetGroup);

	TargetSelection.SetFileId("tgt_selection");
	TargetSelection.AddItem("��׼�����");
	TargetSelection.AddItem("����");
	TargetSelection.AddItem("Ѫ�����ٵ�");
	TargetGroup.PlaceLabledControl("���鲿λ", this, &TargetSelection);

	TargetFriendlyFire.SetFileId("tgt_friendlyfire");
	TargetGroup.PlaceLabledControl("�Ѿ��˺�", this, &TargetFriendlyFire);

	TargetHitbox.SetFileId("tgt_hitbox");
	TargetHitbox.AddItem("ͷ��");
	TargetHitbox.AddItem("����");
	TargetHitbox.AddItem("�ز�");
	TargetHitbox.AddItem("����");
	TargetGroup.PlaceLabledControl("������λ", this, &TargetHitbox);

	TargetHitscan.SetFileId("tgt_hitscan");
	TargetHitscan.AddItem("�ر�"); //0
	TargetHitscan.AddItem("��Ч��"); // 1
	TargetHitscan.AddItem("�е�"); // 2
	TargetHitscan.AddItem("��Ч��"); // 3
	TargetGroup.PlaceLabledControl("ɨ���ٶ�", this, &TargetHitscan);

#pragma endregion Targetting controls 

#pragma region Accuracy
	AccuracyGroup.SetPosition(408, 48);
	AccuracyGroup.SetText("׼��");
	AccuracyGroup.SetSize(360, 290);
	RegisterControl(&AccuracyGroup);

	AccuracySpread.SetFileId("acc_nospread");
	AccuracyGroup.PlaceLabledControl("����ɢ", this, &AccuracySpread);

	AccuracyRecoil.SetFileId("acc_norecoil");
	AccuracyGroup.PlaceLabledControl("�޺���", this, &AccuracyRecoil);

	AccuracyAutoWall.SetFileId("acc_awall");
	AccuracyGroup.PlaceLabledControl("�Զ���ǽ", this, &AccuracyAutoWall);

	AccuracyAutoStop.SetFileId("acc_stop");
	AccuracyGroup.PlaceLabledControl("�Զ���ͣ", this, &AccuracyAutoStop);

	AccuracyAutoCrouch.SetFileId("acc_crouch");
	AccuracyGroup.PlaceLabledControl("�Զ���", this, &AccuracyAutoCrouch);

	AccuracySpreadLimit.SetFileId("acc_spreadlimon");
	AccuracyGroup.PlaceLabledControl("���м���", this, &AccuracySpreadLimit);

	AccuracyMinimumSpread.SetFileId("acc_spreadlim");
	AccuracyMinimumSpread.SetBoundaries(0.f, 5.0f);
	AccuracyMinimumSpread.SetValue(1.5f);
	AccuracyGroup.PlaceLabledControl("���л���", this, &AccuracyMinimumSpread);

	AccuracyMinimumDamage.SetFileId("acc_mindmg");
	AccuracyMinimumDamage.SetBoundaries(1.f, 100.f);
	AccuracyMinimumDamage.SetValue(1.f);
	AccuracyGroup.PlaceLabledControl("��С�˺�", this, &AccuracyMinimumDamage);

	AccuracyAngleFix.SetFileId("acc_aaa");
	AccuracyGroup.PlaceLabledControl("λ�ý���", this, &AccuracyAngleFix);

#pragma endregion  Accuracy controls get Setup in here

#pragma region AntiAim
	AntiAimGroup.SetPosition(408, 354);
	AntiAimGroup.SetText("������");
	AntiAimGroup.SetSize(360, 220);
	RegisterControl(&AntiAimGroup);

	AntiAimEnable.SetFileId("aa_enable");
	AntiAimGroup.PlaceLabledControl("����", this, &AntiAimEnable);

	AntiAimPitch.SetFileId("aa_x");
	AntiAimPitch.AddItem("�ر�");
	AntiAimPitch.AddItem("��ͷ");
	AntiAimPitch.AddItem("̧ͷ");
	AntiAimPitch.AddItem("����");

	AntiAimGroup.PlaceLabledControl("X��", this, &AntiAimPitch);

	AntiAimYaw.SetFileId("aa_y");
	AntiAimYaw.AddItem("�ر�");
	AntiAimYaw.AddItem("��ת");
	AntiAimYaw.AddItem("��ת");
	AntiAimYaw.AddItem("����");
	AntiAimYaw.AddItem("��ת");
	AntiAimYaw.AddItem("����");
	AntiAimYaw.AddItem("αװ����");
	AntiAimYaw.AddItem("αװ����");
	AntiAimYaw.AddItem("αװ����");

	AntiAimGroup.PlaceLabledControl("Y��", this, &AntiAimYaw);

	//AntiAimX.SetFileId("aa_antiaimX");
//	AntiAimX.SetBoundaries(0.f, 180.f);
//	AntiAimX.SetValue(0.f);
//	AntiAimGroup.PlaceLabledControl("X Value", this, &AntiAimX);

//	AntiAimY.SetFileId("aa_antiaimY");
//	AntiAimY.SetBoundaries(0.f, 180.f);
//	AntiAimY.SetValue(0.f);
//	AntiAimGroup.PlaceLabledControl("Y Value", this, &AntiAimY);


#pragma endregion  AntiAim controls get setup in here
}

void CLegitBotTab::Setup()
{
	SetTitle("΢����");

	ActiveLabel.SetPosition(16, 16);
	ActiveLabel.SetText("����");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetPosition(66, 16);
	RegisterControl(&Active);

#pragma region Aimbot
	AimbotGroup.SetPosition(16, 48);
	AimbotGroup.SetText("����");
	AimbotGroup.SetSize(240, 210);
	RegisterControl(&AimbotGroup);

	AimbotEnable.SetFileId("aim_enable");
	AimbotGroup.PlaceLabledControl("Enable", this, &AimbotEnable);

	AimbotAutoFire.SetFileId("aim_autofire");
	AimbotGroup.PlaceLabledControl("�Զ�����", this, &AimbotAutoFire);

	AimbotFriendlyFire.SetFileId("aim_friendfire");
	AimbotGroup.PlaceLabledControl("�Ѿ��˺�", this, &AimbotFriendlyFire);

	AimbotKeyPress.SetFileId("aim_usekey");
	AimbotGroup.PlaceLabledControl("��������", this, &AimbotKeyPress);

	AimbotKeyBind.SetFileId("aim_key");
	AimbotGroup.PlaceLabledControl("���鰴��", this, &AimbotKeyBind);
	
	AimbotAutoPistol.SetFileId("aim_apistol");
	AimbotGroup.PlaceLabledControl("�Զ���ǹ", this, &AimbotAutoPistol);

#pragma endregion Aimbot shit

#pragma region Triggerbot
	TriggerGroup.SetPosition(272, 48);
	TriggerGroup.SetText("���");
	TriggerGroup.SetSize(240, 210);
	RegisterControl(&TriggerGroup);

	TriggerEnable.SetFileId("trig_enable");
	TriggerGroup.PlaceLabledControl("����", this, &TriggerEnable);

	TriggerKeyPress.SetFileId("trig_onkey");
	TriggerGroup.PlaceLabledControl("�������", this, &TriggerKeyPress);

	TriggerKeyBind.SetFileId("trig_key");
	TriggerGroup.PlaceLabledControl("�������", this, &TriggerKeyBind);
#pragma endregion Triggerbot stuff

#pragma region Main Weapon
	WeaponMainGroup.SetPosition(16, 274);
	WeaponMainGroup.SetText("��ǹ/����");
	WeaponMainGroup.SetSize(240, 210);
	RegisterControl(&WeaponMainGroup);

	WeaponMainSpeed.SetFileId("main_speed");
	WeaponMainSpeed.SetBoundaries(0.1f, 2.f); 
	WeaponMainSpeed.SetValue(1.0f);
	WeaponMainGroup.PlaceLabledControl("�����ٶ�", this, &WeaponMainSpeed);

	WeaponMainFoV.SetFileId("main_fov");
	WeaponMainFoV.SetBoundaries(0.1f, 30.f);
	WeaponMainFoV.SetValue(5.f);
	WeaponMainGroup.PlaceLabledControl("���鷶Χ", this, &WeaponMainFoV);

	WeaponMainRecoil.SetFileId("main_recoil");
	WeaponMainGroup.PlaceLabledControl("������", this, &WeaponMainRecoil);

	WeaponMainHitbox.SetFileId("main_hitbox");
	WeaponMainHitbox.AddItem("ͷ");
	WeaponMainHitbox.AddItem("����");
	WeaponMainHitbox.AddItem("�ز�");
	WeaponMainHitbox.AddItem("����");
	WeaponMainGroup.PlaceLabledControl("���鲿λ", this, &WeaponMainHitbox);
#pragma endregion

#pragma region Pistols
	WeaponPistGroup.SetPosition(272, 274);
	WeaponPistGroup.SetText("��ǹ");
	WeaponPistGroup.SetSize(240, 210);
	RegisterControl(&WeaponPistGroup);

	WeaponPistSpeed.SetFileId("pist_speed");
	WeaponPistSpeed.SetBoundaries(0.1f, 2.f);
	WeaponPistSpeed.SetValue(1.0f);
	WeaponPistGroup.PlaceLabledControl("�����ٶ�", this, &WeaponPistSpeed);

	WeaponPistFoV.SetFileId("pist_fov");
	WeaponPistFoV.SetBoundaries(0.1f, 30.f);
	WeaponPistFoV.SetValue(5.f);
	WeaponPistGroup.PlaceLabledControl("���鷶Χ", this, &WeaponPistFoV);

	WeaponPistRecoil.SetFileId("pist_recoil");
	WeaponPistGroup.PlaceLabledControl("������", this, &WeaponPistRecoil);

	WeaponPistHitbox.SetFileId("pist_hitbox");
	WeaponPistHitbox.AddItem("ͷ��");
	WeaponPistHitbox.AddItem("����");
	WeaponPistHitbox.AddItem("�ز�");
	WeaponPistHitbox.AddItem("θ��");
	WeaponPistGroup.PlaceLabledControl("���鲿λ", this, &WeaponPistHitbox);
#pragma endregion

#pragma region Snipers
	WeaponSnipGroup.SetPosition(528, 274);
	WeaponSnipGroup.SetText("�ѻ�ǹ");
	WeaponSnipGroup.SetSize(240, 210);
	RegisterControl(&WeaponSnipGroup);

	WeaponSnipSpeed.SetFileId("snip_speed");
	WeaponSnipSpeed.SetBoundaries(0.1f, 2.f);
	WeaponSnipSpeed.SetValue(1.0f);
	WeaponSnipGroup.PlaceLabledControl("�����ٶ�", this, &WeaponSnipSpeed);

	WeaponSnipFoV.SetFileId("snip_fov");
	WeaponSnipFoV.SetBoundaries(0.1f, 30.f);
	WeaponSnipFoV.SetValue(5.f);
	WeaponSnipGroup.PlaceLabledControl("���鷶Χ", this, &WeaponSnipFoV);

	WeaponSnipRecoil.SetFileId("snip_recoil");
	WeaponSnipGroup.PlaceLabledControl("������", this, &WeaponSnipRecoil);

	WeaponSnipHitbox.SetFileId("snip_hitbox");
	WeaponSnipHitbox.AddItem("ͷ��");
	WeaponSnipHitbox.AddItem("����");
	WeaponSnipHitbox.AddItem("�ز�");
	WeaponSnipHitbox.AddItem("����");
	WeaponSnipGroup.PlaceLabledControl("���鲿λ", this, &WeaponSnipHitbox);
#pragma endregion
}

void CVisualTab::Setup()
{
	SetTitle("͸��");

	ActiveLabel.SetPosition(16, 16);
	ActiveLabel.SetText("����");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetPosition(66, 16);
	RegisterControl(&Active);

#pragma region Options
	OptionsGroup.SetText("��ʾѡ��");
	OptionsGroup.SetPosition(16, 48);
	OptionsGroup.SetSize(193, 430);
	RegisterControl(&OptionsGroup);

	OptionsBox.SetFileId("opt_box");
	OptionsGroup.PlaceLabledControl("����", this, &OptionsBox);

	OptionsName.SetFileId("opt_name");
	OptionsGroup.PlaceLabledControl("����", this, &OptionsName);

	OptionsHealth.SetFileId("opt_hp");
	OptionsGroup.PlaceLabledControl("Ѫ��", this, &OptionsHealth);

	OptionsWeapon.SetFileId("opt_weapon");
	OptionsGroup.PlaceLabledControl("����", this, &OptionsWeapon);

	OptionsInfo.SetFileId("opt_info");
	OptionsGroup.PlaceLabledControl("��Ϣ", this, &OptionsInfo);

	//Broken
	//OptionsChams.SetFileId("opt_chams");
	//OptionsChams.AddItem("�رղ���");
	//OptionsChams.AddItem("��ɫ");
	//OptionsChams.AddItem("ƽ̹��");
	//OptionsGroup.PlaceLabledControl("����", this, &OptionsChams);

	OptionsSkeleton.SetFileId("opt_bone");
	OptionsGroup.PlaceLabledControl("�Ǽ�", this, &OptionsSkeleton);

	OptionsAimSpot.SetFileId("opt_aimspot");
	OptionsGroup.PlaceLabledControl("ͷ��", this, &OptionsAimSpot);
	
	OptionsCompRank.SetFileId("opt_comprank");
	OptionsGroup.PlaceLabledControl("��λ", this, &OptionsCompRank);

#pragma endregion Setting up the Options controls

#pragma region Filters
	FiltersGroup.SetText("����ѡ��");
	FiltersGroup.SetPosition(225, 48);
	FiltersGroup.SetSize(193, 430);
	RegisterControl(&FiltersGroup);

	FiltersAll.SetFileId("ftr_all");
	FiltersGroup.PlaceLabledControl("ȫѡ", this, &FiltersAll);

	FiltersPlayers.SetFileId("ftr_players");
	FiltersGroup.PlaceLabledControl("���", this, &FiltersPlayers);

	FiltersEnemiesOnly.SetFileId("ftr_enemyonly");
	FiltersGroup.PlaceLabledControl("����", this, &FiltersEnemiesOnly);

	FiltersWeapons.SetFileId("ftr_weaps");
	FiltersGroup.PlaceLabledControl("Я������", this, &FiltersWeapons);

	FiltersChickens.SetFileId("ftr_chickens");
	FiltersGroup.PlaceLabledControl("��", this, &FiltersChickens);

	//Broken
	//FiltersC4.SetFileId("ftr_c4");
	//FiltersGroup.PlaceLabledControl("C4", this, &FiltersC4);
#pragma endregion Setting up the Filters controls

#pragma region Other
	OtherGroup.SetText("����");
	OtherGroup.SetPosition(434, 48);
	OtherGroup.SetSize(334, 430);
	RegisterControl(&OtherGroup);

	OtherCrosshair.SetFileId("otr_xhair");
	OtherGroup.PlaceLabledControl("׼��", this, &OtherCrosshair);

	OtherRecoilCrosshair.SetFileId("otr_recoilhair");
	OtherRecoilCrosshair.AddItem("�ر�");
	OtherRecoilCrosshair.AddItem("������λ��");
	OtherRecoilCrosshair.AddItem("�������뾶");
	OtherGroup.PlaceLabledControl("������׼��", this, &OtherRecoilCrosshair);

	OtherNoVisualRecoil.SetFileId("otr_visrecoil");
	OtherGroup.PlaceLabledControl("ȡ������������", this, &OtherNoVisualRecoil);

	OtherNoHands.SetFileId("otr_hands");
	OtherNoHands.AddItem("�ر�");
	OtherNoHands.AddItem("����");
	OtherNoHands.AddItem("͸��");
	OtherNoHands.AddItem("����");
	OtherNoHands.AddItem("��˹��");
	OtherGroup.PlaceLabledControl("��", this, &OtherNoHands);

#pragma endregion Setting up the Other controls
}
void CColorsTab::Setup()
{
	SetTitle("��ɫ");
	RGBGroup.SetText("������ɫ");
	RGBGroup.SetPosition(16, 48);
	RGBGroup.SetSize(334, 280);
	RegisterControl(&RGBGroup);
#pragma region Colors
	MenuR.SetFileId("clr_menuR");
	MenuR.SetBoundaries(0.f, 255.f);
	MenuR.SetValue(150.0f);
	RGBGroup.PlaceLabledControl("��", this, &MenuR);

	MenuG.SetFileId("clr_menuG");
	MenuG.SetBoundaries(0.f, 255.f);
	MenuG.SetValue(1.0f);
	RGBGroup.PlaceLabledControl("��", this, &MenuG);

	MenuB.SetFileId("clr_menuB");
	MenuB.SetBoundaries(0.f, 255.f);
	MenuB.SetValue(1.0f);
	RGBGroup.PlaceLabledControl("��", this, &MenuB);

}
void CMiscTab::Setup()
{
	SetTitle("����");

//Broken
//#pragma region Knife
//	KnifeGroup.SetPosition(408, 48);
//	KnifeGroup.SetSize(360, 95);
//	KnifeGroup.SetText("�� Ƥ��");
//	RegisterControl(&KnifeGroup);
//
//	KnifeEnable.SetFileId("knife_enable");
//	KnifeGroup.PlaceLabledControl("����", this, &KnifeEnable);
//	//bayonet karambit flip gut m9 hunts butterfly
//	KnifeModel.SetFileId("knife_model");
//	KnifeModel.AddItem("�ر�");
//	KnifeModel.AddItem("�̵�");
//	KnifeModel.AddItem("צ�ӵ�");
//	KnifeModel.AddItem("�䵶");
//	KnifeModel.AddItem("ɱ��");
//	KnifeModel.AddItem("M9 �̵�");
//	KnifeModel.AddItem("����");
//	KnifeModel.AddItem("������");
//	KnifeModel.AddItem("��Ӱ˫ذ");
//	KnifeModel.AddItem("�䵶2");
//	KnifeModel.AddItem("�����Ե�");
//	KnifeGroup.PlaceLabledControl("��", this, &KnifeModel);
//
//	//KnifeSkin.SetFileId("knife_skin");
//	//KnifeSkin.AddItem("Doppler Sapphire");
//	//KnifeSkin.AddItem("Doppler Ruby");
//	//KnifeSkin.AddItem("Tiger");
//	//KnifeSkin.AddItem("Plain");
//	//KnifeGroup.PlaceLabledControl("Skin", this, &KnifeSkin);
//
//	KnifeApply.SetText("����");
//	KnifeApply.SetCallback(KnifeApplyCallbk);
//	KnifeGroup.PlaceLabledControl("", this, &KnifeApply);
//
//#pragma endregion

#pragma region Other
	OtherGroup.SetPosition(16, 48);
	OtherGroup.SetSize(376, 280);
	OtherGroup.SetText("����");
	RegisterControl(&OtherGroup);

	OtherAutoJump.SetFileId("otr_autojump");
	OtherGroup.PlaceLabledControl("����", this, &OtherAutoJump);

	OtherAutoStrafe.SetFileId("otr_autostrafe");
	OtherGroup.PlaceLabledControl("�Զ�ɨ��", this, &OtherAutoStrafe);

	OtherSafeMode.SetFileId("otr_safemode");
	OtherSafeMode.SetState(true);
	OtherGroup.PlaceLabledControl("��ȫģʽ", this, &OtherSafeMode);

	OtherChatSpam.SetFileId("otr_spam");
	OtherChatSpam.AddItem("ˢ��");
	OtherChatSpam.AddItem("�غ�");
	OtherChatSpam.AddItem("����");
	OtherChatSpam.AddItem("�ٱ�");
	OtherGroup.PlaceLabledControl("ˢ��", this, &OtherChatSpam);

	OtherAirStuck.SetFileId("otr_astuck");
	OtherGroup.PlaceLabledControl("����", this, &OtherAirStuck);

	OtherSpectators.SetFileId("otr_speclist");
	OtherGroup.PlaceLabledControl("�۲����б�", this, &OtherSpectators);

	OtherServerLag.SetFileId("otr_serverlag");
	OtherGroup.PlaceLabledControl("�������ӳ�", this, &OtherServerLag);
	
	OtherFakeLag.SetFileId("otr_fakelag");
	OtherGroup.PlaceLabledControl("��ping", this, &OtherFakeLag);

#pragma endregion other random options
}

void Menu::SetupMenu()
{
	Window.Setup();

	GUI.RegisterWindow(&Window);
	GUI.BindWindow(VK_INSERT, &Window);
}

void Menu::DoUIFrame()
{
	// General Processing

	// If the "all filter is selected tick all the others
	if (Window.VisualsTab.FiltersAll.GetState())
	{
		Window.VisualsTab.FiltersEnemiesOnly.SetState(true);
		Window.VisualsTab.FiltersC4.SetState(true);
		Window.VisualsTab.FiltersChickens.SetState(true);
		Window.VisualsTab.FiltersPlayers.SetState(true);
		Window.VisualsTab.FiltersWeapons.SetState(true);
	}
	if (Window.MiscTab.OtherSafeMode.GetState())
	{
		Window.RageBotTab.AccuracySpread.SetState(false);
	}
	
	

	
	GUI.Update();
	GUI.Draw();

	
}


