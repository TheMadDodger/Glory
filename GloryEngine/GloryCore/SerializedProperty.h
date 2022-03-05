#pragma once
#include <any>
#include <string>
#include <typeindex>
#include "Debug.h"
#include "ResourceType.h"
#include "Object.h"
#include "SerializedTypes.h"

namespace Glory
{
	class SerializedProperty
	{
	public:
		SerializedProperty();
		SerializedProperty(UUID objectUUID, const std::string& name, size_t typeHash, void* pMember, uint32_t flags = 0);

		const std::string& Name() const;
		uint32_t Flags() const;
		void* MemberPointer() const;
		size_t TypeHash() const;
		size_t ElementTypeHash() const;
		Object* ObjectReference() const;

	protected:
		const std::string m_PropertyName;
		void* m_pMember;
		size_t m_TypeHash;
		size_t m_ElementTypeHash;
		uint32_t m_Flags;
		UUID m_ObjectUUID;
	};

	template<typename T>
	class BasicTemplatedSerializedProperty : public SerializedProperty
	{
	public:
		BasicTemplatedSerializedProperty()
			: SerializedProperty(UUID(), "invalid", ResourceType::GetHash<T>(), nullptr)
		{
			//m_ElementByteSize = sizeof(T);
			m_ElementTypeHash = ResourceType::GetHash<T>();
		}

		BasicTemplatedSerializedProperty(UUID objectUUID, const std::string& name, T* pMember, uint32_t flags)
			: SerializedProperty(objectUUID, name, ResourceType::GetHash<T>(), pMember, flags)
		{
			//m_ElementByteSize = sizeof(T);
			m_ElementTypeHash = ResourceType::GetHash<T>();
		}

		void Update(T* pMember)
		{
			m_pMember = pMember;
		}
	};
}
