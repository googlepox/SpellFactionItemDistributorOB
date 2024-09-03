#include "Hooks.h"
#include "Manager.h"
#include "obse/GameForms.h"
#include "obse/GameObjects.h"

namespace SpellFactionItemDistributor
{

	static void AddToCache(TESObjectREFR* ref) {
		Manager* manager = Manager::GetSingleton();
		manager->AddToCache(ref->refID);
		std::string formString = std::to_string(ref->refID) + "\n";
		std::fstream idCache;
		idCache.open("SFIDCache.txt", std::ios_base::binary | std::ios_base::app);
		idCache << std::hex << ref->refID;
		idCache << '\n';
		idCache.close();
	}

	static void AddMiscItem(TESObjectREFR* ref, TESForm* form, UInt32 amount) {
		ref->AddItem(form, nullptr, amount);
	}

	static void AddEquipItem(TESObjectREFR* ref, TESForm* form, UInt32 amount) {
		ref->AddItem(form, nullptr, amount);
		ref->Equip(form, amount, &ref->baseExtraList, 0);
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
		if (newForm) {
			AddSingleSpell(ref, newForm);
		}

		SpellItem* spell = dynamic_cast<SpellItem*>(form);
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
			AddToCache(a_ref);
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

	static inline std::uint32_t originalAddress;

	static void __fastcall LinkFormHook(TESObjectREFR* a_ref, void* edx)
	{
		bool skip = false;
		if (const auto base = a_ref->baseForm) {
			Manager* manager = Manager::GetSingleton();
			manager->LoadFormsOnce();
			const auto& [ref, sfidResult] = manager->GetSwapData(a_ref, base);
			if (!ref || sfidResult.traits.amount == 0) {
				skip = true;
			}
			auto seededRNG = SeedRNG(static_cast<std::uint32_t>(a_ref->refID));
			if (sfidResult.traits.chance != 100 && !skip) {
				const auto rng = sfidResult.traits.trueRandom ? SeedRNG().Generate<std::uint32_t>(0, 100) :
					seededRNG.Generate<std::uint32_t>(0, 100);
				if (rng > sfidResult.traits.chance) {
					skip = true;
				}
			}
			if (!skip && std::holds_alternative<UInt32>(sfidResult.formToAdd)) {
				const auto formToAdd = std::get<UInt32>(sfidResult.formToAdd);
				if (formToAdd == 0) {
					skip = true;
				}
				auto form = LookupFormByID(formToAdd);
				//_MESSAGE("\t\tadding %u %u to %u", sfidResult.traits.amount, formToAdd, a_ref->refID);
				AddForms(a_ref, form, sfidResult.traits.amount);
			}
			else if (!skip && std::holds_alternative<std::unordered_set<UInt32>>(sfidResult.formToAdd)) {
				const auto& set = std::get<std::unordered_set<UInt32>>(sfidResult.formToAdd);
				for (const auto& form : set) {
					TESForm* newForm = LookupFormByID(form);
					if (newForm) {
						AddForms(a_ref, newForm, sfidResult.traits.amount);
					}
				}
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
		originalAddress = DetourVtable(0xA6FDE8, reinterpret_cast<UInt32>(LinkFormHook)); // kVtbl_Character_LinkForm
		_MESSAGE("Installed Character vtable hook");

	}
}
