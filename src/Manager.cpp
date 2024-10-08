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
			std::vector<std::string> conditionType = string::split(formString, ":");
			boost::trim(conditionType[0]);
			boost::trim(conditionType[1]);
			//_MESSAGE("%s", conditionType[0].c_str());
			//_MESSAGE("%s", conditionType[1].c_str());
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
				if (a_condition.contains('&') || a_condition.contains(':')) {
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
				if (string::icontains(section, "|")) {
					auto splitSection = string::split(section, "|");
					boost::trim(splitSection[1]);
					auto conditions = string::split(splitSection[1], ",");  //[Forms|EditorID,EditorID2]

					_MESSAGE("\t\treading [%s] : %u conditions", splitSection[0].c_str(), conditions.size());

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
								if (string::icontains(key.pItem, "ALL")) {
									get_forms_all(path, key.pItem, processedConditions, splitSection[0]);
								}
								else {
									get_forms(path, key.pItem, processedConditions, splitSection[0]);
								}
							}
						}
					}
					else if (splitSection[0] == "Equipment") {
						if (!values.empty()) {
							_MESSAGE("\t\t\t%u equippables found", values.size());
							for (const auto& key : values) {
								if (string::icontains(key.pItem, "ALL")) {
									get_forms_all(path, key.pItem, processedConditions, splitSection[0]);
								}
								else {
									get_forms(path, key.pItem, processedConditions, splitSection[0]);
								}
							}
						}
					}
					else if (splitSection[0] == "Spells") {
						if (!values.empty()) {
							_MESSAGE("\t\t\t%u spells found", values.size());
							for (const auto& key : values) {
								if (string::icontains(key.pItem, "ALL")) {
									get_forms_all(path, key.pItem, processedConditions, splitSection[0]);
								}
								else {
									get_forms(path, key.pItem, processedConditions, splitSection[0]);
								}
							}
						}
					}
					else if (splitSection[0] == "Factions") {
						if (!values.empty()) {
							_MESSAGE("\t\t\t%u factions found", values.size());
							for (const auto& key : values) {
								if (string::icontains(key.pItem, "ALL")) {
									get_forms_all(path, key.pItem, processedConditions, splitSection[0]);
								}
								else {
									get_forms(path, key.pItem, processedConditions, splitSection[0]);
								}
							}
						}
					}
					else if (splitSection[0] == "Packages") {
						if (!values.empty()) {
							_MESSAGE("\t\t\t%u packages found", values.size());
							for (const auto& key : values) {
								if (string::icontains(key.pItem, "ALL")) {
									get_forms_all(path, key.pItem, processedConditions, splitSection[0]);
								}
								else {
									get_forms(path, key.pItem, processedConditions, splitSection[0]);
								}
							}
						}
					}
				}
				else {
					_MESSAGE("\t\treading [%s]", section);

					CSimpleIniA::TNamesDepend values;
					ini.GetAllKeys(section, values);
					values.sort(CSimpleIniA::Entry::LoadOrder());

					if (string::iequals(section, "Items")) {
						if (!values.empty()) {

							_MESSAGE("\t\t\t%u items found", values.size());
							for (const auto& key : values) {
								get_forms(path, key.pItem, allItems, section);
							}
						}
					}
					else if (string::iequals(section, "Equipment")) {
						if (!values.empty()) {
							_MESSAGE("\t\t\t%u equippables found", values.size());
							for (const auto& key : values) {
								get_forms(path, key.pItem, allEquipment, section);
							}
						}
					}
					else if (string::iequals(section, "Spells")) {
						if (!values.empty()) {
							_MESSAGE("\t\t\t%u spells found", values.size());
							for (const auto& key : values) {
								get_forms(path, key.pItem, allSpells, section);
							}
						}
					}
					else if (string::iequals(section, "Factions")) {
						if (!values.empty()) {
							_MESSAGE("\t\t\t%u factions found", values.size());
							for (const auto& key : values) {
								get_forms(path, key.pItem, allFactions, section);
							}
						}
					}
					else if (string::iequals(section, "Packages")) {
						if (!values.empty()) {
							_MESSAGE("\t\t\t%u packages found", values.size());
							for (const auto& key : values) {
								get_forms(path, key.pItem, allPackages, section);
							}
						}
					}
				}
			}
		}

		_MESSAGE("-RESULT-");

		_MESSAGE("%u Items processed", allItems.size());
		_MESSAGE("%u conditional Items processed", allItemsConditional.size());
		_MESSAGE("%u conditional Items processed for ALL\n", applyToAllItems.size());

		_MESSAGE("%u Equippables processed", allEquipment.size());
		_MESSAGE("%u conditional Equippables processed", allEquipmentConditional.size());
		_MESSAGE("%u conditional Equippables processed for ALL\n", applyToAllEquipment.size());

		_MESSAGE("%u Spells processed", allSpells.size());
		_MESSAGE("%u conditional Spells processed", allSpellsConditional.size());
		_MESSAGE("%u conditional Spells processed for ALL\n", applyToAllSpells.size());

		_MESSAGE("%u Factions processed", allFactions.size());
		_MESSAGE("%u conditional Factions processed", allFactionsConditional.size());
		_MESSAGE("%u conditional Factions processed for ALL\n", applyToAllFactions.size());

		_MESSAGE("%u Packages processed", allPackages.size());
		_MESSAGE("%u conditional Packages processed", allPackagesConditional.size());
		_MESSAGE("%u conditional Packages processed for ALL\n", applyToAllPackages.size());

		_MESSAGE("-CONFLICTS-");

		// TODO
		/*
		const auto log_conflicts = [&]<typename T>(std::string_view a_type, const FormMap<T>&a_map) {
			if (a_map.empty()) {
				return;
			}
			bool conflicts = false;
			for (auto& [baseID, SwapDataVec] : a_map) {
				if (SwapDataVec.size() > 1) {
					const auto& winningRecord = SwapDataVec.back();
					if (winningRecord.traits.chance != 100) {  //ignore if winning record is randomized
						continue;
					}
					conflicts = true;
					auto winningForm = string::split(winningRecord.record, "|");
					_WARNING("\t%s", winningForm[0].c_str());
					_WARNING("\t\twinning record : %s (%s)", winningForm[1].c_str(), SwapDataVec.back().path.c_str());
					_WARNING("\t\t%u conflicts", SwapDataVec.size() - 1);
					for (auto it = SwapDataVec.rbegin() + 1; it != SwapDataVec.rend(); ++it) {
						auto losingRecord = it->record.substr(it->record.find('|') + 1);
						_WARNING("\t\t\t%s (%s)", losingRecord.c_str(), it->path.c_str());
					}
				}
			}
			if (!conflicts) {
				_MESSAGE("\tNo conflicts found");
			}
			else {
				hasConflicts = true;
			}
		};

		log_conflicts("Items", allItems);
		log_conflicts("Equippables", allEquipment);
		log_conflicts("Spells", allSpells);
		log_conflicts("Factions", allFactions);
		log_conflicts("Packages", allPackages); */

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
		if (const auto it = conditionalForms.find(a_ref->refID); it != conditionalForms.end()) {
			const ConditionalInput input(a_ref, a_base);
			const auto             result = std::ranges::find_if(it->second, [&](const auto& a_data) {
				return input.IsValidAll(a_data.first, a_ref);
				});
			if (result != it->second.end()) {
				for (DistributeRecordData SwapData : result->second | std::ranges::views::reverse) {
					return { a_ref, SwapData };
				}
			}
			else {
				return { nullptr, empty };
			}
		}
		else if (const auto it = conditionalForms.find(a_base->refID); it != conditionalForms.end()) {
			const ConditionalInput input(a_ref, a_base);
			const auto             result = std::ranges::find_if(it->second, [&](const auto& a_data) {
				return input.IsValidAll(a_data.first, a_ref);
				});
			if (result != it->second.end()) {
				for (DistributeRecordData SwapData : result->second | std::ranges::views::reverse) {
					return { a_ref, SwapData };
				}
			}
		}
		else if (const auto it = conditionalForms.find(static_cast<std::uint32_t>(0xFFFFFFFF)); it != conditionalForms.end()) {
			const ConditionalInput input(a_ref, a_base);
			const auto             result = std::ranges::find_if(it->second, [&](const auto& a_data) {
				return input.IsValidAll(a_data.first, a_ref);
				});
			if (result != it->second.end()) {
				for (DistributeRecordData SwapData : result->second | std::ranges::views::reverse) {
					return { a_ref, SwapData };
				}
			}
		}
		return { nullptr, empty };
	}

	std::vector<SFIDResult> Manager::GetBaseAll(TESObjectREFR* a_ref, TESForm* a_base, FormMap<SwapDataVec> forms, FormMap<SwapDataConditional> conditionalForms, FormMap<SwapDataConditional> conditionalFormsAll, std::string formType)
	{
		std::vector<SFIDResult> newVec;
		newVec.reserve(1024);
		SFIDResult result;
		DistributeRecordData newSwapData;
		FormIDSet newSet;
		newSet.reserve(2048);
		if (const auto it = forms.find(a_ref->refID); it != forms.end()) {
			for (DistributeRecordData swapData : it->second | std::ranges::views::reverse) {
				//newSwapData = swapData;
				result.first = a_ref;
				result.second = swapData;
				newVec.push_back(result);
				if (it->second.size() > 1) {
					for (auto swapItem : it->second) {
						if (std::holds_alternative<UInt32>(swapItem.formToAdd)) {
							newSet.emplace(std::get<UInt32>(swapItem.formToAdd));
						}
						else if (std::holds_alternative<FormIDSet>(swapItem.formToAdd)) {
							for (auto setItem : std::get<FormIDSet>(swapItem.formToAdd)) {
								newSet.emplace(setItem);
							}
						}
					}
				}
				if (std::holds_alternative<UInt32>(swapData.formToAdd)) {
					newSet.emplace(std::get<UInt32>(swapData.formToAdd));
				}
				else if (std::holds_alternative<FormIDSet>(swapData.formToAdd)) {
					for (auto setItem : std::get<FormIDSet>(swapData.formToAdd)) {
						newSet.emplace(setItem);
					}
				}
			}
		}
		if (const auto it = forms.find(a_base->refID); it != forms.end()) {
			for (DistributeRecordData swapData : it->second | std::ranges::views::reverse) {
				//newSwapData = swapData;
				result.first = a_ref;
				result.second = swapData;
				newVec.push_back(result);
				if (it->second.size() > 1) {
					for (auto swapItem : it->second) {
						if (std::holds_alternative<UInt32>(swapItem.formToAdd)) {
							newSet.emplace(std::get<UInt32>(swapItem.formToAdd));
						}
						else if (std::holds_alternative<FormIDSet>(swapItem.formToAdd)) {
							for (auto setItem : std::get<FormIDSet>(swapItem.formToAdd)) {
								newSet.emplace(setItem);
							}
						}
					}
				}
				if (std::holds_alternative<UInt32>(swapData.formToAdd)) {
					newSet.emplace(std::get<UInt32>(swapData.formToAdd));
				}
				else if (std::holds_alternative<FormIDSet>(swapData.formToAdd)) {
					for (auto setItem : std::get<FormIDSet>(swapData.formToAdd)) {
						newSet.emplace(setItem);
					}
				}
			}
		}
		if (const auto it = conditionalForms.find(a_ref->refID); it != conditionalForms.end()) {
			for (auto vecData : it->second) {
				const ConditionalInput input(a_ref, a_base);
				if (input.IsValidAll(vecData.first, a_ref)) {
					for (DistributeRecordData swapData : vecData.second | std::ranges::views::reverse) {
						//newSwapData = swapData;
						result.first = a_ref;
						result.second = swapData;
						newVec.push_back(result);
						if (std::holds_alternative<UInt32>(swapData.formToAdd)) {
							newSet.emplace(std::get<UInt32>(swapData.formToAdd));
						}
						else if (std::holds_alternative<FormIDSet>(swapData.formToAdd)) {
							for (auto setItem : std::get<FormIDSet>(swapData.formToAdd)) {
								newSet.emplace(setItem);
							}
						}
					}
				}
			}
		}
		if (const auto it = conditionalForms.find(a_base->refID); it != conditionalForms.end()) {
			for (auto vecData : it->second) {
				const ConditionalInput input(a_ref, a_base);
				if (input.IsValidAll(vecData.first, a_ref)) {
					for (DistributeRecordData swapData : vecData.second | std::ranges::views::reverse) {
						//newSwapData = swapData;
						result.first = a_ref;
						result.second = swapData;
						newVec.push_back(result);
						if (std::holds_alternative<UInt32>(swapData.formToAdd)) {
							newSet.emplace(std::get<UInt32>(swapData.formToAdd));
						}
						else if (std::holds_alternative<FormIDSet>(swapData.formToAdd)) {
							for (auto setItem : std::get<FormIDSet>(swapData.formToAdd)) {
								newSet.emplace(setItem);
							}
						}
					}
				}
			}
		}
		if (const auto it = conditionalFormsAll.find(static_cast<std::uint32_t>(0xFFFFFFFF)); it != conditionalFormsAll.end()) {
			for (auto vecData : it->second) {
				const ConditionalInput input(a_ref, a_base);
				if (input.IsValidAll(vecData.first, a_ref)) {
					if (!string::iequals(formType, "Equipment")) {
						for (DistributeRecordData swapData : vecData.second | std::ranges::views::reverse) {
							result.first = a_ref;
							result.second = swapData;
							newVec.push_back(result);
							if (std::holds_alternative<UInt32>(swapData.formToAdd)) {
								newSet.emplace(std::get<UInt32>(swapData.formToAdd));
							}
							else if (std::holds_alternative<FormIDSet>(swapData.formToAdd)) {
								for (auto setItem : std::get<FormIDSet>(swapData.formToAdd)) {
									newSet.emplace(setItem);
								}
							}
						}
					}
					else {
						std::random_device rd;
						std::mt19937 g(rd());
						std::shuffle(vecData.second.begin(), vecData.second.end(), g);
						result.first = a_ref;
						result.second = vecData.second.at(0);
						newVec.push_back(result);
					}
					
				}
			}
		}
		//newSwapData.formToAdd = newSet;
		//newSwapData.formIDSet = newSet;
		return newVec;
		//return { a_ref, newVec };
	}

	void Manager::InsertLeveledItemRef(const TESObjectREFR* a_refr)
	{
		swappedLeveledItemRefs.insert(a_refr->refID);
	}

	bool Manager::IsLeveledItemRefSwapped(const TESObjectREFR* a_refr) const
	{
		return swappedLeveledItemRefs.contains(a_refr->refID);
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
			//processedForms.emplace(formID);
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
		FormMap<SwapDataVec> allForms = get_form_vec(formType);
		FormMap<SwapDataConditional> allFormsConditional = get_form_map(formType);
		FormMap<SwapDataConditional> applyToAllForms = get_form_map_all(formType);

		DistributeRecordData empty;
		std::vector<SFIDResult> emptyResult{};

		if (const auto it = processedForms.find(a_ref->refID); it != processedForms.end()) {
			return emptyResult;
		}

		if (const auto it = cachedForms.find(a_ref->refID); it != cachedForms.end()) {
			
			if (string::iequals(formType, "Items") || string::iequals(formType, "Equipment")) {
				return emptyResult;
			}
		}

		std::vector<SFIDResult> sfidResult{};
		sfidResult.reserve(1024);

		sfidResult = GetBaseAll(a_ref, a_base, allForms, allFormsConditional, applyToAllForms, formType);
		//_MESSAGE("sfidResult size: %u", sfidResult.size());
		return sfidResult;
	}

	std::vector<std::vector<SFIDResult>> Manager::GetAllSwapData(TESObjectREFR* a_ref, TESForm* a_base) {
		std::vector<std::vector<SFIDResult>> resultVec{ };
		resultVec.reserve(5);
		if (allFactions.size() > 0 || allFactionsConditional.size() > 0 || applyToAllFactions.size() > 0) {
			resultVec.push_back(GetSingleSwapData(a_ref, a_base, "Factions"));
		}
		if (allItems.size() > 0 || allItems.size() > 0 || applyToAllItems.size() > 0) {
			resultVec.push_back(GetSingleSwapData(a_ref, a_base, "Items"));
		}
		if (allEquipment.size() > 0 || allEquipmentConditional.size() > 0 || applyToAllEquipment.size() > 0) {
			resultVec.push_back(GetSingleSwapData(a_ref, a_base, "Equipment"));
		}
		if (allSpells.size() > 0 || allSpellsConditional.size() > 0 || applyToAllSpells.size() > 0) {
			resultVec.push_back(GetSingleSwapData(a_ref, a_base, "Spells"));
		}
		if (allPackages.size() > 0 || allPackagesConditional.size() > 0 || applyToAllPackages.size() > 0) {
			resultVec.push_back(GetSingleSwapData(a_ref, a_base, "Packages"));
		}

		return resultVec;
	}
}