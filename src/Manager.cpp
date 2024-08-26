#include "Manager.h"
#include "fstream"

extern OBSEScriptInterface* g_script;

namespace SpellFactionItemDistributor
{
	enum FormCode {
		form,
		spell,
		faction,
		equippable,
		item
	};

	FormCode GetFormCodeFromString(std::string const& formString) {
		if (formString == "Forms") return form;
		if (formString == "Spells") return spell;
		if (formString == "Factions") return faction;
		if (formString == "Equippables") return equippable;
		if (formString == "Items") return item;
	}

	FormMap<SwapDataVec>& Manager::get_form_map(const std::string& a_str)
	{
		switch (GetFormCodeFromString(a_str))
		{
		case (form): {
			return allForms;
			break;
		}
		default:
			return allForms;
			break;
		}
	}

	void Manager::get_forms(const std::string& a_path, const std::string& a_str, FormMap<SwapDataVec>& a_map)
	{
		return SwapData::GetForms(a_path, a_str, [&](UInt32 a_baseID, const SwapData& a_SwapData) {
			a_map[a_baseID].push_back(a_SwapData);
			});
	}

	void Manager::get_forms(const std::string& a_path, const std::string& a_str, const std::vector<FormIDStr>& a_conditionalIDs)
	{
		return SwapData::GetForms(a_path, a_str, [&](const UInt32 a_baseID, const SwapData& a_SwapData) {
			for (auto& id : a_conditionalIDs) {
				allFormsConditional[a_baseID][id].push_back(a_SwapData);
			}
			});
	}

	void Manager::get_forms_all(const std::string& a_path, const std::string& a_str, const std::vector<FormIDStr>& a_conditionalIDs)
	{
		return SwapData::GetForms(a_path, a_str, [&](const UInt32 a_baseID, const SwapData& a_SwapData) {
			for (auto& id : a_conditionalIDs) {
				applyToAllForms[a_baseID][id].push_back(a_SwapData);
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

	bool ConditionalInput::IsValid(const FormIDStr& a_data) const
	{
		_MESSAGE("is valid?");
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
			return HasKeywordCell(currentCell, std::get<std::string>(a_data));
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

					if (!values.empty()) {
						_MESSAGE("\t\t\t%u Items found", values.size());
						for (const auto& key : values) {
							if (string::icontains(key.pItem, "ALL")) {
								get_forms_all(path, key.pItem, processedConditions);
							}
							else {
								get_forms(path, key.pItem, processedConditions);
							}
						}
					}
				}
				else {
					_MESSAGE("\t\treading [%s]", section);

					CSimpleIniA::TNamesDepend values;
					ini.GetAllKeys(section, values);
					values.sort(CSimpleIniA::Entry::LoadOrder());

					if (!values.empty()) {
						_MESSAGE("\t\t\t%u items found", values.size());
						for (const auto& key : values) {
							get_forms(path, key.pItem, allForms);
						}
					}
				}
			}
		}

		_MESSAGE("-RESULT-");

		_MESSAGE("%u forms processed", allForms.size());
		_MESSAGE("%u conditional forms processed", allFormsConditional.size());

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

		log_conflicts("Forms", allForms);

		_MESSAGE("-END-");
	}

	void Manager::PrintConflicts() const
	{
		if (hasConflicts) {
			Console_Print(std::format("[SFID] Conflicts found, check SpellFactionItemDistributor.log in {} for more info\n", GetOblivionDirectory()).c_str());
		}
	}

	SFIDResult Manager::GetConditionalBase(TESObjectREFR* a_ref, TESForm* a_base, FormMap<SwapDataConditional> conditionalForms)
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

				_MESSAGE("GetConditionalBase first if");
				if (result != it->second.end()) {
					for (SwapData SwapData : result->second | std::ranges::views::reverse) {
						return { a_ref, SwapData };
					}
				}
				else {
					return { nullptr, empty };
				}
			}
			else if (const auto it = allForms.find(static_cast<std::uint32_t>(0xFFFFFFFF)); it != allForms.end()) {
				_MESSAGE("first if for all forms");
				for (SwapData swapData : it->second | std::ranges::views::reverse) {
					_MESSAGE("looping for all forms");
					if (!swapData.GetSwapBase(a_ref)) {
						_MESSAGE("loop is for all forms");
						return { a_ref, swapData };
					}
					else {
						_MESSAGE("loop else for all forms");
						return { a_ref, swapData };
					}
				}
				SwapData empty;
				return { nullptr, empty };
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
		_MESSAGE("opening cache");
		idCache.open("SFIDCache.txt");
		while (std::getline(idCache, formLine)) {
			_MESSAGE("loaded line %s", formLine.c_str());
			processedForms.emplace(atoi(formLine.c_str()));
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
				_MESSAGE("first if");
				for (SwapData swapData : it->second | std::ranges::views::reverse) {
					_MESSAGE("looping");
					if (!swapData.GetSwapBase(a_ref)) {
						_MESSAGE("loop is");
						return { a_ref, swapData };
					}
					else {
						_MESSAGE("loop else");
						return { a_ref, swapData };
					}
				}
			}
			SwapData empty;
			return { nullptr, empty };
			};

		SFIDResult SFIDResult{ a_ref, empty };

		if (a_ref->refID < 0xFF000000) {
			_MESSAGE("refID check");
			SFIDResult = get_swap_base(a_base, allForms);
		}

		if (!SFIDResult.first) {
			_MESSAGE("no first 1");
			SFIDResult = GetConditionalBase(a_ref, a_base, allFormsConditional);
		}
		
		if (!SFIDResult.first) {
			_MESSAGE("no first 2");
			SFIDResult = GetConditionalBase(a_ref, nullptr, applyToAllForms);
		}

		if (!SFIDResult.first) {
			_MESSAGE("no first 3");
			SFIDResult = GetConditionalBase(nullptr, nullptr, applyToAllForms);
		}
		
		if (const auto it = processedForms.find(a_ref->refID); it == processedForms.end()) {
			_MESSAGE("not found");
			return GetConditionalBase(a_ref, nullptr, applyToAllForms);
		}
		else {
			_MESSAGE("found");
			return { nullptr, empty };
		}
	}
}