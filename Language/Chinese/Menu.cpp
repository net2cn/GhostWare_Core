

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
	SetTitle("Csgo幽灵辅助by.Emo129");

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
	SaveButton.SetText("保存配置");
	SaveButton.SetCallback(SaveCallbk);
	SaveButton.SetPosition(16, Client.bottom - 42);

	LoadButton.SetText("载入配置");
	LoadButton.SetCallback(LoadCallbk);
	LoadButton.SetPosition(203, Client.bottom - 42);
	
	UnloadButton.SetText("卸载配置");
	UnloadButton.SetCallback(UnLoadCallbk);
	UnloadButton.SetPosition(396, Client.bottom - 42);

	MiscTab.RegisterControl(&SaveButton);
	MiscTab.RegisterControl(&LoadButton);
	MiscTab.RegisterControl(&UnloadButton);

#pragma endregion Setting up the settings buttons
}

void CRageBotTab::Setup()
{
	SetTitle("暴力");

	ActiveLabel.SetPosition(16, 16);
	ActiveLabel.SetText("启动");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetPosition(66, 16);
	RegisterControl(&Active);

#pragma region Aimbot

	AimbotGroup.SetPosition(16, 48);
	AimbotGroup.SetText("暴力自瞄");
	AimbotGroup.SetSize(376, 290);
	RegisterControl(&AimbotGroup);

	AimbotEnable.SetFileId("aim_enable");
	AimbotGroup.PlaceLabledControl("开启", this, &AimbotEnable);

	AimbotAutoFire.SetFileId("aim_autofire");
	AimbotGroup.PlaceLabledControl("自动开火", this, &AimbotAutoFire);

	AimbotFov.SetFileId("aim_fov");
	AimbotFov.SetBoundaries(0.f, 180.f);
	AimbotFov.SetValue(0.f);
	AimbotGroup.PlaceLabledControl("自瞄范围", this, &AimbotFov);

	AimbotSilentAim.SetFileId("aim_silent");
	AimbotGroup.PlaceLabledControl("静默自瞄", this, &AimbotSilentAim);

	AimbotPSilent.SetFileId("aim_psilent");
	AimbotGroup.PlaceLabledControl("完美静默", this, &AimbotPSilent);

	AimbotAutoPistol.SetFileId("aim_autopistol");
	AimbotGroup.PlaceLabledControl("自动手枪", this, &AimbotAutoPistol);

	AimbotAimStep.SetFileId("aim_aimstep");
	AimbotGroup.PlaceLabledControl("Speed Limit", this, &AimbotAimStep);


	//AimbotAimStep.SetFileId("aim_aimstep");
	//AimbotAimStep.SetBoundaries(0.f, 180.f);
	//AimbotAimStep.SetValue(39.f);
	//AimbotGroup.PlaceLabledControl("Speed Limit", this, &AimbotAimStep);

	AimbotKeyPress.SetFileId("aim_usekey");
	AimbotGroup.PlaceLabledControl("按键自瞄", this, &AimbotKeyPress);

	AimbotKeyBind.SetFileId("aim_key");
	AimbotGroup.PlaceLabledControl("自瞄按键", this, &AimbotKeyBind);

	AimbotChicken.SetFileId("aim_chicken");
	AimbotGroup.PlaceLabledControl("自瞄鸡", this, &AimbotChicken);

#pragma endregion Aimbot Controls Get Setup in here

#pragma region Target
	TargetGroup.SetPosition(16, 354);
	TargetGroup.SetText("目标选择");
	TargetGroup.SetSize(376, 220);
	RegisterControl(&TargetGroup);

	TargetSelection.SetFileId("tgt_selection");
	TargetSelection.AddItem("离准心最近");
	TargetSelection.AddItem("距离");
	TargetSelection.AddItem("血量最少的");
	TargetGroup.PlaceLabledControl("自瞄部位", this, &TargetSelection);

	TargetFriendlyFire.SetFileId("tgt_friendlyfire");
	TargetGroup.PlaceLabledControl("友军伤害", this, &TargetFriendlyFire);

	TargetHitbox.SetFileId("tgt_hitbox");
	TargetHitbox.AddItem("头部");
	TargetHitbox.AddItem("脖子");
	TargetHitbox.AddItem("胸部");
	TargetHitbox.AddItem("腹部");
	TargetGroup.PlaceLabledControl("暴力部位", this, &TargetHitbox);

	TargetHitscan.SetFileId("tgt_hitscan");
	TargetHitscan.AddItem("关闭"); //0
	TargetHitscan.AddItem("低效率"); // 1
	TargetHitscan.AddItem("中等"); // 2
	TargetHitscan.AddItem("高效率"); // 3
	TargetGroup.PlaceLabledControl("扫描速度", this, &TargetHitscan);

#pragma endregion Targetting controls 

#pragma region Accuracy
	AccuracyGroup.SetPosition(408, 48);
	AccuracyGroup.SetText("准度");
	AccuracyGroup.SetSize(360, 290);
	RegisterControl(&AccuracyGroup);

	AccuracySpread.SetFileId("acc_nospread");
	AccuracyGroup.PlaceLabledControl("无扩散", this, &AccuracySpread);

	AccuracyRecoil.SetFileId("acc_norecoil");
	AccuracyGroup.PlaceLabledControl("无后座", this, &AccuracyRecoil);

	AccuracyAutoWall.SetFileId("acc_awall");
	AccuracyGroup.PlaceLabledControl("自动穿墙", this, &AccuracyAutoWall);

	AccuracyAutoStop.SetFileId("acc_stop");
	AccuracyGroup.PlaceLabledControl("自动急停", this, &AccuracyAutoStop);

	AccuracyAutoCrouch.SetFileId("acc_crouch");
	AccuracyGroup.PlaceLabledControl("自动蹲", this, &AccuracyAutoCrouch);

	AccuracySpreadLimit.SetFileId("acc_spreadlimon");
	AccuracyGroup.PlaceLabledControl("命中纪律", this, &AccuracySpreadLimit);

	AccuracyMinimumSpread.SetFileId("acc_spreadlim");
	AccuracyMinimumSpread.SetBoundaries(0.f, 5.0f);
	AccuracyMinimumSpread.SetValue(1.5f);
	AccuracyGroup.PlaceLabledControl("击中机率", this, &AccuracyMinimumSpread);

	AccuracyMinimumDamage.SetFileId("acc_mindmg");
	AccuracyMinimumDamage.SetBoundaries(1.f, 100.f);
	AccuracyMinimumDamage.SetValue(1.f);
	AccuracyGroup.PlaceLabledControl("最小伤害", this, &AccuracyMinimumDamage);

	AccuracyAngleFix.SetFileId("acc_aaa");
	AccuracyGroup.PlaceLabledControl("位置解析", this, &AccuracyAngleFix);

#pragma endregion  Accuracy controls get Setup in here

#pragma region AntiAim
	AntiAimGroup.SetPosition(408, 354);
	AntiAimGroup.SetText("反自瞄");
	AntiAimGroup.SetSize(360, 220);
	RegisterControl(&AntiAimGroup);

	AntiAimEnable.SetFileId("aa_enable");
	AntiAimGroup.PlaceLabledControl("开启", this, &AntiAimEnable);

	AntiAimPitch.SetFileId("aa_x");
	AntiAimPitch.AddItem("关闭");
	AntiAimPitch.AddItem("低头");
	AntiAimPitch.AddItem("抬头");
	AntiAimPitch.AddItem("羊癫疯");

	AntiAimGroup.PlaceLabledControl("X轴", this, &AntiAimPitch);

	AntiAimYaw.SetFileId("aa_y");
	AntiAimYaw.AddItem("关闭");
	AntiAimYaw.AddItem("快转");
	AntiAimYaw.AddItem("慢转");
	AntiAimYaw.AddItem("背身");
	AntiAimYaw.AddItem("反转");
	AntiAimYaw.AddItem("羊癫疯");
	AntiAimYaw.AddItem("伪装羊癫疯");
	AntiAimYaw.AddItem("伪装侧身");
	AntiAimYaw.AddItem("伪装背身");

	AntiAimGroup.PlaceLabledControl("Y轴", this, &AntiAimYaw);

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
	SetTitle("微自瞄");

	ActiveLabel.SetPosition(16, 16);
	ActiveLabel.SetText("启用");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetPosition(66, 16);
	RegisterControl(&Active);

#pragma region Aimbot
	AimbotGroup.SetPosition(16, 48);
	AimbotGroup.SetText("自瞄");
	AimbotGroup.SetSize(240, 210);
	RegisterControl(&AimbotGroup);

	AimbotEnable.SetFileId("aim_enable");
	AimbotGroup.PlaceLabledControl("Enable", this, &AimbotEnable);

	AimbotAutoFire.SetFileId("aim_autofire");
	AimbotGroup.PlaceLabledControl("自动开火", this, &AimbotAutoFire);

	AimbotFriendlyFire.SetFileId("aim_friendfire");
	AimbotGroup.PlaceLabledControl("友军伤害", this, &AimbotFriendlyFire);

	AimbotKeyPress.SetFileId("aim_usekey");
	AimbotGroup.PlaceLabledControl("按键自瞄", this, &AimbotKeyPress);

	AimbotKeyBind.SetFileId("aim_key");
	AimbotGroup.PlaceLabledControl("自瞄按键", this, &AimbotKeyBind);
	
	AimbotAutoPistol.SetFileId("aim_apistol");
	AimbotGroup.PlaceLabledControl("自动手枪", this, &AimbotAutoPistol);

#pragma endregion Aimbot shit

#pragma region Triggerbot
	TriggerGroup.SetPosition(272, 48);
	TriggerGroup.SetText("扳机");
	TriggerGroup.SetSize(240, 210);
	RegisterControl(&TriggerGroup);

	TriggerEnable.SetFileId("trig_enable");
	TriggerGroup.PlaceLabledControl("开启", this, &TriggerEnable);

	TriggerKeyPress.SetFileId("trig_onkey");
	TriggerGroup.PlaceLabledControl("按键扳机", this, &TriggerKeyPress);

	TriggerKeyBind.SetFileId("trig_key");
	TriggerGroup.PlaceLabledControl("扳机按键", this, &TriggerKeyBind);
#pragma endregion Triggerbot stuff

#pragma region Main Weapon
	WeaponMainGroup.SetPosition(16, 274);
	WeaponMainGroup.SetText("步枪/其他");
	WeaponMainGroup.SetSize(240, 210);
	RegisterControl(&WeaponMainGroup);

	WeaponMainSpeed.SetFileId("main_speed");
	WeaponMainSpeed.SetBoundaries(0.1f, 2.f); 
	WeaponMainSpeed.SetValue(1.0f);
	WeaponMainGroup.PlaceLabledControl("自瞄速度", this, &WeaponMainSpeed);

	WeaponMainFoV.SetFileId("main_fov");
	WeaponMainFoV.SetBoundaries(0.1f, 30.f);
	WeaponMainFoV.SetValue(5.f);
	WeaponMainGroup.PlaceLabledControl("自瞄范围", this, &WeaponMainFoV);

	WeaponMainRecoil.SetFileId("main_recoil");
	WeaponMainGroup.PlaceLabledControl("后坐力", this, &WeaponMainRecoil);

	WeaponMainHitbox.SetFileId("main_hitbox");
	WeaponMainHitbox.AddItem("头");
	WeaponMainHitbox.AddItem("脖子");
	WeaponMainHitbox.AddItem("胸部");
	WeaponMainHitbox.AddItem("腹部");
	WeaponMainGroup.PlaceLabledControl("自瞄部位", this, &WeaponMainHitbox);
#pragma endregion

#pragma region Pistols
	WeaponPistGroup.SetPosition(272, 274);
	WeaponPistGroup.SetText("手枪");
	WeaponPistGroup.SetSize(240, 210);
	RegisterControl(&WeaponPistGroup);

	WeaponPistSpeed.SetFileId("pist_speed");
	WeaponPistSpeed.SetBoundaries(0.1f, 2.f);
	WeaponPistSpeed.SetValue(1.0f);
	WeaponPistGroup.PlaceLabledControl("自瞄速度", this, &WeaponPistSpeed);

	WeaponPistFoV.SetFileId("pist_fov");
	WeaponPistFoV.SetBoundaries(0.1f, 30.f);
	WeaponPistFoV.SetValue(5.f);
	WeaponPistGroup.PlaceLabledControl("自瞄范围", this, &WeaponPistFoV);

	WeaponPistRecoil.SetFileId("pist_recoil");
	WeaponPistGroup.PlaceLabledControl("后坐力", this, &WeaponPistRecoil);

	WeaponPistHitbox.SetFileId("pist_hitbox");
	WeaponPistHitbox.AddItem("头部");
	WeaponPistHitbox.AddItem("脖子");
	WeaponPistHitbox.AddItem("胸部");
	WeaponPistHitbox.AddItem("胃口");
	WeaponPistGroup.PlaceLabledControl("自瞄部位", this, &WeaponPistHitbox);
#pragma endregion

#pragma region Snipers
	WeaponSnipGroup.SetPosition(528, 274);
	WeaponSnipGroup.SetText("狙击枪");
	WeaponSnipGroup.SetSize(240, 210);
	RegisterControl(&WeaponSnipGroup);

	WeaponSnipSpeed.SetFileId("snip_speed");
	WeaponSnipSpeed.SetBoundaries(0.1f, 2.f);
	WeaponSnipSpeed.SetValue(1.0f);
	WeaponSnipGroup.PlaceLabledControl("自瞄速度", this, &WeaponSnipSpeed);

	WeaponSnipFoV.SetFileId("snip_fov");
	WeaponSnipFoV.SetBoundaries(0.1f, 30.f);
	WeaponSnipFoV.SetValue(5.f);
	WeaponSnipGroup.PlaceLabledControl("自瞄范围", this, &WeaponSnipFoV);

	WeaponSnipRecoil.SetFileId("snip_recoil");
	WeaponSnipGroup.PlaceLabledControl("后坐力", this, &WeaponSnipRecoil);

	WeaponSnipHitbox.SetFileId("snip_hitbox");
	WeaponSnipHitbox.AddItem("头部");
	WeaponSnipHitbox.AddItem("脖子");
	WeaponSnipHitbox.AddItem("胸部");
	WeaponSnipHitbox.AddItem("腹部");
	WeaponSnipGroup.PlaceLabledControl("自瞄部位", this, &WeaponSnipHitbox);
#pragma endregion
}

void CVisualTab::Setup()
{
	SetTitle("透视");

	ActiveLabel.SetPosition(16, 16);
	ActiveLabel.SetText("开启");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetPosition(66, 16);
	RegisterControl(&Active);

#pragma region Options
	OptionsGroup.SetText("显示选项");
	OptionsGroup.SetPosition(16, 48);
	OptionsGroup.SetSize(193, 430);
	RegisterControl(&OptionsGroup);

	OptionsBox.SetFileId("opt_box");
	OptionsGroup.PlaceLabledControl("方框", this, &OptionsBox);

	OptionsName.SetFileId("opt_name");
	OptionsGroup.PlaceLabledControl("名字", this, &OptionsName);

	OptionsHealth.SetFileId("opt_hp");
	OptionsGroup.PlaceLabledControl("血量", this, &OptionsHealth);

	OptionsWeapon.SetFileId("opt_weapon");
	OptionsGroup.PlaceLabledControl("武器", this, &OptionsWeapon);

	OptionsInfo.SetFileId("opt_info");
	OptionsGroup.PlaceLabledControl("信息", this, &OptionsInfo);

	//Broken
	//OptionsChams.SetFileId("opt_chams");
	//OptionsChams.AddItem("关闭彩人");
	//OptionsChams.AddItem("上色");
	//OptionsChams.AddItem("平坦化");
	//OptionsGroup.PlaceLabledControl("彩人", this, &OptionsChams);

	OptionsSkeleton.SetFileId("opt_bone");
	OptionsGroup.PlaceLabledControl("骨架", this, &OptionsSkeleton);

	OptionsAimSpot.SetFileId("opt_aimspot");
	OptionsGroup.PlaceLabledControl("头点", this, &OptionsAimSpot);
	
	OptionsCompRank.SetFileId("opt_comprank");
	OptionsGroup.PlaceLabledControl("段位", this, &OptionsCompRank);

#pragma endregion Setting up the Options controls

#pragma region Filters
	FiltersGroup.SetText("人物选项");
	FiltersGroup.SetPosition(225, 48);
	FiltersGroup.SetSize(193, 430);
	RegisterControl(&FiltersGroup);

	FiltersAll.SetFileId("ftr_all");
	FiltersGroup.PlaceLabledControl("全选", this, &FiltersAll);

	FiltersPlayers.SetFileId("ftr_players");
	FiltersGroup.PlaceLabledControl("玩家", this, &FiltersPlayers);

	FiltersEnemiesOnly.SetFileId("ftr_enemyonly");
	FiltersGroup.PlaceLabledControl("敌人", this, &FiltersEnemiesOnly);

	FiltersWeapons.SetFileId("ftr_weaps");
	FiltersGroup.PlaceLabledControl("携带武器", this, &FiltersWeapons);

	FiltersChickens.SetFileId("ftr_chickens");
	FiltersGroup.PlaceLabledControl("鸡", this, &FiltersChickens);

	//Broken
	//FiltersC4.SetFileId("ftr_c4");
	//FiltersGroup.PlaceLabledControl("C4", this, &FiltersC4);
#pragma endregion Setting up the Filters controls

#pragma region Other
	OtherGroup.SetText("其他");
	OtherGroup.SetPosition(434, 48);
	OtherGroup.SetSize(334, 430);
	RegisterControl(&OtherGroup);

	OtherCrosshair.SetFileId("otr_xhair");
	OtherGroup.PlaceLabledControl("准星", this, &OtherCrosshair);

	OtherRecoilCrosshair.SetFileId("otr_recoilhair");
	OtherRecoilCrosshair.AddItem("关闭");
	OtherRecoilCrosshair.AddItem("后坐力位置");
	OtherRecoilCrosshair.AddItem("后坐力半径");
	OtherGroup.PlaceLabledControl("后坐力准星", this, &OtherRecoilCrosshair);

	OtherNoVisualRecoil.SetFileId("otr_visrecoil");
	OtherGroup.PlaceLabledControl("取消后坐力动画", this, &OtherNoVisualRecoil);

	OtherNoHands.SetFileId("otr_hands");
	OtherNoHands.AddItem("关闭");
	OtherNoHands.AddItem("隐藏");
	OtherNoHands.AddItem("透明");
	OtherNoHands.AddItem("彩人");
	OtherNoHands.AddItem("迪斯科");
	OtherGroup.PlaceLabledControl("手", this, &OtherNoHands);

#pragma endregion Setting up the Other controls
}
void CColorsTab::Setup()
{
	SetTitle("颜色");
	RGBGroup.SetText("界面颜色");
	RGBGroup.SetPosition(16, 48);
	RGBGroup.SetSize(334, 280);
	RegisterControl(&RGBGroup);
#pragma region Colors
	MenuR.SetFileId("clr_menuR");
	MenuR.SetBoundaries(0.f, 255.f);
	MenuR.SetValue(150.0f);
	RGBGroup.PlaceLabledControl("红", this, &MenuR);

	MenuG.SetFileId("clr_menuG");
	MenuG.SetBoundaries(0.f, 255.f);
	MenuG.SetValue(1.0f);
	RGBGroup.PlaceLabledControl("绿", this, &MenuG);

	MenuB.SetFileId("clr_menuB");
	MenuB.SetBoundaries(0.f, 255.f);
	MenuB.SetValue(1.0f);
	RGBGroup.PlaceLabledControl("蓝", this, &MenuB);

}
void CMiscTab::Setup()
{
	SetTitle("杂项");

//Broken
//#pragma region Knife
//	KnifeGroup.SetPosition(408, 48);
//	KnifeGroup.SetSize(360, 95);
//	KnifeGroup.SetText("刀 皮肤");
//	RegisterControl(&KnifeGroup);
//
//	KnifeEnable.SetFileId("knife_enable");
//	KnifeGroup.PlaceLabledControl("开启", this, &KnifeEnable);
//	//bayonet karambit flip gut m9 hunts butterfly
//	KnifeModel.SetFileId("knife_model");
//	KnifeModel.AddItem("关闭");
//	KnifeModel.AddItem("刺刀");
//	KnifeModel.AddItem("爪子刀");
//	KnifeModel.AddItem("弯刀");
//	KnifeModel.AddItem("杀猪刀");
//	KnifeModel.AddItem("M9 刺刀");
//	KnifeModel.AddItem("屠夫");
//	KnifeModel.AddItem("蝴蝶刀");
//	KnifeModel.AddItem("暗影双匕");
//	KnifeModel.AddItem("弯刀2");
//	KnifeModel.AddItem("鲍勃猎刀");
//	KnifeGroup.PlaceLabledControl("刀", this, &KnifeModel);
//
//	//KnifeSkin.SetFileId("knife_skin");
//	//KnifeSkin.AddItem("Doppler Sapphire");
//	//KnifeSkin.AddItem("Doppler Ruby");
//	//KnifeSkin.AddItem("Tiger");
//	//KnifeSkin.AddItem("Plain");
//	//KnifeGroup.PlaceLabledControl("Skin", this, &KnifeSkin);
//
//	KnifeApply.SetText("接受");
//	KnifeApply.SetCallback(KnifeApplyCallbk);
//	KnifeGroup.PlaceLabledControl("", this, &KnifeApply);
//
//#pragma endregion

#pragma region Other
	OtherGroup.SetPosition(16, 48);
	OtherGroup.SetSize(376, 280);
	OtherGroup.SetText("其他");
	RegisterControl(&OtherGroup);

	OtherAutoJump.SetFileId("otr_autojump");
	OtherGroup.PlaceLabledControl("连跳", this, &OtherAutoJump);

	OtherAutoStrafe.SetFileId("otr_autostrafe");
	OtherGroup.PlaceLabledControl("自动扫射", this, &OtherAutoStrafe);

	OtherSafeMode.SetFileId("otr_safemode");
	OtherSafeMode.SetState(true);
	OtherGroup.PlaceLabledControl("安全模式", this, &OtherSafeMode);

	OtherChatSpam.SetFileId("otr_spam");
	OtherChatSpam.AddItem("刷屏");
	OtherChatSpam.AddItem("回合");
	OtherChatSpam.AddItem("规律");
	OtherChatSpam.AddItem("举报");
	OtherGroup.PlaceLabledControl("刷屏", this, &OtherChatSpam);

	OtherAirStuck.SetFileId("otr_astuck");
	OtherGroup.PlaceLabledControl("卡空", this, &OtherAirStuck);

	OtherSpectators.SetFileId("otr_speclist");
	OtherGroup.PlaceLabledControl("观察者列表", this, &OtherSpectators);

	OtherServerLag.SetFileId("otr_serverlag");
	OtherGroup.PlaceLabledControl("服务器延迟", this, &OtherServerLag);
	
	OtherFakeLag.SetFileId("otr_fakelag");
	OtherGroup.PlaceLabledControl("假ping", this, &OtherFakeLag);

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


