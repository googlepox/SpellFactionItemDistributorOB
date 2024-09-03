#include "Manager.h"
#include "fstream"

extern OBSEScriptInterface* g_script;

namespace SpellFactionItemDistributor
{
	FormCode GetFormCodeFromString(std::string formString) {
		if (formString == "Forms") return form;
		if (formString == "Spells") return spell;
		if (formString == "Factions") return faction;
		if (formString == "Equippables") return equippable;
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
			return allItems;
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
			return allItemsConditional;
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
			return applyToAllItems;
			break;
		}
	}

	void Manager::get_forms(const std::string& a_path, const std::string& a_str, FormMap<SwapDataVec>& a_map, std::string formType)
	{
		return SwapData::GetForms(a_path, a_str, [&](UInt32 a_baseID, const SwapData& a_SwapData) {
			a_map[a_baseID].push_back(a_SwapData);
			});
	}

	void Manager::get_forms(const std::string& a_path, const std::string& a_str, const std::vector<FormIDStr>& a_conditionalIDs, std::string formType)
	{
		return SwapData::GetForms(a_path, a_str, [&](const UInt32 a_baseID, const SwapData& a_SwapData) {
			for (auto& id : a_conditionalIDs) {
				get_form_map(formType)[a_baseID][id].push_back(a_SwapData);
			}
			});
	}

	void Manager::get_forms_all(const std::string& a_path, const std::string& a_str, const std::vector<FormIDStr>& a_conditionalIDs, std::string formType)
	{
		return SwapData::GetForms(a_path, a_str, [&](const UInt32 a_baseID, const SwapData& a_SwapData) {
			for (auto& id : a_conditionalIDs) {
				get_form_map_all(formType)[a_baseID][id].push_back(a_SwapData);
			}
			});
	}

	static bool HasKeywordCell(TESObjectCELL* a_cell, const std::string& a_keyword)
	{
		if (a_cell) {
			std::string editorID = (a_cell->GetEditorID2());
			std::string newKey = a_keyword;
			std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
			std::transform(editorID.begin(), editorID.end(), editorID.begin(), tolower);
			std::string cStrKey = newKey.c_str();
			std::string cStrEditorID = editorID.c_str();
			if ((cStrEditorID.find(cStrKey.c_str()) != std::string::npos) && (cStrKey.find('-') == std::string::npos)) {
				return true;
			}
			return false;
		}
		else {
			return false;
		}

	}

	static bool HasKeywordEditorID(TESObjectREFR* ref, const std::string& a_keyword)
	{
		if (ref) {
			std::string editorID = (ref->GetEditorID2());
			std::string newKey = a_keyword;
			std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
			std::transform(editorID.begin(), editorID.end(), editorID.begin(), tolower);
			std::string cStrKey = newKey.c_str();
			std::string cStrEditorID = editorID.c_str();
			if ((cStrEditorID.find(cStrKey.c_str()) != std::string::npos) && (cStrKey.find('-') == std::string::npos)) {
				return true;
			}
			return false;
		}
		else {
			return false;
		}

	}

	static bool HasKeywordRace(TESObjectREFR* ref, const std::string& a_keyword)
	{
		if (ref) {
			TESActorBase* actor = dynamic_cast<TESActorBase*>(ref->baseForm);
			TESNPC* npc = dynamic_cast<TESNPC*>(actor);
			std::string editorID = (npc->race.race->GetEditorID2());
			std::string refID = std::to_string(npc->race.race->refID).c_str();
			std::string newKey = a_keyword;
			std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
			std::transform(editorID.begin(), editorID.end(), editorID.begin(), tolower);
			std::string cStrKey = newKey.c_str();
			std::string cStrEditorID = editorID.c_str();
			if ((cStrEditorID.find(cStrKey.c_str()) != std::string::npos) && (cStrKey.find('-') == std::string::npos)) {
				return true;
			}
			else if ((refID.find(cStrKey.c_str()) != std::string::npos) && (cStrKey.find('-') == std::string::npos)) {
				return true;
			}
			return false;
		}
		else {
			return false;
		}
	}

	static bool HasKeywordFaction(TESObjectREFR* ref, const std::string& a_keyword)
	{
		if (ref) {
			TESActorBase* actor = dynamic_cast<TESActorBase*>(ref->baseForm);
			TESNPC* npc = dynamic_cast<TESNPC*>(actor);
			TESActorBaseData::FactionListEntry* entry = &npc->actorBaseData.factionList;
			while (entry && entry->data)
			{	
				TESFaction* faction = entry->data->faction;
				std::string editorID = faction->GetEditorID2();
				std::string refID = std::to_string(faction->refID).c_str();
				std::string newKey = a_keyword;
				std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
				std::transform(editorID.begin(), editorID.end(), editorID.begin(), tolower);
				std::string cStrKey = newKey.c_str();
				std::string cStrEditorID = editorID.c_str();
				if ((cStrEditorID.find(cStrKey.c_str()) != std::string::npos) && (cStrKey.find('-') == std::string::npos)) {
					return true;
				}
				else if ((refID.find(cStrKey.c_str()) != std::string::npos) && (cStrKey.find('-') == std::string::npos)) {
					return true;
				}
				entry = entry->Next();
			}
			return false;
		}
		else {
			return false;
		}
	}

	bool ConditionalInput::IsValid(const FormIDStr& a_data) const
	{
		if (std::holds_alternative<UInt32>(a_data)) {
			if (const auto form = LookupFormByID(std::get<UInt32>(a_data))) {
				switch (form->GetFormType()) {
				case kFormType_Region:
				{
					if (const auto region = (form)) {
						if (const auto regionList = dynamic_cast<ExtraRegionList*>(currentCell ? currentCell->extraData.GetByType(kExtraData_RegionList) : nullptr)) {
							if (const auto list = (regionList->regionList))
							{
								TESRegionList::Entry* regionPtr = &(list->regionList);
								while (regionPtr != NULL)
								{
									if (regionPtr->region == region)
									{
										return true;
									}
									regionPtr = regionPtr->next;
								}
							}
						}
					}
					return false;
				}
				case kFormType_Cell:
				{
					return currentCell == form;
				}
				case kFormType_Faction:
				{
					return faction == form;
				}
				case kFormType_Race:
				{
					return race == form;
				}
				default:
					break;
				}
			}
		}
		else {
			if (HasKeywordCell(currentCell, std::get<std::string>(a_data))) {
				return true;
			}
			else if (HasKeywordEditorID(ref, std::get<std::string>(a_data))) {
				return true;
			}
			else if (HasKeywordRace(ref, std::get<std::string>(a_data))) {
				return true;
			}
			else if (HasKeywordFaction(ref, std::get<std::string>(a_data))) {
				return true;
			}
			else {
				return false;
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
				if (const auto processedID = SwapData::GetFormID(a_condition); processedID != 0) {
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
									get_forms_all(path, key.pItem, processedConditions, section);
								}
								else {
									get_forms(path, key.pItem, processedConditions, section);
								}
							}
						}
					}
					else if (splitSection[0] == "Equipment") {
						if (!values.empty()) {
							_MESSAGE("\t\t\t%u equippables found", values.size());
							for (const auto& key : values) {
								if (string::icontains(key.pItem, "ALL")) {
									get_forms_all(path, key.pItem, processedConditions, section);
								}
								else {
									get_forms(path, key.pItem, processedConditions, section);
								}
							}
						}
					}
					else if (splitSection[0] == "Spells") {
						if (!values.empty()) {
							_MESSAGE("\t\t\t%u spells found", values.size());
							for (const auto& key : values) {
								if (string::icontains(key.pItem, "ALL")) {
									get_forms_all(path, key.pItem, processedConditions, section);
								}
								else {
									get_forms(path, key.pItem, processedConditions, section);
								}
							}
						}
					}
					else if (splitSection[0] == "Factions") {
						if (!values.empty()) {
							_MESSAGE("\t\t\t%u factions found", values.size());
							for (const auto& key : values) {
								if (string::icontains(key.pItem, "ALL")) {
									get_forms_all(path, key.pItem, processedConditions, section);
								}
								else {
									get_forms(path, key.pItem, processedConditions, section);
								}
							}
						}
					}
					else if (splitSection[0] == "Packages") {
						if (!values.empty()) {
							_MESSAGE("\t\t\t%u packages found", values.size());
							for (const auto& key : values) {
								if (string::icontains(key.pItem, "ALL")) {
									get_forms_all(path, key.pItem, processedConditions, section);
								}
								else {
									get_forms(path, key.pItem, processedConditions, section);
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
						_MESSAGE("it's a faction section");
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

		_MESSAGE("%u Equippables processed", allEquipment.size());
		_MESSAGE("%u conditional Equippables processed", allEquipmentConditional.size());

		_MESSAGE("%u Spells processed", allSpells.size());
		_MESSAGE("%u conditional Spells processed", allSpellsConditional.size());

		_MESSAGE("%u Factions processed", allFactions.size());
		_MESSAGE("%u conditional Factions processed", allFactionsConditional.size());

		_MESSAGE("%u Packages processed", allPackages.size());
		_MESSAGE("%u conditional Packages processed", allPackagesConditional.size());

		_MESSAGE("-CONFLICTS-");

		// TODO

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
		log_conflicts("Packages", allPackages);

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
		SwapData empty;

		if (!a_base) {
			if (!a_ref) {
				return { nullptr, empty };;
			}
			if (const auto it = conditionalForms.find(static_cast<std::uint32_t>(0xFFFFFFFF)); it != conditionalForms.end()) {;
				const ConditionalInput input(a_ref, a_base);
				const auto             result = std::ranges::find_if(it->second, [&](const auto& a_data) {
					return input.IsValid(a_data.first);
					});

				if (result != it->second.end()) {
					for (SwapData SwapData : result->second | std::ranges::views::reverse) {
						return { a_ref, SwapData };
					}
				}
				else {
					return { nullptr, empty };
				}
			}
			else if (const auto it = get_form_vec(formType).find(static_cast<std::uint32_t>(0xFFFFFFFF)); it != get_form_vec(formType).end()) {
				for (SwapData swapData : it->second | std::ranges::views::reverse) {
					if (!swapData.GetSwapBase(a_ref)) {
						return { a_ref, swapData };
					}
					else {
						return { a_ref, swapData };
					}
				}
			}
		}
		else if (const auto it = conditionalForms.find(static_cast<std::uint32_t>(a_base->refID)); it != conditionalForms.end()) {
			const ConditionalInput input(a_ref, a_base);
			const auto             result = std::ranges::find_if(it->second, [&](const auto& a_data) {
				return input.IsValid(a_data.first);
				});

			if (result != it->second.end()) {
				for (SwapData SwapData : result->second | std::ranges::views::reverse) {
					return { a_ref, SwapData };
				}
			}
		}

		return { nullptr, empty };
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
		std::string formLine;
		std::ifstream idCache;
		idCache.open("SFIDCache.txt");
		while (std::getline(idCache, formLine)) {
			std::stringstream stringStream;
			stringStream << std::hex << formLine;
			UInt32 formID;
			stringStream >> formID;
			processedForms.emplace(formID);
		}
		idCache.close();
	}

	void Manager::AddToCache(UInt32 formID)
	{
		processedForms.emplace(formID);
	}

	SFIDResult Manager::GetSwapData(TESObjectREFR* a_ref, TESForm* a_base)
	{
		SwapData empty;

		const auto get_swap_base = [a_ref](const TESForm* a_form, const FormMap<SwapDataVec>& a_map) -> SFIDResult {
			if (const auto it = a_map.find(a_form->refID); it != a_map.end()) {
				for (SwapData swapData : it->second | std::ranges::views::reverse) {
					if (!swapData.GetSwapBase(a_ref)) {
						return { a_ref, swapData };
					}
					else {
						return { a_ref, swapData };
					}
				}
			}
			SwapData empty;
			return { nullptr, empty };
			};

		SFIDResult SFIDResult{ a_ref, empty };

		if (a_ref->refID < 0xFF000000) {
			//SFIDResult = get_swap_base(a_base, allForms);
		}

		if (!SFIDResult.first) {
			//SFIDResult = GetConditionalBase(a_ref, a_base, allFormsConditional);
		}
		
		if (!SFIDResult.first) {
			//SFIDResult = GetConditionalBase(a_ref, nullptr, applyToAllForms);
		}

		if (!SFIDResult.first) {
			//SFIDResult = GetConditionalBase(nullptr, nullptr, applyToAllForms);
		}
		
		if (const auto it = processedForms.find(a_ref->refID); it == processedForms.end()) {
			return SFIDResult;
			//return GetConditionalBase(a_ref, nullptr, applyToAllForms);
		}
		else if (const auto it = processedForms.find(a_ref->refID); it != processedForms.end()) {

		}
		else {
			return { nullptr, empty };
		}
	}

	SFIDResult Manager::GetSingleSwapData(TESObjectREFR* a_ref, TESForm* a_base, std::string formType)
	{
		FormMap<SwapDataVec> allForms = get_form_vec(formType);
		FormMap<SwapDataConditional> allFormsConditional = get_form_map(formType);
		FormMap<SwapDataConditional> applyToAllForms = get_form_map_all(formType);

		SwapData empty;
		SFIDResult emptyResult = { nullptr, empty };

		const auto get_swap_base = [a_ref](const TESForm* a_form, const FormMap<SwapDataVec>& a_map) -> SFIDResult {
			if (const auto it = a_map.find(a_form->refID); it != a_map.end()) {
				for (SwapData swapData : it->second | std::ranges::views::reverse) {
					if (!swapData.GetSwapBase(a_ref)) {
						return { a_ref, swapData };
					}
					else {
						return { a_ref, swapData };
					}
				}
			}
			SwapData empty;
			return { nullptr, empty };
			};

		SFIDResult SFIDResult{ a_ref, empty };

		if (a_ref->refID < 0xFF000000) {
			SFIDResult = get_swap_base(a_base, allForms);
		}

		if (!SFIDResult.first) {
			SFIDResult = GetConditionalBase(a_ref, a_base, allFormsConditional, formType);
		}

		if (!SFIDResult.first) {
			SFIDResult = GetConditionalBase(a_ref, nullptr, applyToAllForms, formType);
		}

		if (!SFIDResult.first) {
			SFIDResult = GetConditionalBase(nullptr, nullptr, applyToAllForms, formType);
		}

		if (const auto it = processedForms.find(a_ref->refID); it == processedForms.end()) {
			return SFIDResult;
			//return GetConditionalBase(a_ref, nullptr, applyToAllForms);
		}
		else if (formType != "Items" || formType != "Equipment") {
			return SFIDResult;
		}
		else {
			return { nullptr, empty };
		}
	}

	std::vector<SFIDResult> Manager::GetAllSwapData(TESObjectREFR* a_ref, TESForm* a_base) {
		std::vector<SFIDResult> resultVec;

		if (allItems.size() > 0 || allItems.size() > 0 || applyToAllItems.size() > 0) {
			resultVec.push_back(GetSingleSwapData(a_ref, a_base, "Items"));
		}
		if (allEquipment.size() > 0 || allEquipmentConditional.size() > 0 || applyToAllEquipment.size() > 0) {
			resultVec.push_back(GetSingleSwapData(a_ref, a_base, "Equippables"));
		}
		if (allSpells.size() > 0 || allSpellsConditional.size() > 0 || applyToAllSpells.size() > 0) {
			resultVec.push_back(GetSingleSwapData(a_ref, a_base, "Spells"));
		}
		if (allFactions.size() > 0 || allFactionsConditional.size() > 0 || applyToAllFactions.size() > 0) {
			_MESSAGE("FACTION");
			resultVec.push_back(GetSingleSwapData(a_ref, a_base, "Factions"));
		}
		if (allPackages.size() > 0 || allPackagesConditional.size() > 0 || applyToAllPackages.size() > 0) {
			resultVec.push_back(GetSingleSwapData(a_ref, a_base, "Packages"));
		}

		return resultVec;
	}
}