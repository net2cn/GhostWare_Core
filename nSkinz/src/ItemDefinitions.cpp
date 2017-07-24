#include "ItemDefinitions.hpp"

// We need these for overriding viewmodels and icons
const std::map<size_t, Item_t> k_weapon_info =
{
	{WEAPON_KNIFE,{"models/weapons/v_knife_default_ct.mdl", "knife_default_ct"}},
	{WEAPON_KNIFE_T,{"models/weapons/v_knife_default_t.mdl", "knife_t"}},
	{WEAPON_KNIFE_BAYONET, {"models/weapons/v_knife_bayonet.mdl", "bayonet"}},
	{WEAPON_KNIFE_FLIP, {"models/weapons/v_knife_flip.mdl", "knife_flip"}},
	{WEAPON_KNIFE_GUT, {"models/weapons/v_knife_gut.mdl", "knife_gut"}},
	{WEAPON_KNIFE_KARAMBIT, {"models/weapons/v_knife_karam.mdl", "knife_karambit"}},
	{WEAPON_KNIFE_M9_BAYONET, {"models/weapons/v_knife_m9_bay.mdl", "knife_m9_bayonet"}},
	{WEAPON_KNIFE_TACTICAL, {"models/weapons/v_knife_tactical.mdl", "knife_tactical"}},
	{WEAPON_KNIFE_FALCHION, {"models/weapons/v_knife_falchion_advanced.mdl", "knife_falchion"}},
	{WEAPON_KNIFE_SURVIVAL_BOWIE, {"models/weapons/v_knife_survival_bowie.mdl", "knife_survival_bowie"}},
	{WEAPON_KNIFE_BUTTERFLY, {"models/weapons/v_knife_butterfly.mdl", "knife_butterfly"}},
	{WEAPON_KNIFE_PUSH, {"models/weapons/v_knife_push.mdl", "knife_push"}},
	{GLOVE_STUDDED_BLOODHOUND,{"models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl"}},
	{GLOVE_T_SIDE,{"models/weapons/v_models/arms/glove_fingerless/v_glove_fingerless.mdl"}},
	{GLOVE_CT_SIDE,{"models/weapons/v_models/arms/glove_hardknuckle/v_glove_hardknuckle.mdl"}},
	{GLOVE_SPORTY,{"models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl"}},
	{GLOVE_SLICK,{"models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl"}},
	{GLOVE_LEATHER_WRAP,{"models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl"}},
	{GLOVE_MOTORCYCLE,{"models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl"}},
	{GLOVE_SPECIALIST,{"models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl"}}
};

const std::vector<WeaponName_t> k_knife_names =
{
	{0, "\xe9\xbb\x98\xe8\xae\xa4" },
	{WEAPON_KNIFE_BAYONET, "\xe5\x88\xba\xe5\x88\x80" },
	{WEAPON_KNIFE_FLIP, "\xe6\x8a\x98\xe5\x8f\xa0\xe5\x88\x80" },
	{WEAPON_KNIFE_GUT, "\xe6\x9d\x80\xe7\x8c\xaa\xe5\x88\x80" },
	{WEAPON_KNIFE_KARAMBIT, "\xe7\x88\xaa\xe5\xad\x90\xe5\x88\x80" },
	{WEAPON_KNIFE_M9_BAYONET, "M9\xe5\x88\xba\xe5\x88\x80" },
	{WEAPON_KNIFE_TACTICAL, "\xe7\x8c\x8e\xe6\x9d\x80\xe8\x80\x85\xe5\x8c\x95\xe9\xa6\x96" },
	{WEAPON_KNIFE_FALCHION, "\xe5\xbc\xaf\xe5\x88\x80" },
	{WEAPON_KNIFE_SURVIVAL_BOWIE, "\xe9\xb2\x8d\xe4\xbc\x8a\xe7\x8c\x8e\xe5\x88\x80" },
	{WEAPON_KNIFE_BUTTERFLY, "\xe8\x9d\xb4\xe8\x9d\xb6\xe5\x88\x80" },
	{WEAPON_KNIFE_PUSH, "\xe6\x9a\x97\xe5\xbd\xb1\xe5\x8f\x8c\xe5\x8c\x95" }
};

const std::vector<WeaponName_t> k_glove_names =
{
	{0, "Default"},
	{GLOVE_STUDDED_BLOODHOUND, "Bloodhound"},
	{GLOVE_T_SIDE, "Default (Terrorists)"},
	{GLOVE_CT_SIDE, "Default (Counter-Terrorists)"},
	{GLOVE_SPORTY, "Sporty"},
	{GLOVE_SLICK, "Slick"},
	{GLOVE_LEATHER_WRAP, "Handwrap"},
	{GLOVE_MOTORCYCLE, "Motorcycle"},
	{GLOVE_SPECIALIST, "Specialist"}
};

const std::vector<WeaponName_t> k_weapon_names =
{
	{WEAPON_KNIFE, "Knife"},
	{GLOVE_T_SIDE, "Glove"},
	{7, "AK-47"},
	{8, "AUG"},
	{9, "AWP"},
	{63, "CZ75 Auto"},
	{1, "Desert Eagle"},
	{2, "Dual Berettas"},
	{10, "FAMAS"},
	{3, "Five-SeveN"},
	{11, "G3SG1"},
	{13, "Galil AR"},
	{4, "Glock-18"},
	{14, "M249"},
	{60, "M4A1-S"},
	{16, "M4A4"},
	{17, "MAC-10"},
	{27, "MAG-7"},
	{33, "MP7"},
	{34, "MP9"},
	{28, "Negev"},
	{35, "Nova"},
	{32, "P2000"},
	{36, "P250"},
	{19, "P90"},
	{26, "PP-Bizon"},
	{64, "R8 Revolver"},
	{29, "Sawed-Off"},
	{38, "SCAR-20"},
	{40, "SSG 08"},
	{39, "SG 553"},
	{30, "Tec-9"},
	{24, "UMP-45"},
	{61, "USP-S"},
	{25, "XM1014"},
};

const std::vector<QualityName_t> k_quality_names =
{
	{ 0, "\xe9\xbb\x98\xe8\xae\xa4" },
	{ 1, "\xe7\xba\xaf\xe6\xad\xa3" },
	{ 2, "\xe6\x9c\x80\xe4\xbd\xb3\xe9\x80\x89\xe6\x8b\xa9" },
	{ 3, "\xe5\xbc\x82\xe5\xb8\xb8" },
	{ 5, "\xe7\xa4\xbe\xe5\x8c\xba" },
	{ 6, "\xe5\xbc\x80\xe5\x8f\x91\xe8\x80\x85" },
	{ 7, "\xe8\x87\xaa\xe5\x88\xb6" },
	{ 8, "\xe5\xae\x9a\xe5\x88\xb6" },
	{ 9, "\xe4\xb8\x8d\xe8\xa7\x84\xe5\x88\x99" },
	{ 10, "\xe5\xae\x8c\xe6\x95\xb4\xe7\x89\x88" },
	{ 12, "\xe7\xab\x9e\xe6\x8a\x80" }
};
