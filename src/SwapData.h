#pragma once

#include "Defs.h"

namespace SpellFactionItemDistributor
{
	inline srell::regex genericRegex{ R"(\((.*?)\))" };

	struct Traits
	{
		Traits() = default;
		explicit Traits(const std::string& a_str);

		// members
		bool          trueRandom{ false };
		std::uint32_t chance{ 100 };
		std::uint32_t amount{ 1 };
	};

	class SFID
	{
	public:

	};

	class Transform
	{
	public:
		Transform() = default;
		explicit Transform(const std::string& a_str);

		void SetTransform(TESObjectREFR* a_refr) const;
		bool IsValid() const;
		bool operator==(Transform const& a_rhs) const
		{
			return true;
		}

	private:
		[[nodiscard]] static MinMax<float>         get_scale_from_string(const std::string& a_str);

		struct Input
		{
			bool       trueRandom{ false };
			std::uint32_t refSeed{ 0 };

			bool  clamp{ false };
			float clampMin{ 0.0f };
			float clampMax{ 0.0f };
		};

		static float        get_random_value(const Input& a_input, float a_min, float a_max);

		// members
		std::optional<MinMax<float>>         refScale{ std::nullopt };
		std::optional<bool>         refDisable{ std::nullopt };

		bool useTrueRandom{ false };

		static inline srell::regex transformRegex{ R"(\((.*?),(.*?),(.*?)\))" };
		static inline srell::regex stringRegex{ R"(,\s*(?![^()]*\)))" };

		friend class TransformData;
	};

	class TransformData
	{
	public:
		struct Input
		{
			FormIDOrSet formStr;
			std::string traitsStr;
			std::string record;
			std::string path;
		};

		TransformData() = default;
		explicit TransformData(const Input& a_input);

		[[nodiscard]] static std::uint32_t GetFormID(const std::string& a_str);
		bool                            IsTransformValid(const TESObjectREFR* a_ref) const;

		static void GetTransforms(const std::string& a_path, const std::string& a_str, std::function<void(std::uint32_t, TransformData&)> a_func);

		// members
		Transform transform{};
		FormIDOrSet formToAdd{};
		Traits    traits{};

		std::string record{};
		std::string path{};
	};

	class SwapData : public TransformData
	{
	public:
		SwapData();
		SwapData(FormIDOrSet a_id, const Input& a_input, FormIDOrSet a_baseId);

		[[nodiscard]] static FormIDOrSet GetSwapFormID(const std::string& a_str);
		bool GetSwapBase(const TESObjectREFR* a_ref) const;

		static void GetFormsAll(const std::string& a_path, const std::string& a_str, std::function<void(std::uint32_t, SwapData&)> a_func);

		static void GetForms(const std::string& a_path, const std::string& a_str, std::function<void(std::uint32_t, SwapData&)> a_func);

		// members
		FormIDOrSet formIDSet{};
		FormIDOrSet formIDSetBase{};
	};

	using SwapDataVec = std::vector<SwapData>;
	using TransformDataVec = std::vector<TransformData>;

	using SwapDataConditional = std::unordered_map<FormIDStr, SwapDataVec>;
	using TransformDataConditional = std::unordered_map<FormIDStr, TransformDataVec>;

	using TransformResult = std::optional<Transform>;


	using SFIDResult = std::pair<TESObjectREFR*, SwapData>;
}