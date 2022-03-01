#pragma once
#include <any>
#include <string>
#include <typeindex>
#include "Debug.h"
#include "ResourceType.h"
#include "Object.h"

namespace Glory
{
	enum SerializedType : size_t
	{
		ST_Value,
		ST_String,
		ST_Object,
		ST_Asset,
		ST_Array,
		ST_Struct,
	};

	class SerializedProperty
	{
	public:
		SerializedProperty();
		SerializedProperty(const std::string& name, void* pMember, size_t typeHash, uint32_t flags = 0);

		const std::string& Name() const;
		uint32_t Flags() const;
		void* MemberPointer() const;
		size_t TypeHash() const;
		size_t ElementTypeHash() const;
		size_t ArrayElementsCount();
		SerializedProperty GetArrayElementAt(size_t index);
		Object* ObjectReference() const;

	protected:
		const std::string m_PropertyName;
		void* m_pMember;
		size_t m_TypeHash;
		size_t m_ElementTypeHash;
		uint32_t m_Flags;
		size_t m_ElementByteSize;
		size_t m_ArrayElementsCount;
	};

	template<typename T>
	class BasicTemplatedSerializedProperty : public SerializedProperty
	{
	public:
		BasicTemplatedSerializedProperty()
			: SerializedProperty("invalid", nullptr, ResourceType::GetHash<T>())
		{
			m_ElementByteSize = sizeof(T);
			m_ElementTypeHash = ResourceType::GetHash<T>();
		}

		BasicTemplatedSerializedProperty(const std::string& name, uint32_t flags, T* pMember)
			: SerializedProperty(name, pMember, ResourceType::GetHash<T>(), flags)
		{
			m_ElementByteSize = sizeof(T);
			m_ElementTypeHash = ResourceType::GetHash<T>();
		}

		void Update(T* pMember)
		{
			m_pMember = pMember;
		}
	};

	template<class T>
	class AssetReferencePropertyTemplate : public SerializedProperty
	{
	public:
		AssetReferencePropertyTemplate()
			: SerializedProperty("invalid", nullptr, SerializedType::ST_Asset)
		{
			m_ElementTypeHash = ResourceType::GetHash<T>();
		}
		AssetReferencePropertyTemplate(const std::string& name, uint32_t flags, T** pMember)
			: SerializedProperty(name, pMember, SerializedType::ST_Asset, flags)
		{
			m_ElementTypeHash = ResourceType::GetHash<T>();
		}

		void Update(T** pMember)
		{
			m_pMember = pMember;
		}
	};

	//template<typename T>
	//class SimpleSerializedContainerProperty : public SerializedProperty
	//{
	//public:
	//	SimpleSerializedContainerProperty()
	//		: SerializedProperty("invalid", nullptr, ResourceType::GetHash<std::vector<SerializedProperty>>())
	//	{
	//		m_ElementByteSize = sizeof(T);
	//		m_ElementTypeHash = ResourceType::GetHash<T>();
	//	}
	//
	//	SimpleSerializedContainerProperty(const std::string& name, std::vector<T>* pMember, uint32_t flags = 0)
	//		: SerializedProperty(name, pMember, ResourceType::GetHash<std::vector<SerializedProperty>>(), flags)
	//	{
	//		m_ArrayElementsCount = pMember->size();
	//		for (size_t i = 0; i < m_ArrayElementsCount; i++)
	//		{
	//			m_SerializedArray.push_back(SerializedProperty("Element" + std::to_string(i), &pMember[i], ResourceType::GetHash<T>(), flags);
	//		}
	//	}
	//};
}
