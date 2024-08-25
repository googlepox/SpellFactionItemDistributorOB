#include "Hooks.h"
#include "Manager.h"

namespace SpellFactionItemDistributor
{
	struct LinkFormNPCImpl
	{
		static void AddForms(TESObjectREFR* a_ref, TESForm* formToAdd, UInt32 amount) {
			if (formToAdd) {
				TESNPC* npc = npc = dynamic_cast<TESNPC*>(a_ref);
				switch (formToAdd->GetFormType())
				{
				case (FormType::kFormType_Misc):
					a_ref->AddItem(formToAdd, &a_ref->baseExtraList, amount);
					break;
				case (FormType::kFormType_AlchemyItem):
					a_ref->AddItem(formToAdd, &a_ref->baseExtraList, amount);
					break;
				case (FormType::kFormType_SoulGem):
					a_ref->AddItem(formToAdd, &a_ref->baseExtraList, amount);
					break;
				case (FormType::kFormType_Apparatus):
					a_ref->AddItem(formToAdd, &a_ref->baseExtraList, amount);
					break;
				case (FormType::kFormType_Book):
					a_ref->AddItem(formToAdd, &a_ref->baseExtraList, amount);
					break;
				case (FormType::kFormType_Ingredient):
					a_ref->AddItem(formToAdd, &a_ref->baseExtraList, amount);
					break;
				case (FormType::kFormType_Key):
					a_ref->AddItem(formToAdd, &a_ref->baseExtraList, amount);
					break;
				case (FormType::kFormType_LeveledItem):
					a_ref->AddItem(formToAdd, &a_ref->baseExtraList, amount);
					break;
				case (FormType::kFormType_Armor):
					a_ref->AddItem(formToAdd, &a_ref->baseExtraList, amount);
					a_ref->Equip(formToAdd, 1, &a_ref->baseExtraList, 0);
					break;
				case (FormType::kFormType_Weapon):
					a_ref->AddItem(formToAdd, &a_ref->baseExtraList, amount);
					a_ref->Equip(formToAdd, 1, &a_ref->baseExtraList, 0);
					break;
				case (FormType::kFormType_Ammo):
					a_ref->AddItem(formToAdd, &a_ref->baseExtraList, amount);
					a_ref->Equip(formToAdd, amount, &a_ref->baseExtraList, 0);
					break;
				case (FormType::kFormType_Clothing):
					a_ref->AddItem(formToAdd, &a_ref->baseExtraList, amount);
					a_ref->Equip(formToAdd, 1, &a_ref->baseExtraList, 0);
					break;
				case (FormType::kFormType_Spell):
					ThisStdCall(0x46F350, &npc->spellList, formToAdd);
					break;
				case (FormType::kFormType_LeveledSpell):
					ThisStdCall(0x46F350, &npc->spellList, formToAdd);
					break;
				case (FormType::kFormType_Package):
					ThisStdCall(0x468380, &npc->aiForm, formToAdd);
					break;
				case (FormType::kFormType_Faction):
					ThisStdCall(0x4675E, &npc->actorBaseData, formToAdd, 1);
					break;
				default:
					break;
				}
			}
		}

		static void __fastcall LinkFormHook(TESObjectREFR* a_ref, void* edx)
		{
			if (const auto base = a_ref->baseForm) {
				Manager* manager = Manager::GetSingleton();
				manager->LoadFormsOnce();
				const auto& [ref, sfidResult ] = manager->GetSwapData(a_ref, base);
				auto seededRNG = SeedRNG(static_cast<std::uint32_t>(a_ref->refID));
				if (sfidResult.traits.chance != 100) {
					const auto rng = sfidResult.traits.trueRandom ? SeedRNG().Generate<std::uint32_t>(0, 100) :
						seededRNG.Generate<std::uint32_t>(0, 100);
					if (rng > sfidResult.traits.chance) {
						return;
					}
				}
				if (std::holds_alternative<UInt32>(sfidResult.formToAdd)) {
					_MESSAGE("single");
					const auto formToAdd = std::get<UInt32>(sfidResult.formToAdd);
					auto form = LookupFormByID(formToAdd);
					AddForms(a_ref, form, sfidResult.traits.amount);
				}
				else if (std::holds_alternative<std::unordered_set<UInt32>>(sfidResult.formToAdd)) {
					_MESSAGE("set");
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
		static inline std::uint32_t originalAddress;

		static void Install()
		{
			originalAddress = DetourVtable(0xA6FDE8, reinterpret_cast<UInt32>(LinkFormHook)); // kVtbl_Character_LinkForm
			_MESSAGE("Installed Character vtable hook");
		}
	};

	struct LinkFormCREAImpl
	{
		static void __fastcall LinkFormHook(TESObjectREFR* a_ref, void* edx)
		{
			if (const auto base = a_ref->baseForm) {
				
			}
			ThisStdCall(originalAddress, a_ref);
		}
		static inline std::uint32_t originalAddress;

		static void Install()
		{
			originalAddress = DetourVtable(0xA71160, reinterpret_cast<UInt32>(LinkFormHook)); // kVtbl_Creature_LinkForm
			_MESSAGE("Installed Creature vtable hook");
		}
	};

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
		LinkFormNPCImpl::Install();
		LinkFormCREAImpl::Install();
		_MESSAGE("Installed all vtable hooks");

	}
}
