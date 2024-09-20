#include "SwapData.h"

#define DEGTORAD 0.01745329252f

constexpr float PI = static_cast<float>(3.1415926535897932);;
constexpr float HALF_PI = 0.5F * PI;
constexpr float TWO_PI = 2.0F * PI;

namespace SpellFactionItemDistributor
{
	namespace detail
	{
		MinMax<float> get_min_max(const std::string& a_str)
		{
			constexpr auto get_float = [](const std::string& str) {
				return string::to_num<float>(str);
				};

			if (const auto splitNum = string::split(a_str, R"(/)"); splitNum.size() > 1) {
				return { get_float(splitNum[0]), get_float(splitNum[1]) };
			}
			else {
				auto num = get_float(a_str);
				return { num, num };
			}
		}
	}

	Traits::Traits(const std::string& a_str)
	{
		if (dist::is_valid_entry(a_str)) {
			if (a_str.contains("chance")) {
				if (a_str.contains("R")) {
					trueRandom = true;
				}
				if (srell::cmatch match; srell::regex_search(a_str.c_str(), match, genericRegex)) {
					chance = string::to_num<std::uint32_t>(match[1].str());
				}
			}

			if (a_str.contains("amount")) {
				if (srell::cmatch match; srell::regex_search(a_str.c_str(), match, genericRegex)) {
					amount = string::to_num<std::uint32_t>(match[1].str());
				}
			}

			if (a_str.contains("remove")) {
				if (srell::cmatch match; srell::regex_search(a_str.c_str(), match, genericRegex)) {
					remove = true;
				}
			}
		}
	}

	DistributeData::DistributeData(const Input& a_input) :
		formToAdd(a_input.formStr),
		traits(a_input.traitsStr),
		record(a_input.record),
		path(a_input.path)
	{

	}

	DistributeRecordData::DistributeRecordData()
	{
		formIDSet = static_cast<std::uint32_t>(0);
	}

	DistributeRecordData::DistributeRecordData(FormIDOrSet a_id, const Input& a_input, FormIDOrSet a_baseId) :
		formIDSet(std::move(a_id)),
		DistributeData(a_input),
		formIDSetBase(std::move(a_baseId))
	{}

	std::uint32_t DistributeData::GetFormID(const std::string& a_str)
	{
		if (a_str == "ALL") {
			return 0xFFFFFFFF;
		}
		constexpr auto lookup_formID = [](std::uint32_t a_refID, const std::string& modName) -> std::uint32_t
			{
				const auto modIdx = (*g_dataHandler)->GetModIndex(modName.c_str());
				return modIdx == 0xFF ? 0 : (a_refID & 0xFFFFFF) | modIdx << 24;
			};

		if (const auto splitID = string::split(a_str, "~"); splitID.size() == 2) {
			const auto  formID = string::to_num<std::uint32_t>(splitID[0], true);
			const auto& modName = splitID[1];
			return lookup_formID(formID, modName);
		}
		if (string::is_only_hex(a_str, true))
		{
			if (const auto form = LookupFormByID(string::to_num<std::uint32_t>(a_str, true))) {
				return form->refID;
			}
		}
		/*if (const auto form = GetFormByID(a_str.c_str())) {
			return form->refID;
		} */
		return 0;
	}

	FormIDOrSet DistributeRecordData::GetSwapFormID(const std::string& a_str)
	{
		if (a_str.contains(",")) {
			FormIDSet  set;
			const auto IDStrs = string::split(a_str, ",");
			set.reserve(IDStrs.size());
			for (auto& IDStr : IDStrs) {
				if (auto formID = GetFormID(IDStr); formID != 0) {
					set.emplace(formID);
				}
				else {
					_ERROR("\t\t\tfailed to process %s (SWAP formID not found)", IDStr.c_str());
				}
			}
			return set;
		}
		else {
			return GetFormID(a_str);
		}
	}

	bool DistributeRecordData::GetSwapBase(const TESObjectREFR* a_ref) const
	{

		if (const auto formID = std::get_if<UInt32>(&formIDSet); formID) {
			return true;
		}
		else {
			return false;

			// return random element from set
			/*
			auto& set = std::get<FormIDSet>(formIDSet);
			
			const auto setEnd = std::distance(set.begin(), set.end()) - 1;
			const auto randIt = traits.trueRandom ? SeedRNG().Generate<std::int64_t>(0, setEnd) :
				seededRNG.Generate<std::int64_t>(0, setEnd);

			return static_cast<TESObjectREFR*>(LookupFormByID(*std::next(set.begin(), randIt)));
			*/
		}
	}

	void DistributeRecordData::GetFormsAll(const std::string& a_path, const std::string& a_str, std::function<void(std::uint32_t, DistributeRecordData&)> a_func)
	{
		constexpr auto swap_empty = [](const FormIDOrSet& a_set) {
			if (const auto formID = std::get_if<UInt32>(&a_set); formID) {
				return *formID == 0;
			}
			else {
				return std::get<FormIDSet>(a_set).empty();
			}
			};

		const auto formPair = string::split(a_str, "|");
		
		if (formPair[0] == "ALL") {
			UInt32 baseFormID;
			if (const auto swapFormID = GetSwapFormID(formPair[1]); !swap_empty(swapFormID)) {

				const Input input(
					swapFormID, // items
					formPair.size() > 2 ? formPair[2] : std::string{},  // traits
					a_str,
					a_path);
				DistributeRecordData swapData(swapFormID, input, baseFormID);
				a_func(baseFormID, swapData);
			}
			else {
				_ERROR("\t\t\tfailed to process %s (SWAP formID not found)", a_str.c_str());
			}
		}
	}

	void DistributeRecordData::GetForms(const std::string& a_path, const std::string& a_str, std::function<void(std::uint32_t, DistributeRecordData&)> a_func)
	{
		constexpr auto swap_empty = [](const FormIDOrSet& a_set) {
			if (const auto formID = std::get_if<UInt32>(&a_set); formID) {
				return *formID == 0;
			}
			else {
				return std::get<FormIDSet>(a_set).empty();
			}
			};

		const auto formPair = string::split(a_str, "|");

		if (const auto baseFormID = GetFormID(formPair[0]); baseFormID != 0) {
			if (const auto swapFormID = GetSwapFormID(formPair[1]); !swap_empty(swapFormID)) {
				const Input input(
					swapFormID, // items
					formPair.size() > 2 ? formPair[2] : std::string{},  // traits
					a_str,
					a_path);
				DistributeRecordData swapData(swapFormID, input, baseFormID);
				a_func(baseFormID, swapData);
			}
			else {
				_ERROR("\t\t\tfailed to process %s (SWAP formID not found)", a_str.c_str());
			}
		}
		else if (formPair[0] == "ALL") {
			const auto baseFormID = GetFormID(formPair[0]);
			if (const auto swapFormID = GetSwapFormID(formPair[1]); !swap_empty(swapFormID)) {
				const Input input(
					swapFormID, // items
					formPair.size() > 2 ? formPair[2] : std::string{},  // traits
					a_str,
					a_path);
				DistributeRecordData swapData(swapFormID, input, baseFormID);
				a_func(baseFormID, swapData);
			}
			else {
				_ERROR("\t\t\tfailed to process %s (SWAP formID not found)", a_str.c_str());
			}
		}
		else {
			_ERROR("\t\t\tfailed to process %s (BASE formID not found)", a_str.c_str());
		}
	}
}