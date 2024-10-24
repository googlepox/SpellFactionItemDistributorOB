#include "Manager.h"
#include "fstream"
#include "lib/boost/trim.hpp"

extern OBSEScriptInterface* g_script;

namespace SpellFactionItemDistributor
{
	FormCode GetFormCodeFromString(std::string formString) {
		if (formString == "Forms") return form;
		if (formString == "Spells") return spell;
		if (formString == "Factions") return faction;
		if (formString == "Equipment") return equippable;
		if (formString == "Packages") return package;
		if (formString == "Items") return item;
	}

	FormMap<SwapDataVec>& Manager::get_form_vec(const std::string& a_str)
	{
		switch (GetFormCodeFromString(a_str))
		{
		case (item): {
			return allItems;
			break;
		}
		case (equippable): {
			return allEquipment;
			break;
		}
		case (spell): {
			return allSpells;
			break;
		}
		case (faction): {
			return allFactions;
			break;
		}
		case (package): {
			return allPackages;
			break;
		}
		default:
			break;
		}
	}

	FormMap<SwapDataConditional>& Manager::get_form_map(const std::string& a_str)
	{
		switch (GetFormCodeFromString(a_str))
		{
		case (item): {
			return allItemsConditional;
			break;
		}
		case (equippable): {
			return allEquipmentConditional;
			break;
		}
		case (spell): {
			return allSpellsConditional;
			break;
		}
		case (faction): {
			return allFactionsConditional;
			break;
		}
		case (package): {
			return allPackagesConditional;
			break;
		}
		default:
			break;
		}
	}

	FormMap<SwapDataConditional>& Manager::get_form_map_all(const std::string& a_str)
	{
		switch (GetFormCodeFromString(a_str))
		{
		case (item): {
			return applyToAllItems;
			break;
		}
		case (equippable): {
			return applyToAllEquipment;
			break;
		}
		case (spell): {
			return applyToAllSpells;
			break;
		}
		case (faction): {
			return applyToAllFactions;
			break;
		}
		case (package): {
			return applyToAllPackages;
			break;
		}
		default:
			break;
		}
	}

	void Manager::get_forms(const std::string& a_path, const std::string& a_str, FormMap<SwapDataVec>& a_map, std::string formType)
	{
		return DistributeRecordData::GetForms(a_path, a_str, [&](UInt32 a_baseID, const DistributeRecordData& a_SwapData) {
			a_map[a_baseID].push_back(a_SwapData);
			});
	}

	void Manager::get_forms(const std::string& a_path, const std::string& a_str, const std::vector<FormIDStr>& a_conditionalIDs, std::string formType)
	{
		return DistributeRecordData::GetForms(a_path, a_str, [&](const UInt32 a_baseID, const DistributeRecordData& a_SwapData) {
			for (auto& id : a_conditionalIDs) {
				get_form_map(formType)[a_baseID][id].push_back(a_SwapData);
			}
			});
	}

	void Manager::get_forms_all(const std::string& a_path, const std::string& a_str, const std::vector<FormIDStr>& a_conditionalIDs, std::string formType)
	{
		return DistributeRecordData::GetForms(a_path, a_str, [&](const UInt32 a_baseID, const DistributeRecordData& a_SwapData) {
			for (auto& id : a_conditionalIDs) {
				get_form_map_all(formType)[a_baseID][id].push_back(a_SwapData);
			}
			});
	}

	static bool HasKeywordCell(TESObjectCELL* a_cell, const FormIDStr& a_keyword, bool isExclusion)
	{
		if (a_cell) {
			std::string newKey = std::get<std::string>(a_keyword);
			UInt32 cellID = a_cell->refID;
			UInt32 newFormID = DistributeRecordData::GetFormID(newKey.c_str());
			if (newFormID) {
				if (newFormID && ((newFormID == cellID) || (std::to_string(cellID).contains(std::to_string(newFormID))))) {
					return !isExclusion;
				}
				return isExclusion;
			}
			else {
				std::string editorID = (a_cell->GetEditorName());
				std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
				std::transform(editorID.begin(), editorID.end(), editorID.begin(), tolower);
				std::string cStrKey = newKey.c_str();
				std::string cStrEditorID = editorID.c_str();
				if (cStrEditorID.find(cStrKey.c_str()) != std::string::npos) {
					return !isExclusion;
				}
				return isExclusion;
			}
		}
		else {
			return false;
		}
	}

	static bool HasKeywordWorldspace(TESObjectCELL* a_cell, const FormIDStr& a_keyword, bool isExclusion)
	{
		if (a_cell) {
			std::string newKey = std::get<std::string>(a_keyword);
			UInt32 cellID = a_cell->worldSpace->refID;
			UInt32 newFormID = DistributeRecordData::GetFormID(newKey.c_str());
			if (newFormID) {
				if (newFormID && ((newFormID == cellID) || (std::to_string(cellID).contains(std::to_string(newFormID))))) {
					return !isExclusion;
				}
				return isExclusion;
			}
			else {
				std::string editorID = (a_cell->worldSpace->GetEditorName());
				std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
				std::transform(editorID.begin(), editorID.end(), editorID.begin(), tolower);
				std::string cStrKey = newKey.c_str();
				std::string cStrEditorID = editorID.c_str();
				if (cStrEditorID.find(cStrKey.c_str()) != std::string::npos) {
					return !isExclusion;
				}
				return isExclusion;
			}
		}
		else {
			return false;
		}
	}

	static bool HasKeywordRegion(TESObjectCELL* a_cell, const FormIDStr& a_keyword, bool isExclusion)
	{
		if (a_cell) {
			ExtraRegionList* regionList = dynamic_cast<ExtraRegionList*>(a_cell->extraData.GetByType(kExtraData_RegionList));
			TESRegionList* regions = regionList->regionList;
			TESRegionList::Entry* regionPtr = &(regions->regionList);
			bool found = false;
			while (regionPtr != NULL)
			{
				UInt32 regionID = regionPtr->region->refID;
				std::string newKey = std::get<std::string>(a_keyword);
				UInt32 newFormID = DistributeRecordData::GetFormID(newKey.c_str());
				if (newFormID) {
					if (newFormID && ((newFormID == regionID) || (std::to_string(regionID).contains(std::to_string(newFormID))))) {
						found = true;
					}
				}
				else {
					std::string editorID = (regionPtr->region->GetEditorName());
					std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
					std::transform(editorID.begin(), editorID.end(), editorID.begin(), tolower);
					std::string cStrKey = newKey.c_str();
					std::string cStrEditorID = editorID.c_str();
					if (cStrEditorID.find(cStrKey.c_str()) != std::string::npos) {
						found = true;
					}
				}
				regionPtr = regionPtr->next;
			}
			if (found) {
				return !isExclusion;
			}
			return isExclusion;
		}
		else {
			return false;
		}
	}

	static bool HasKeywordEditorID(TESObjectREFR* ref, const FormIDStr& a_keyword, bool isExclusion)
	{
		if (ref) {
			std::string newKey = std::get<std::string>(a_keyword);
			UInt32 refID = ref->baseForm->refID;
			UInt32 newFormID = std::atoi(newKey.c_str());
			if (newFormID) {
				if (newFormID && ((newFormID == refID))) {
					return !isExclusion;
				}
				return isExclusion;
			}
			else {
				std::string editorID;
				if (ref->baseForm) {
					editorID = (ref->baseForm->GetEditorName());
				}
				else {
					editorID = (ref->GetEditorName());
				}
				std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
				std::transform(editorID.begin(), editorID.end(), editorID.begin(), tolower);
				std::string cStrKey = newKey.c_str();
				std::string cStrEditorID = editorID.c_str();
				if (cStrEditorID.find(cStrKey.c_str()) != std::string::npos) {
					return !isExclusion;
				}
				return isExclusion;
			}
		}
		else {
			return false;
		}
	}

	static bool HasKeywordName(TESObjectREFR* ref, const FormIDStr& a_keyword, bool isExclusion)
	{
		if (ref) {
			std::string newKey = std::get<std::string>(a_keyword);
			std::string editorID;
			if (ref->baseForm) {
				editorID = (ref->baseForm->GetFullName()->name.m_data);
			}
			else {
				editorID = (ref->GetFullName()->name.m_data);
			}
			std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
			std::transform(editorID.begin(), editorID.end(), editorID.begin(), tolower);
			std::string cStrKey = newKey.c_str();
			std::string cStrEditorID = editorID.c_str();
			if (cStrEditorID.find(cStrKey.c_str()) != std::string::npos) {
				return !isExclusion;
			}
			return isExclusion;
		}
		else {
			return false;
		}
	}

	static bool HasKeywordRace(TESObjectREFR* ref, const FormIDStr& a_keyword, bool isExclusion)
	{
		if (ref) {
			TESActorBase* actor = dynamic_cast<TESActorBase*>(ref->baseForm);
			TESNPC* npc = dynamic_cast<TESNPC*>(actor);
			std::string newKey = std::get<std::string>(a_keyword);
			std::string editorID = (npc->race.race->GetEditorName());
			UInt32 refID = npc->race.race->refID;
			UInt32 newFormID = std::atoi(newKey.c_str());
			if (newFormID) {
				if (newFormID && ((newFormID == refID) || (std::to_string(refID).contains(std::to_string(newFormID))))) {
					return !isExclusion;
				}
				return isExclusion;
			}
			else {
				std::string editorID = (ref->baseForm->GetEditorName());
				std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
				std::transform(editorID.begin(), editorID.end(), editorID.begin(), tolower);
				std::string cStrKey = newKey.c_str();
				std::string cStrEditorID = editorID.c_str();
				if (cStrEditorID.find(cStrKey.c_str()) != std::string::npos) {
					return !isExclusion;
				}
				return isExclusion;
			}
		}
		else {
			return false;
		}
	}

	static bool HasKeywordFaction(TESObjectREFR* ref, const FormIDStr& a_keyword, bool isExclusion)
	{
		if (ref) {
			bool found = false;
			TESActorBase* actor = dynamic_cast<TESActorBase*>(ref->baseForm);
			TESNPC* npc = dynamic_cast<TESNPC*>(actor);
			TESActorBaseData::FactionListEntry* entry = &npc->actorBaseData.factionList;
			std::string newKey = std::get<std::string>(a_keyword);
			while (entry && entry->data)
			{	
				TESFaction* faction = entry->data->faction;
				std::string editorID = faction->GetEditorName();
				std::string refID = std::to_string(faction->refID).c_str();
				std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
				std::transform(editorID.begin(), editorID.end(), editorID.begin(), tolower);
				std::string cStrKey = newKey.c_str();
				std::string cStrEditorID = editorID.c_str();
				if (cStrEditorID.find(cStrKey.c_str()) != std::string::npos) {
					found = true;
				}
				else if (refID.find(cStrKey.c_str()) != std::string::npos) {
					found = true;
				}
				entry = entry->Next();
			}
			if (found) {
				return !isExclusion;
			}
			return isExclusion;
		}
		else {
			return false;
		}
	}

	static bool HasKeywordClass(TESObjectREFR* ref, const FormIDStr& a_keyword, bool isExclusion)
	{
		if (ref) {
			TESActorBase* actor = dynamic_cast<TESActorBase*>(ref->baseForm);
			TESNPC* npc = dynamic_cast<TESNPC*>(actor);
			std::string newKey = std::get<std::string>(a_keyword);
			std::string editorID = (npc->npcClass->GetEditorName());
			UInt32 refID = npc->npcClass->refID;
			UInt32 newFormID = std::atoi(newKey.c_str());
			if (newFormID) {
				if (newFormID && ((newFormID == refID) || (std::to_string(refID).contains(std::to_string(newFormID))))) {
					return !isExclusion;
				}
				return isExclusion;
			}
			else {
				std::string editorID = (ref->baseForm->GetEditorName());
				std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
				std::transform(editorID.begin(), editorID.end(), editorID.begin(), tolower);
				std::string cStrKey = newKey.c_str();
				std::string cStrEditorID = editorID.c_str();
				if (cStrEditorID.find(cStrKey.c_str()) != std::string::npos) {
					return !isExclusion;
				}
				return isExclusion;
			}
		}
		else {
			return false;
		}
	}

	static bool HasKeywordItem(TESObjectREFR* ref, const FormIDStr& a_keyword, bool isExclusion)
	{
		if (ref) {
			bool found = false;
			TESActorBase* actor = dynamic_cast<TESActorBase*>(ref->baseForm);
			Character* npc = dynamic_cast<Character*>(actor);
			TESContainer* cont = ref->GetContainer();
			TESContainer::Entry* entry = &cont->list;
			while (entry && entry->data) {
				TESForm* form = entry->data->type;
				std::string editorID = form->GetEditorName();
				UInt32 refID = form->refID;
				std::string newKey = std::get<std::string>(a_keyword);
				std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
				std::transform(editorID.begin(), editorID.end(), editorID.begin(), tolower);
				std::string cStrKey = newKey.c_str();
				std::string cStrEditorID = editorID.c_str();
				if (cStrEditorID.find(cStrKey.c_str()) != std::string::npos) {
					found = true;
				}
				else if (refID == atoi(cStrKey.c_str())) {
					found = true;
				}
				entry = entry->Next();
			}
			if (found) {
				return !isExclusion;
			}
		}
		else {
			return false;
		}
	}

	static bool HasKeywordMod(TESObjectREFR* ref, const FormIDStr& a_keyword, bool isExclusion)
	{
		if (ref) {
			std::string newKey = std::get<std::string>(a_keyword);
			UInt8 modIndex = ref->baseForm->GetModIndex();
			std::string modName = (*g_dataHandler)->GetNthModName(modIndex);
			std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
			std::transform(modName.begin(), modName.end(), modName.begin(), tolower);
			std::string cStrKey = newKey.c_str();
			std::string cStrModName = modName.c_str();
			if (cStrModName.find(cStrKey.c_str()) != std::string::npos) {
				return !isExclusion;
			}
			return isExclusion;
		}
		else {
			return false;
		}
	}

	bool ConditionalInput::IsValid(const FormIDStr& a_data, TESObjectREFR* refToCheck) const
	{
		if (refToCheck) {
			FormIDStr newData = a_data;
			UInt32 formID;
			std::string formString = std::get<std::string>(a_data);
			boost::trim(formString);
			bool isExclusion = false;
			if (formString.find('-') != std::string::npos) {
				std::string::iterator end_pos = std::remove(formString.begin(), formString.end(), '-');
				formString.erase(end_pos, formString.end());
				isExclusion = true;
			}
			if (!formString.contains(':')) {
				return HasKeywordEditorID(refToCheck, formString, isExclusion);;
			}
			auto conditionType = string::split(formString, ":");
			boost::trim(conditionType[0]);
			boost::trim(conditionType[1]);
			formID = DistributeRecordData::GetFormID(conditionType[1]);
			if (formID) {
				newData = std::to_string(formID);
			}
			else {
				newData = conditionType[1];
			}
			if (conditionType[0] == "Cell") {
				return HasKeywordCell(refToCheck->parentCell, newData, isExclusion);
			}
			else if (conditionType[0] == "EditorID") {
				return HasKeywordEditorID(refToCheck, newData, isExclusion);
			}
			else if (conditionType[0] == "Race") {
				return HasKeywordRace(refToCheck, newData, isExclusion);
			}
			else if (conditionType[0] == "Class") {
				return HasKeywordClass(refToCheck, newData, isExclusion);
			}
			else if (conditionType[0] == "Faction") {
				return HasKeywordFaction(refToCheck, newData, isExclusion);
			}
			else if (conditionType[0] == "Item") {
				return HasKeywordItem(refToCheck, newData, isExclusion);
			}
			else if (conditionType[0] == "Name") {
				return HasKeywordName(refToCheck, newData, isExclusion);
			}
			else if (conditionType[0] == "Mod") {
				return HasKeywordMod(refToCheck, newData, isExclusion);
			}
			else {
				return false;
			}
		}
		return false;
	}

	bool ConditionalInput::IsValidAll(const FormIDStr& a_data, TESObjectREFR* refToCheck) const
	{
		if (refToCheck) {
			std::string conditionStr = std::get<std::string>(a_data);
			std::vector<bool> resultVec;
			if (conditionStr.contains("ALL")) return true;
			if (conditionStr.contains("&")) {
				auto conditions = string::split(conditionStr, "&");
				for (const auto& condition : conditions) {
					resultVec.push_back(IsValid(condition, refToCheck));
				}
				for (const auto& result : resultVec) {
					if (!result) {
						return false;
					}
				}
				return true;
			}
			else {
				return IsValid(conditionStr, refToCheck);
			}
			
		}
		return false;
	}

	void Manager::LoadFormsOnce()
	{
		std::call_once(init, [this] {
			LoadForms();
			});
	}

	void Manager::LoadForms()
	{
		_MESSAGE("-INI-");

		const std::filesystem::path sfidFolder{ R"(Data\SpellFactionItemDistributor)" };
		if (!exists(sfidFolder)) {
			_WARNING("SFID folder not found...");
			return;
		}

		const auto configs = dist::get_configs(R"(Data\SpellFactionItemDistributor)");

		if (configs.empty()) {
			_WARNING("No .ini files were found in Data\\SpellFactionItemDistributor folder, aborting...");
			return;
		}

		_MESSAGE("%u matching inis found", configs.size());

		for (auto& path : configs) {
			_MESSAGE("\tINI : %s", path.c_str());

			CSimpleIniA ini;
			ini.SetUnicode();
			ini.SetMultiKey();
			ini.SetAllowKeyOnly();

			if (const auto rc = ini.LoadFile(path.c_str()); rc < 0) {
				_ERROR("\tcouldn't read INI");
				continue;
			}

			CSimpleIniA::TNamesDepend sections;
			ini.GetAllSections(sections);
			sections.sort(CSimpleIniA::Entry::LoadOrder());

			constexpr auto push_filter = [](const std::string& a_condition, std::vector<FormIDStr>& a_processedFilters) {
				if (a_condition.contains('&') || a_condition.contains(':') || a_condition.contains("ALL")) {
					a_processedFilters.emplace_back(a_condition);
				}
				else if (const auto processedID = DistributeRecordData::GetFormID(a_condition); processedID != 0) {
					a_processedFilters.emplace_back(processedID);
				}
				else {
					_ERROR("\t\tFilter  [%s] INFO - unable to find form, treating filter as string", a_condition.c_str());
					a_processedFilters.emplace_back(a_condition);
				}
				};

			for (auto& [section, comment, keyOrder] : sections) {
				std::vector<std::string> conditions;
				std::vector<std::string> splitSection = string::split(section, "|");
				if (string::icontains(section, "|")) {
					boost::trim(splitSection[1]);
					conditions = string::split(splitSection[1], ",");  //[Forms|EditorID,EditorID2]
					_MESSAGE("\t\treading [%s] : %u conditions", splitSection[0].c_str(), conditions.size());
				}

				std::vector<FormIDStr> processedConditions;
				processedConditions.reserve(conditions.size());
				for (auto& condition : conditions) {
					push_filter(condition, processedConditions);
				}

				CSimpleIniA::TNamesDepend values;
				ini.GetAllKeys(section, values);
				values.sort(CSimpleIniA::Entry::LoadOrder());
				if (splitSection[0] == "Items") {
					if (!values.empty()) {
						_MESSAGE("\t\t\t%u items found", values.size());
						for (const auto& key : values) {
							get_forms(path, key.pItem, processedConditions, splitSection[0]);
						}
					}
				}
				else if (splitSection[0] == "Equipment") {
					if (!values.empty()) {
						_MESSAGE("\t\t\t%u equippables found", values.size());
						for (const auto& key : values) {
							get_forms(path, key.pItem, processedConditions, splitSection[0]);
						}
					}
				}
				else if (splitSection[0] == "Spells") {
					if (!values.empty()) {
						_MESSAGE("\t\t\t%u spells found", values.size());
						for (const auto& key : values) {
							get_forms(path, key.pItem, processedConditions, splitSection[0]);
						}
					}
				}
				else if (splitSection[0] == "Factions") {
					if (!values.empty()) {
						_MESSAGE("\t\t\t%u factions found", values.size());
						for (const auto& key : values) {
							get_forms(path, key.pItem, processedConditions, splitSection[0]);
						}
					}
				}
				else if (splitSection[0] == "Packages") {
					if (!values.empty()) {
						_MESSAGE("\t\t\t%u packages found", values.size());
						for (const auto& key : values) {
							get_forms(path, key.pItem, processedConditions, splitSection[0]);
						}
					}
				}
			}
		}

		_MESSAGE("-RESULT-");

		//_MESSAGE("%u Items processed", allItems.size());
		_MESSAGE("%u Items processed", allItemsConditional.size());
		//_MESSAGE("%u conditional Items processed for ALL\n", applyToAllItems.size());

		//_MESSAGE("%u Equippables processed", allEquipment.size());
		_MESSAGE("%u Equippables processed", allEquipmentConditional.size());
		//_MESSAGE("%u conditional Equippables processed for ALL\n", applyToAllEquipment.size());

		//_MESSAGE("%u Spells processed", allSpells.size());
		_MESSAGE("%u Spells processed", allSpellsConditional.size());
		//_MESSAGE("%u conditional Spells processed for ALL\n", applyToAllSpells.size());

		//_MESSAGE("%u Factions processed", allFactions.size());
		_MESSAGE("%u Factions processed", allFactionsConditional.size());
		//_MESSAGE("%u conditional Factions processed for ALL\n", applyToAllFactions.size());

		//_MESSAGE("%u Packages processed", allPackages.size());
		_MESSAGE("%u Packages processed", allPackagesConditional.size());
		//_MESSAGE("%u conditional Packages processed for ALL\n", applyToAllPackages.size());

		_MESSAGE("-END-");
	}
	void Manager::PrintConflicts() const
	{
		if (hasConflicts) {
			Console_Print(std::format("[SFID] Conflicts found, check SpellFactionItemDistributor.log in {} for more info\n", GetOblivionDirectory()).c_str());
		}
	}

	SFIDResult Manager::GetConditionalBase(TESObjectREFR* a_ref, TESForm* a_base, FormMap<SwapDataConditional> conditionalForms, std::string formType)
	{
		DistributeRecordData empty;
		const auto itRef = conditionalForms.find(a_ref->refID);
		const auto itBase = conditionalForms.find(a_base->refID);
		const auto itAll = conditionalForms.find(static_cast<std::uint32_t>(0xFFFFFFFF));
		bool foundRef = false;
		bool foundBase = false;
		bool foundAll = false;
		if (itRef != conditionalForms.end()) {
			foundRef = true;
		}
		if (itBase != conditionalForms.end()) {
			foundBase = true;
		}
		if (itAll != conditionalForms.end()) {
			foundAll = true;
		}
		if (foundRef || foundBase || foundAll) {
			const ConditionalInput input(a_ref, a_base);
			if (foundRef) {
				const auto             result = std::ranges::find_if(itRef->second, [&](const auto& a_data) {
					return input.IsValidAll(a_data.first, a_ref);
					});
				if (result != itRef->second.end()) {
					for (DistributeRecordData SwapData : result->second | std::ranges::views::reverse) {
						return { a_ref, SwapData };
					}
				}
			}
			else if (foundBase) {
				const auto             result = std::ranges::find_if(itBase->second, [&](const auto& a_data) {
					return input.IsValidAll(a_data.first, a_ref);
					});
				if (result != itBase->second.end()) {
					for (DistributeRecordData SwapData : result->second | std::ranges::views::reverse) {
						return { a_ref, SwapData };
					}
				}
			}
			else {
				const auto             result = std::ranges::find_if(itAll->second, [&](const auto& a_data) {
					return input.IsValidAll(a_data.first, a_ref);
					});
				if (result != itAll->second.end()) {
					for (DistributeRecordData SwapData : result->second | std::ranges::views::reverse) {
						return { a_ref, SwapData };
					}
				}
			}
		}
		return { nullptr, empty };
	}

	std::vector<SFIDResult> Manager::GetBaseAll(TESObjectREFR* a_ref, TESForm* a_base, FormMap<SwapDataConditional> conditionalForms, std::string formType)
	{
		std::vector<SFIDResult> newVec;
		SFIDResult sfidResult;
		DistributeRecordData newSwapData;
		FormIDSet newSet;
		std::random_device rd;
		std::mt19937 g(rd());
		bool getAll = false;

		if (!string::iequals(formType, "Equipment") && !string::iequals(formType, "Items")) {
			getAll = true;
		}

		const auto itRef = conditionalForms.find(a_ref->refID);
		const auto itBase = conditionalForms.find(a_base->refID);
		const auto itAll = conditionalForms.find(static_cast<std::uint32_t>(0xFFFFFFFF));
		bool foundRef = false;
		bool foundBase = false;
		bool foundAll = false;
		if (itRef != conditionalForms.end()) {
			foundRef = true;
		}
		if (itBase != conditionalForms.end()) {
			foundBase = true;
		}
		if (itAll != conditionalForms.end()) {
			foundAll = true;
		}
		if (foundRef || foundBase || foundAll) {
			const ConditionalInput input(a_ref, a_base);
			if (foundRef) {
				for (auto vecData : itRef->second) {
					const ConditionalInput input(a_ref, a_base);
					if (input.IsValidAll(vecData.first, a_ref)) {
						if (getAll) {
							for (DistributeRecordData swapData : vecData.second | std::ranges::views::reverse) {
								sfidResult.first = a_ref;
								sfidResult.second = swapData;
								newVec.push_back(sfidResult);
							}
						}
						else {
							std::shuffle(vecData.second.begin(), vecData.second.end(), g);
							sfidResult.first = a_ref;
							sfidResult.second = vecData.second.at(0);
							newVec.push_back(sfidResult);
						}
					}
				}
			}
			else if (foundBase) {
				for (auto vecData : itBase->second) {
					const ConditionalInput input(a_ref, a_base);
					if (input.IsValidAll(vecData.first, a_ref)) {
						if (getAll) {
							for (DistributeRecordData swapData : vecData.second | std::ranges::views::reverse) {
								sfidResult.first = a_ref;
								sfidResult.second = swapData;
								newVec.push_back(sfidResult);
							}
						}
						else {
							std::shuffle(vecData.second.begin(), vecData.second.end(), g);
							sfidResult.first = a_ref;
							sfidResult.second = vecData.second.at(0);
							newVec.push_back(sfidResult);
						}
					}
				}
			}
			else {
				for (auto vecData : itAll->second) {
					const ConditionalInput input(a_ref, a_base);
					if (input.IsValidAll(vecData.first, a_ref)) {
						if (getAll) {
							for (DistributeRecordData swapData : vecData.second | std::ranges::views::reverse) {
								sfidResult.first = a_ref;
								sfidResult.second = swapData;
								newVec.push_back(sfidResult);
							}
						}
						else {
							std::shuffle(vecData.second.begin(), vecData.second.end(), g);
							sfidResult.first = a_ref;
							sfidResult.second = vecData.second.at(0);
							newVec.push_back(sfidResult);
						}

					}
				}
			}
		}
		return newVec;
	}

	void Manager::LoadCache() {
		LoadFormsOnce();
		std::string formLine;
		std::ifstream idCache;
		idCache.open("SFIDCache.txt");
		while (std::getline(idCache, formLine)) {
			std::stringstream stringStream;
			stringStream << std::hex << formLine;
			UInt32 formID;
			stringStream >> formID;
			cachedForms.emplace(formID);
		}
		idCache.close();
	}

	void Manager::AddToCache(TESObjectREFR* ref)
	{
		if (const auto it = cachedForms.find(ref->refID); it == cachedForms.end()) {
			cachedForms.emplace(ref->refID);
			std::string formString = std::to_string(ref->refID) + "\n";
			std::fstream idCache;
			idCache.open("SFIDCache.txt", std::ios_base::binary | std::ios_base::app);
			idCache << std::hex << ref->refID;
			idCache << '\n';
			idCache.close();
		}
	}

	std::vector<SFIDResult> Manager::GetSingleSwapData(TESObjectREFR* a_ref, TESForm* a_base, std::string formType)
	{
		FormMap<SwapDataConditional> allFormsConditional = get_form_map(formType);

		DistributeRecordData empty;
		std::vector<SFIDResult> emptyResult;
		if (const auto it = processedForms.find(a_ref->refID); it != processedForms.end()) {
			return emptyResult;
		}
		/*
		if (const auto it = cachedForms.find(a_ref->refID); it != cachedForms.end()) {
			
			if (string::iequals(formType, "Items") || string::iequals(formType, "Equipment")) {
				return emptyResult;
			}
		} */
		std::vector<SFIDResult> sfidResult;
		sfidResult = GetBaseAll(a_ref, a_base, allFormsConditional, formType);
		return sfidResult;
	}

	std::vector<std::vector<SFIDResult>> Manager::GetAllSwapData(TESObjectREFR* a_ref, TESForm* a_base) {
		std::vector<std::vector<SFIDResult>> resultVec;
		resultVec.reserve(5);
		if (allFactionsConditional.size() > 0) {
			resultVec.push_back(GetSingleSwapData(a_ref, a_base, "Factions"));
		}
		if (allItemsConditional.size() > 0) {
			resultVec.push_back(GetSingleSwapData(a_ref, a_base, "Items"));
		}
		if (allEquipmentConditional.size() > 0) {
			resultVec.push_back(GetSingleSwapData(a_ref, a_base, "Equipment"));
		}
		if (allSpellsConditional.size() > 0) {
			resultVec.push_back(GetSingleSwapData(a_ref, a_base, "Spells"));
		}
		if (allPackagesConditional.size() > 0) {
			resultVec.push_back(GetSingleSwapData(a_ref, a_base, "Packages"));
		}

		return resultVec;
	}
}