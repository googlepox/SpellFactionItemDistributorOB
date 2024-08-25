#pragma once

#include "SwapData.h"

namespace SpellFactionItemDistributor
{
	inline Script* HasKeywordScript;
	[[nodiscard]] bool HasKeyword(TESObjectCELL* a_cell, const std::string& a_keyword);

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

		[[nodiscard]] bool IsValid(const FormIDStr& a_data) const;

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
		SFIDResult      GetSwapData(TESObjectREFR* a_ref, TESForm* a_base);
		SFIDResult      GetConditionalBase(TESObjectREFR* a_ref, TESForm* a_base, FormMap<SwapDataConditional> conditionalForms);
		SFIDResult GetSpellData(TESObjectREFR* a_ref, TESForm* a_base);
		SFIDResult GetFactionData(TESObjectREFR* a_ref, TESForm* a_base);
		SFIDResult GetEquippableData(TESObjectREFR* a_ref, TESForm* a_base);
		SFIDResult GetItemData(TESObjectREFR* a_ref, TESForm* a_base);

		void InsertLeveledItemRef(const TESObjectREFR* a_refr);
		bool IsLeveledItemRefSwapped(const TESObjectREFR* a_refr) const;

	private:
		Manager() = default;
		~Manager() = default;

		Manager(const Manager&) = delete;
		Manager(Manager&&) = delete;
		Manager& operator=(const Manager&) = delete;
		Manager& operator=(Manager&&) = delete;

		void LoadForms();

		FormMap<SwapDataVec>& get_form_map(const std::string& a_str);
		static void           get_forms(const std::string& a_path, const std::string& a_str, FormMap<SwapDataVec>& a_map);
		void                  get_forms(const std::string& a_path, const std::string& a_str, const std::vector<FormIDStr>& a_conditionalIDs);

		// members
		FormMap<SwapDataVec>         allForms{};
		FormMap<SwapDataConditional> allFormsConditional{};
		FormMap<SwapDataVec>         spellForms{};
		FormMap<SwapDataConditional> spellFormsConditional{};
		FormMap<SwapDataVec>         factionForms{};
		FormMap<SwapDataConditional> factionFormsConditional{};
		FormMap<SwapDataVec>         equippableForms{};
		FormMap<SwapDataConditional> equippableFormsConditional{};
		FormMap<SwapDataVec>         itemForms{};
		FormMap<SwapDataConditional> itemFormsConditional{};


		std::unordered_set<std::uint32_t> swappedLeveledItemRefs{};

		bool           hasConflicts{ false };
		std::once_flag init{};
	};
}