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
		SerializedProperty(UUID objectUUID, const std::string& name, uint32_t typeHash, void* pMember, uint32_t flags = 0);
		SerializedProperty(UUID objectUUID, const std::string& name, uint32_t typeHash, uint32_t elementTypeHash, void* pMember, uint32_t flags = 0);
		virtual ~SerializedProperty();

		const std::string& Name() const;
		uint32_t Flags() const;
		void* MemberPointer() const;
		uint32_t TypeHash() const;
		uint32_t ElementTypeHash() const;
		Object* ObjectReference() const;

	protected:
		const std::string m_PropertyName;
		void* m_pMember;
		uint32_t m_TypeHash;
		uint32_t m_ElementTypeHash;
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
