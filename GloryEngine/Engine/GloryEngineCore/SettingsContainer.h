#pragma once
#include <type_traits>

namespace Glory
{
	namespace Utils::Reflect
	{
		struct TypeData;
	}

	struct SettingsBase
	{
	public:
		SettingsBase(void* data): m_pSettingsData(data) {}

		void* operator*()
		{
			return m_pSettingsData;
		}

		virtual const Utils::Reflect::TypeData* GetType() const = 0;

	private:
		void* m_pSettingsData;
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
		SettingsContainer(): SettingsBase(&m_Data) {}

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
