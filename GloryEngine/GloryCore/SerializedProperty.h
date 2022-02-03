#pragma once
#include <any>
#include <string>
#include <typeindex>
#include "Debug.h"
#include "ResourceType.h"

namespace Glory
{
	class SerializedProperty
	{
	public:
		SerializedProperty();
		SerializedProperty(const std::string& name, void* pMember, size_t typeHash, uint32_t flags = 0);

		const std::string& Name() const;
		uint32_t Flags() const;
		void* MemberPointer() const;
		virtual size_t TypeHash() const;

	private:
		const std::string m_PropertyName;
		void* m_pMember;
		size_t m_TypeHash;
		uint32_t m_Flags;
	};

	template<typename T>
	class BasicTemplatedSerializedProperty : public SerializedProperty
	{
	public:
		BasicTemplatedSerializedProperty()
			: SerializedProperty("invalid", nullptr, ResourceType::GetHash<T>()) {}

		BasicTemplatedSerializedProperty(const std::string& name, T* pMember, uint32_t flags = 0)
			: SerializedProperty(name, pMember, ResourceType::GetHash<T>(), flags) {}
	};
}
