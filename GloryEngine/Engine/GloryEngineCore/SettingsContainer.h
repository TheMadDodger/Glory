#pragma once
#include <type_traits>
#include <unordered_map>
#include <functional>
#include <filesystem>

#define SETTING_NAME(x) STRINGIZE(x)

namespace Glory
{
	namespace Utils::Reflect
	{
		struct TypeData;
	}

	struct SettingsBase
	{
	public:
		SettingsBase(void* data):
			m_pSettingsData(data) {}

		inline void* operator*()
		{
			return m_pSettingsData;
		}

		using Callback = std::function<void()>;

		virtual const Utils::Reflect::TypeData* GetType() const = 0;

		inline void RegisterChangeHandler(const std::string& prop, Callback handler)
		{
			m_ChangeHandlers.emplace(prop, handler);
		}

		inline void NotifyChange(const std::string& prop)
		{
			auto handlerIter = m_ChangeHandlers.find(prop);
			if (handlerIter == m_ChangeHandlers.end()) return;
			handlerIter->second();
		}

		inline void NotifyFullChange()
		{
			for (const auto& [name, callback] : m_ChangeHandlers)
				callback();
		}

		inline void InsertGroupBefore(std::string&& prop, std::string&& group)
		{
			m_Groups.emplace(std::move(prop), std::move(group));
		}

		inline std::string_view GetGroup(const std::string& prop) const
		{
			const auto iter = m_Groups.find(prop);
			return iter != m_Groups.end() ? iter->second : std::string_view{};
		}

	private:
		void* m_pSettingsData;
		std::unordered_map<std::string, Callback> m_ChangeHandlers;
		std::unordered_map<std::string, std::string> m_Groups;
	};

	template<typename T>
	concept ReflectableType = requires {
		T();
		T::GetTypeData();
	};

	template<ReflectableType Data>
	struct SettingsContainer final : public SettingsBase
	{
	public:
		SettingsContainer():
			SettingsBase(&m_Data) {}

		Data* operator->()
		{
			return &m_Data;
		}

		const Data* operator->() const
		{
			return &m_Data;
		}

		virtual const Utils::Reflect::TypeData* GetType() const override
		{
			return Data::GetTypeData();
		}

	private:
		Data m_Data;
	};
}
