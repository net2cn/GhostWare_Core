#include "Configuration.hpp"
#include "ItemDefinitions.hpp"
#include "SDK.hpp"
#include "KitParser.hpp"
#include "UpdateCheck.hpp"

#include <imgui.h>
#include <functional>

namespace ImGui
{
	// ImGui ListBox lambda binder
	static bool ListBox(const char* label, int* current_item,  std::function<const char*(int)> lambda, int items_count, int height_in_items)
	{
		return ImGui::ListBox(label, current_item, [](void* data, int idx, const char** out_text)
		{
			*out_text = (*reinterpret_cast<std::function<const char*(int)>*>(data))(idx);
			return true;
		}, &lambda, items_count, height_in_items);
	}
}

void DrawGUI()
{
	ImGui::SetNextWindowSize(ImVec2(700, 400));
	if(ImGui::Begin("GWSkinC \xe6\x9e\x84\xe5\xbb\xba\xe6\x97\xa5\xe6\x9c\x9f " __DATE__, nullptr,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings))
	{

		auto& entries = g_config.GetItems();

		// If the user deleted the only config let's add one
		if(entries.size() == 0)
			entries.push_back(EconomyItem_t());

		static auto selected_id = 0;

		ImGui::Columns(2, nullptr, false);

		// Config selection
		{
			ImGui::PushItemWidth(-1);

			char element_name[64];

			ImGui::ListBox("##config", &selected_id, [&element_name, &entries](int idx)
			{
				sprintf_s(element_name, "%s (%s)", entries.at(idx).name, k_weapon_names.at(entries.at(idx).definition_vector_index).name);
				return element_name;
			}, entries.size(), 11);

			auto button_size = ImVec2(ImGui::GetColumnWidth() / 2 - 8.5f, 31);

			if(ImGui::Button("\xe6\xb7\xbb\xe5\x8a\xa0", button_size))
			{
				entries.push_back(EconomyItem_t());
				selected_id = entries.size() - 1;
			}
			ImGui::SameLine();

			if(ImGui::Button("\xe7\xa7\xbb\xe9\x99\xa4", button_size))
				entries.erase(entries.begin() + selected_id);

			ImGui::PopItemWidth();
		}

		ImGui::NextColumn();

		selected_id = selected_id < entries.size() ? selected_id : entries.size() - 1;

		auto& selected_entry = entries[selected_id];

		{
			// Name
			ImGui::InputText("\xe5\x90\x8d\xe5\xad\x97", selected_entry.name, 32);

			// Item to change skins for
			ImGui::Combo("\xe7\x89\xa9\xe5\x93\x81", &selected_entry.definition_vector_index, [](void* data, int idx, const char** out_text)
			{
				*out_text = k_weapon_names[idx].name;
				return true;
			}, nullptr, k_weapon_names.size(), 5);

			// Enabled
			ImGui::Checkbox("\xe5\x90\xaf\xe7\x94\xa8", &selected_entry.enabled);

			// Pattern Seed
			ImGui::InputInt("\xe7\xa7\x8d\xe5\xad\x90", &selected_entry.seed);

			// Custom StatTrak number
			ImGui::InputInt("StatTrak", &selected_entry.stat_trak);

			// Wear Float
			ImGui::SliderFloat("\xe7\xa3\xa8\xe6\x8d\x9f", &selected_entry.wear, FLT_MIN, 1.f, "%.10f", 5);

			// Paint kit
			if(selected_entry.definition_index != GLOVE_T_SIDE)
			{
				ImGui::Combo("\xe9\x80\x89\xe6\x8b\xa9\xe7\x9a\xae\xe8\x82\xa4", &selected_entry.paint_kit_vector_index, [](void* data, int idx, const char** out_text)
				{
					*out_text = k_skins[idx].name.c_str();
					return true;
				}, nullptr, k_skins.size(), 10);
			}
			else
			{
				ImGui::Combo("\xe9\x80\x89\xe6\x8b\xa9\xe7\x9a\xae\xe8\x82\xa4", &selected_entry.paint_kit_vector_index, [](void* data, int idx, const char** out_text)
				{
					*out_text = k_gloves[idx].name.c_str();
					return true;
				}, nullptr, k_gloves.size(), 10);
			}

			// Quality
			ImGui::Combo("\xe5\x93\x81\xe8\xb4\xa8", &selected_entry.entity_quality_vector_index, [](void* data, int idx, const char** out_text)
			{
				*out_text = k_quality_names[idx].name;
				return true;
			}, nullptr, k_quality_names.size(), 5);

			// Yes we do it twice to decide knifes
			selected_entry.UpdateValues();

			// Item defindex override
			if(selected_entry.definition_index == WEAPON_KNIFE)
			{
				ImGui::Combo("\xe5\x88\x80", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
				{
					*out_text = k_knife_names.at(idx).name;
					return true;
				}, nullptr, k_knife_names.size(), 5);
			}
			else if(selected_entry.definition_index == GLOVE_T_SIDE)
			{
				ImGui::Combo("\xe6\x89\x8b\xe5\xa5\x97", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
				{
					*out_text = k_glove_names.at(idx).name;
					return true;
				}, nullptr, k_glove_names.size(), 5);
			}
			else
			{
				// We don't want to override weapons other than knives or gloves
				static auto unused_value = 0;
				selected_entry.definition_override_vector_index = 0;
				ImGui::Combo("\xe4\xb8\x8d\xe9\x80\x82\xe7\x94\xa8", &unused_value, "\xe5\x88\x80\x2f\xe6\x89\x8b\xe5\xa5\x97\xe5\x8f\xaf\xe7\x94\xa8\0");
			}

			selected_entry.UpdateValues();

			// Custom Name tag
			ImGui::InputText("\xe5\x91\xbd\xe5\x90\x8d", selected_entry.custom_name, 32);
		}

		ImGui::NextColumn();

		ImGui::Columns(1, nullptr, false);

		ImGui::Separator();

		{
			ImGui::Columns(2, nullptr, false);

			ImGui::PushID("sticker");

			static auto selected_sticker_slot = 0;

			auto& selected_sticker = selected_entry.stickers[selected_sticker_slot];

			ImGui::PushItemWidth(-1);

			char element_name[64];

			ImGui::ListBox("", &selected_sticker_slot, [&selected_entry, &element_name](int idx)
			{
				auto kit_vector_index = selected_entry.stickers[idx].kit_vector_index;
				sprintf_s(element_name, "#%d (%s)", idx + 1, k_stickers.at(kit_vector_index).name.c_str());
				return element_name;
			}, 5, 5);
			ImGui::PopItemWidth();

			ImGui::NextColumn();

			ImGui::Combo("\xe8\xb4\xb4\xe7\xba\xb8\xe5\xb7\xa5\xe5\x85\xb7", &selected_sticker.kit_vector_index, [](void* data, int idx, const char** out_text)
			{
				*out_text = k_stickers.at(idx).name.c_str();
				return true;
			}, nullptr, k_stickers.size(), 10);

			ImGui::SliderFloat("\xe7\xa3\xa8\xe6\x8d\x9f", &selected_sticker.wear, FLT_MIN, 1.f, "%.10f", 5);

			ImGui::SliderFloat("\xe7\xbc\xa9\xe6\x94\xbe", &selected_sticker.scale, 0.1f, 5.f, "%.3f");

			ImGui::SliderFloat("\xe6\x97\x8b\xe8\xbd\xac", &selected_sticker.rotation, 0.f, 360.f, "%.3f");

			ImGui::NextColumn();

			ImGui::PopID();
		}

		ImGui::Columns(1, nullptr, false);

		ImGui::Separator();

		ImGui::Columns(3, nullptr, false);

		ImGui::PushItemWidth(-1);

		// Lower buttons for modifying items and saving
		{
			auto button_size = ImVec2(ImGui::GetColumnWidth() - 1, 20);

			if(ImGui::Button("\xe5\x88\xb7\xe6\x96\xb0", button_size))
				(*g_client_state)->ForceFullUpdate();
			ImGui::NextColumn();

			if(ImGui::Button("\xe4\xbf\x9d\xe5\xad\x98", button_size))
				g_config.Save();
			ImGui::NextColumn();

			if(ImGui::Button("\xe8\xbd\xbd\xe5\x85\xa5", button_size))
				g_config.Load();
			ImGui::NextColumn();
		}

		ImGui::PopItemWidth();
		ImGui::Columns(1);

		ImGui::Text("GWSkinC \xe7\x94\xb1 NErD Hacks \xe5\x91\x88\xe7\x8e\xb0");
		// We can put our website here.
		//ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize("https://skinchanger.download").x - 17);
		//ImGui::Text("https://skinchanger.download");

		ImGui::End();
	}

	// Auto Update - We don't need this right now.
	//if(g_update_needed && ImGui::Begin("New commits since compile!", &g_update_needed,
	//	{600, 400}, -1, ImGuiWindowFlags_NoSavedSettings))
	//{
	//	ImGui::Columns(3, "commit", true);
	//	for(auto& commit : g_commits_since_compile)
	//	{
	//		ImGui::Text("%s", commit.author.c_str());
	//		ImGui::NextColumn();
	//		ImGui::Text("%s", commit.date.c_str());
	//		ImGui::NextColumn();
	//		ImGui::Text("%s", commit.message.c_str());
	//		ImGui::NextColumn();
	//	}

	//	ImGui::End();
	//}
}
