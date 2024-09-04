#include "Hooks.h"
#include "Manager.h"
#include "obse/GameForms.h"
#include "obse/GameObjects.h"

namespace SpellFactionItemDistributor
{

	static void AddToCache(TESObjectREFR* ref) {
		Manager* manager = Manager::GetSingleton();
		manager->AddToCache(ref);
	}

	static void AddMiscItem(TESObjectREFR* ref, TESForm* form, UInt32 amount) {
		ref->AddItem(form, nullptr, amount);
		AddToCache(ref);
	}

	static void AddEquipItem(TESObjectREFR* ref, TESForm* form, UInt32 amount) {
		ref->AddItem(form, nullptr, amount);
		ref->Equip(form, amount, &ref->baseExtraList, 0);
		AddToCache(ref);
	}

	static void AddLevItem(TESObjectREFR* ref, TESForm* form, UInt32 amount) {
		TESActorBase* npc = dynamic_cast<TESActorBase*>(ref->baseForm);
		SInt16 level = npc->actorBaseData.level;
		SInt16 maxLevel = npc->actorBaseData.maxLevel;
		SInt16 minLevel = npc->actorBaseData.minLevel;
		TESLeveledList* lev = dynamic_cast<TESLeveledList*>(form);
		short itr = amount;
		while (itr > 0) {
			TESForm* newForm = lev->CalcElement(level, true, maxLevel - minLevel);
			if (newForm) {
				ref->AddItem(newForm, nullptr, 1);
			}
			itr = itr - 1;
		}
	}

	static void AddSingleSpell(TESObjectREFR* ref, TESForm* form) {
		TESActorBase* npc = dynamic_cast<TESActorBase*>(ref->baseForm);
		SpellItem* spell = dynamic_cast<SpellItem*>(form);
		SpellListVisitor newVisitor = SpellListVisitor(&npc->spellList.spellList);
		TESSpellList::Entry* newSpell = (TESSpellList::Entry*)FormHeap_Allocate(sizeof(TESSpellList::Entry));
		newSpell->type = spell;
		newSpell->next = NULL;
		newVisitor.Append(newSpell);
	}

	static void AddLevSpell(TESObjectREFR* ref, TESForm* form) {
		TESActorBase* npc = dynamic_cast<TESActorBase*>(ref->baseForm);
		SInt16 level = npc->actorBaseData.level;
		SInt16 maxLevel = npc->actorBaseData.maxLevel;
		SInt16 minLevel = npc->actorBaseData.minLevel;
		TESLeveledList* lev = dynamic_cast<TESLeveledList*>(form);
		TESForm* newForm = lev->CalcElement(level, true, maxLevel - minLevel);
		SpellItem* spell = dynamic_cast<SpellItem*>(newForm);
		SpellListVisitor newVisitor = SpellListVisitor(&npc->spellList.spellList);
		TESSpellList::Entry* newSpell = (TESSpellList::Entry*)FormHeap_Allocate(sizeof(TESSpellList::Entry));
		newSpell->type = spell;
		newSpell->next = NULL;
		newVisitor.Append(newSpell);
	}

	static void AddToFaction(TESObjectREFR* ref, TESForm* form) {
		TESActorBase* npc = dynamic_cast<TESActorBase*>(ref->baseForm);
		TESFaction* faction = dynamic_cast<TESFaction*>(form);
		FactionListVisitor newVisitor = FactionListVisitor(&npc->actorBaseData.factionList);
		TESActorBaseData::FactionListData* newFactionData = (TESActorBaseData::FactionListData*)FormHeap_Allocate(sizeof(TESActorBaseData::FactionListData));
		newFactionData->faction = dynamic_cast<TESFaction*>(form);
		newFactionData->rank = 1;
		TESActorBaseData::FactionListEntry* newFaction = (TESActorBaseData::FactionListEntry*)FormHeap_Allocate(sizeof(TESActorBaseData::FactionListEntry));
		newFaction->data = newFactionData;
		newFaction->next = NULL;
		newVisitor.Append(newFaction);
	}

	static void AddPackage(TESObjectREFR* ref, TESForm* form) {
		TESActorBase* npc = dynamic_cast<TESActorBase*>(ref->baseForm);
		TESPackage* package = dynamic_cast<TESPackage*>(form);
		PackageListVisitor newVisitor = PackageListVisitor(&npc->aiForm.packageList);
		TESAIForm::PackageEntry* newPackageData = (TESAIForm::PackageEntry*)FormHeap_Allocate(sizeof(TESAIForm::PackageEntry));
		newPackageData->package = package;
		newPackageData->next = NULL;
		newVisitor.Append(newPackageData);
	}


	static void AddForms(TESObjectREFR* a_ref, TESForm* formToAdd, UInt32 amount) {
		if (formToAdd) {
			TESNPC* npc = dynamic_cast<TESNPC*>(a_ref);
			switch (formToAdd->GetFormType())
			{
			case (FormType::kFormType_Misc):
				AddMiscItem(a_ref, formToAdd, amount);
				break;
			case (FormType::kFormType_AlchemyItem):
				AddMiscItem(a_ref, formToAdd, amount);
				break;
			case (FormType::kFormType_SoulGem):
				AddMiscItem(a_ref, formToAdd, amount);
				break;
			case (FormType::kFormType_Apparatus):
				AddMiscItem(a_ref, formToAdd, amount);
				break;
			case (FormType::kFormType_Book):
				AddMiscItem(a_ref, formToAdd, amount);
				break;
			case (FormType::kFormType_Ingredient):
				AddMiscItem(a_ref, formToAdd, amount);
				break;
			case (FormType::kFormType_Key):
				AddMiscItem(a_ref, formToAdd, amount);
				break;
			case (FormType::kFormType_LeveledItem):
				AddLevItem(a_ref, formToAdd, amount);
				break;
			case (FormType::kFormType_Armor):
				AddEquipItem(a_ref, formToAdd, amount);
				break;
			case (FormType::kFormType_Weapon):
				AddEquipItem(a_ref, formToAdd, amount);
				break;
			case (FormType::kFormType_Ammo):
				AddEquipItem(a_ref, formToAdd, amount);
				break;
			case (FormType::kFormType_Clothing):
				AddMiscItem(a_ref, formToAdd, amount);
				break;
			case (FormType::kFormType_Spell):
				AddSingleSpell(a_ref, formToAdd);
				break;
			case (FormType::kFormType_LeveledSpell):
				AddSingleSpell(a_ref, formToAdd);
				break;
			case (FormType::kFormType_Package):
				AddPackage(a_ref, formToAdd);
				break;
			case (FormType::kFormType_Faction):
				AddToFaction(a_ref, formToAdd);
				break;
			default:
				break;
			}
		}
	}

	static void ProcessResult(SFIDResult sfidResult) {
		//const auto& [ref, sfidResult] = manager->GetSingleSwapData(a_ref, base, "Factions");
		const auto& [ref, swapData] = sfidResult;
		if (!ref || swapData.traits.amount == 0) {
			return;
		}
		auto seededRNG = SeedRNG(static_cast<std::uint32_t>(ref->refID));
		if (swapData.traits.chance != 100) {
			const auto rng = swapData.traits.trueRandom ? SeedRNG().Generate<std::uint32_t>(0, 100) :
				seededRNG.Generate<std::uint32_t>(0, 100);
			if (rng > swapData.traits.chance) {
				return;
			}
		}
		if (std::holds_alternative<UInt32>(swapData.formToAdd)) {
			const auto formToAdd = std::get<UInt32>(swapData.formToAdd);
			if (formToAdd == 0) {
				return;
			}
			auto form = LookupFormByID(formToAdd);
			AddForms(ref, form, swapData.traits.amount);
		}
		else if (std::holds_alternative<std::unordered_set<UInt32>>(swapData.formToAdd)) {
			const auto& set = std::get<std::unordered_set<UInt32>>(swapData.formToAdd);
			for (const auto& form : set) {
				TESForm* newForm = LookupFormByID(form);
				if (newForm) {
					AddForms(ref, newForm, swapData.traits.amount);
				}
			}
		}
	}

	static inline std::uint32_t originalAddress;

	static void __fastcall LinkFormHook(TESObjectREFR* a_ref, void* edx)
	{
		Manager* manager = Manager::GetSingleton();
		if (const auto base = a_ref->baseForm) {
			manager->LoadFormsOnce();
			std::vector<SFIDResult> resultVec = manager->GetAllSwapData(a_ref, base);
			for (SFIDResult result : resultVec) {
				manager->processedForms.emplace(a_ref->refID);
				ProcessResult(result);
			}
		}
		ThisStdCall(originalAddress, a_ref);
	}

	// Credits to lStewieAl
	[[nodiscard]] __declspec(noinline) UInt32 __stdcall DetourVtable(UInt32 addr, UInt32 dst)
	{
		UInt32 originalFunction = *(UInt32*)addr;
		SafeWrite32(addr, dst);
		return originalFunction;
	}

	void Install()
	{
		_MESSAGE("-HOOKS-");
		originalAddress = DetourVtable(0xA6FDE8, reinterpret_cast<UInt32>(LinkFormHook)); // kVtbl_Character_GenerateNiNode
		_MESSAGE("Installed Character vtable hook");

	}
}
