#pragma once
#include "Configuration.hpp"
#include "ItemDefinitions.hpp"
#include "SDK.hpp"
#include "PaintKitParser.hpp"
#include <imgui.h>

inline void DrawGUI()
{
	ImGui::SetNextWindowSize(ImVec2(450, 320));
	if (ImGui::Begin("GWSkinC \xe6\x9e\x84\xe5\xbb\xba\xe6\x97\xa5\xe6\x9c\x9f " __DATE__, nullptr,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_AlwaysAutoResize))
	{

		ImGui::Columns(2, nullptr, false);

		auto& vItems = Config::Get()->GetItems();
		std::vector<char*> vpszItemNames;

		if (vItems.size() == 0)
			vItems.push_back(EconomyItem_t());

		for (auto& x : vItems)
			vpszItemNames.push_back(x.szName);

		static auto iSelected = 0;

		// If the user deleted the last element or loaded a config
		if (static_cast<size_t>(iSelected) >= vpszItemNames.size())
			iSelected = 0;

		ImGui::PushItemWidth(-1);
		ImGui::ListBox("", &iSelected, const_cast<const char**>(vpszItemNames.data()), vpszItemNames.size(), 13);
		ImGui::PopItemWidth();

		ImGui::NextColumn();

		auto& SelectedItem = vItems[iSelected];

		// Name
		ImGui::InputText("\xe5\x90\x8d\xe5\xad\x97", SelectedItem.szName, 32);

		// Item to change skins for
		ImGui::Combo("\xe7\x89\xa9\xe5\x93\x81", &SelectedItem.iDefinitionId, [](void* data, int idx, const char** out_text)
		{
			*out_text = k_WeaponNames[idx].szName;
			return true;
		}, nullptr, k_WeaponNames.size(), 5);

		// Enabled
		ImGui::Checkbox("\xe5\x90\xaf\xe7\x94\xa8", &SelectedItem.bEnabled);

		// Paint kit ID
		//ImGui::InputInt("Paint Kit", &SelectedItem.iPaintKitIndex);

		// Pattern Seed
		ImGui::InputInt("\xe7\xa7\x8d\xe5\xad\x90", &SelectedItem.iSeed);

		// Custom StatTrak number
		ImGui::InputInt("StatTrak", &SelectedItem.iStatTrak);

		// Wear Float
		ImGui::SliderFloat("\xe7\xa3\xa8\xe6\x8d\x9f", &SelectedItem.flWear, FLT_MIN, 1.f, "%.10f", 5);

		// Paint kit
		ImGui::Combo("\xe9\x80\x89\xe6\x8b\xa9\xe7\x9a\xae\xe8\x82\xa4", &SelectedItem.iPaintKitId, [](void* data, int idx, const char** out_text)
		{
			*out_text = k_Skins[idx].name.c_str();
			return true;
		}, nullptr, k_Skins.size(), 10);

		// Quality
		ImGui::Combo("\xe5\x93\x81\xe8\xb4\xa8", &SelectedItem.iEntityQualityId, [](void* data, int idx, const char** out_text)
		{
			*out_text = k_QualityNames[idx].szName;
			return true;
		}, nullptr, k_QualityNames.size(), 5);

		// Yes we do it twice to decide knifes
		SelectedItem.UpdateValues();

		// Item defindex override
		if (IsKnife(SelectedItem.iDefinitionIndex))
		{
			ImGui::Combo("\xe5\x88\x80", &SelectedItem.iDefinitionOverrideId, [](void* data, int idx, const char** out_text)
			{
				*out_text = k_KniveNames[idx].szName;
				return true;
			}, nullptr, k_KniveNames.size(), 5);
		}
		else
		{
			static auto iThrowaway = 0;
			SelectedItem.iDefinitionOverrideId = 0;
			ImGui::Combo("\xe4\xb8\x8d\xe5\x8f\xaf\xe7\x94\xa8", &iThrowaway, "\xe5\x8f\xaa\xe5\xaf\xb9\xe5\x88\x80\xe6\x9c\x89\xe6\x95\x88\0");
		}

		SelectedItem.UpdateValues();

		// Custom Name tag
		ImGui::InputText("\xe5\x91\xbd\xe5\x90\x8d", SelectedItem.szCustomName, 32);

		ImGui::NextColumn();

		ImGui::Separator();
		ImGui::Columns(5, nullptr, false);
		ImGui::PushItemWidth(-1);

		auto vSize = ImVec2(ImGui::GetColumnWidth(), 20);

		if (ImGui::Button("\xe6\xb7\xbb\xe5\x8a\xa0", vSize))
		{
			vItems.push_back(EconomyItem_t());
			iSelected = vItems.size() - 1;
		}
		ImGui::NextColumn();

		if (ImGui::Button("\xe5\x88\xa0\xe9\x99\xa4", vSize))
			vItems.erase(vItems.begin() + iSelected);
		ImGui::NextColumn();

		if (ImGui::Button("\xe5\x88\xb7\xe6\x96\xb0", vSize))
			(*g_ppClientState)->ForceFullUpdate();
		ImGui::NextColumn();

		if (ImGui::Button("\xe4\xbf\x9d\xe5\xad\x98", vSize))
			Config::Get()->Save();
		ImGui::NextColumn();

		if (ImGui::Button("\xe8\xbd\xbd\xe5\x85\xa5", vSize))
			Config::Get()->Load();
		ImGui::NextColumn();

		ImGui::PopItemWidth();
		ImGui::Columns(1);

		ImGui::Text("GWSkinC \xe7\x94\xb1 NErD Hacks \xe5\x91\x88\xe7\x8e\xb0");
		// We can put our website here - what we don't have right now.
		ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize("").x - 17);
		ImGui::Text("");

		ImGui::End();
	}
}
