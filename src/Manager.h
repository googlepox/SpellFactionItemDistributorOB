#pragma once

#include "SwapData.h"
#include "Defs.h"

namespace SpellFactionItemDistributor
{
	enum FormCode {
		form,
		spell,
		faction,
		package,
		equippable,
		item
	};

	struct ConditionalInput
	{
		ConditionalInput(TESObjectREFR* a_ref, TESForm* a_form) :
			ref(a_ref),
			base(a_form),
			currentCell(a_ref->parentCell),
			faction(nullptr),
			race(nullptr)
		{
		}

		[[nodiscard]] bool IsValid(const FormIDStr& a_data, TESObjectREFR* refToCheck) const;

		// members
		TESObjectREFR* ref;
		TESForm* base;
		TESObjectCELL* currentCell;
		TESFaction* faction;
		TESRace* race;
		std::string editorID;
		std::string name;
	};

	class Manager
	{
	public:

		static Manager* GetSingleton()
		{
			static Manager singleton;
			return std::addressof(singleton);
		}

		void LoadFormsOnce();

		void            PrintConflicts() const;
		SFIDResult      GetSingleSwapData(TESObjectREFR* a_ref, TESForm* a_base, std::string formType);
		std::vector<SFIDResult> GetAllSwapData(TESObjectREFR* a_ref, TESForm* a_base);
		SFIDResult      GetConditionalBase(TESObjectREFR* a_ref, TESForm* a_base, FormMap<SwapDataConditional> conditionalForms, std::string formType);

		void InsertLeveledItemRef(const TESObjectREFR* a_refr);
		bool IsLeveledItemRefSwapped(const TESObjectREFR* a_refr) const;

		void LoadCache();
		short loadCounter;
		void AddToCache(TESObjectREFR* ref);
		std::unordered_set<UInt32>     processedForms;
		std::unordered_set<UInt32>     cachedForms;

	private:
		Manager() = default;
		~Manager() = default;

		Manager(const Manager&) = delete;
		Manager(Manager&&) = delete;
		Manager& operator=(const Manager&) = delete;
		Manager& operator=(Manager&&) = delete;

		void LoadForms();

		FormMap<SwapDataVec>& get_form_vec(const std::string& a_str);
		FormMap<SwapDataConditional>& get_form_map(const std::string& a_str);
		FormMap<SwapDataConditional>& get_form_map_all(const std::string& a_str);
		static void           get_forms(const std::string& a_path, const std::string& a_str, FormMap<SwapDataVec>& a_map, std::string);
		void                  get_forms(const std::string& a_path, const std::string& a_str, const std::vector<FormIDStr>& a_conditionalIDs, std::string);

		void get_forms_all(const std::string& a_path, const std::string& a_str, const std::vector<FormIDStr>& applyToAllForms, std::string);

		FormMap<SwapDataVec> allItems{};
		FormMap<SwapDataConditional> allItemsConditional{};
		FormMap<SwapDataConditional> applyToAllItems{};

		FormMap<SwapDataVec> allEquipment{};
		FormMap<SwapDataConditional> allEquipmentConditional{};
		FormMap<SwapDataConditional> applyToAllEquipment{};

		FormMap<SwapDataVec> allSpells{};
		FormMap<SwapDataConditional> allSpellsConditional{};
		FormMap<SwapDataConditional> applyToAllSpells{};

		FormMap<SwapDataVec> allFactions{};
		FormMap<SwapDataConditional> allFactionsConditional{};
		FormMap<SwapDataConditional> applyToAllFactions{};

		FormMap<SwapDataVec> allPackages{};
		FormMap<SwapDataConditional> allPackagesConditional{};
		FormMap<SwapDataConditional> applyToAllPackages{};


		std::unordered_set<std::uint32_t> swappedLeveledItemRefs{};

		bool           hasConflicts{ false };
		std::once_flag init{};
	};
}