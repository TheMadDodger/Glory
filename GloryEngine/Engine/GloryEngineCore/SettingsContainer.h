#pragma once
#include <type_traits>
#include <unordered_map>
#include <functional>
#include <filesystem>

#include "MacroHelpers.h"

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

		void* operator*()
		{
			return m_pSettingsData;
		}

		using Callback = std::function<void()>;

		virtual const Utils::Reflect::TypeData* GetType() const = 0;

		void RegisterChangeHandler(const std::string& prop, Callback handler)
		{
			m_ChangeHandlers.emplace(prop, handler);
		}

		void NotifyChange(const std::string& prop)
		{
			auto handlerIter = m_ChangeHandlers.find(prop);
			if (handlerIter == m_ChangeHandlers.end()) return;
			handlerIter->second();
		}

		void InsertGroupBefore(std::string&& prop, std::string&& group)
		{
			m_Groups.emplace(std::move(prop), std::move(group));
		}

		std::string_view GetGroup(const std::string& prop) const
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
