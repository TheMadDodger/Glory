#pragma once
#include "ReflectGen.h"
#include "TypeData.h"
#include "Factory.h"
#include "ArrayTypes.h"
#include "Enum.h"
#include "CustomTypeHash.h"
#include <map>
#include <vector>

namespace GloryReflect
{
	class Reflect
	{
	public:
		template<typename T>
		static const TypeData* RegisterType(uint64_t flags = 0)
		{
			const TypeData* pTypeData = T::GetTypeData();
			RegisterType(pTypeData->TypeHash(), pTypeData, flags);
			m_pReflectInstance->m_pFactories.emplace(pTypeData->TypeHash(), new Factory<T>());
			RegisterArrayType<T>(pTypeData);
			return pTypeData;
		}

		template<typename T>
		static const TypeData* RegisterBasicType(const std::string& aliasName = "", uint64_t flags = 0)
		{
			const TypeData* pTypeData = RegisterBasicType(typeid(T), sizeof(T), aliasName, flags);
			m_pReflectInstance->m_pFactories.emplace(pTypeData->TypeHash(), new Factory<T>());
			RegisterArrayType<T>(pTypeData);
			return pTypeData;
		}

		template<typename T>
		static const TypeData* RegisterEnum(const std::string& aliasName = "", uint64_t flags = 0)
		{
			Enum<T>* pNewEnum = new Enum<T>();
			if (!pNewEnum->Valid())
				throw new std::exception("Enum is not a reflectable type!");

			const uint32_t typeHash = Reflect::Hash<T>();
			m_pReflectInstance->m_pEnumTypes.emplace(typeHash, pNewEnum);

			return RegisterEnumType(typeid(T).name(), typeHash, aliasName, flags);
		}

		static const TypeData* RegisterTemplatedType(const char* typeName, uint32_t typeHash, size_t size);

		static size_t TypeCount();
		static const TypeData* GetTyeData(uint32_t hash);
		static const TypeData* GetTyeData(const std::string& name);
		static const TypeData* GetTyeDataAt(size_t index);
		static const uint64_t GetTypeFlags(uint32_t hash);
		static void SetFieldFlags(const FieldData* pFieldData, uint64_t flags = 0);
		static uint64_t GetFieldFlags(const FieldData* pFieldData);
		static std::map<uint32_t, const TypeData*>::iterator GetTypeIterator();
		static std::map<uint32_t, const TypeData*>::iterator GetTypeIteratorEnd();

		static std::any CreateAsValue(uint32_t hash);
		static void* CreateAsPointer(uint32_t hash);
		static void CreateAsTemporary(uint32_t hash, std::function<void(void*)> callback);

		static Reflect* CreateReflectInstance();
		static void DestroyReflectInstance();
		static void SetReflectInstance(Reflect* pInstance);

		static void ResizeArray(void* pArrayAddress, uint32_t elementTypeHash, size_t newSize);
		static const size_t ArraySize(void* pArrayAddress, uint32_t elementTypeHash);
		static void* ElementAddress(void* pArrayAddress, uint32_t elementTypeHash, size_t index);

		static EnumType* GetEnumType(uint32_t hash);

		static uint32_t GetCustomTypeHash(uint32_t hash);

		static const FieldData* GetArrayElementData(const FieldData* pFieldData, size_t index);

		template<typename T>
		static uint32_t Hash()
		{
			return Hash(typeid(T));
		}

		static uint32_t Hash(const std::type_info& type);
		static uint32_t Hash(const char* str);

	private:
		Reflect();
		virtual ~Reflect();

		static void RegisterType(uint32_t hash, const TypeData* pTypeData, uint64_t flags = 0);
		static const TypeData* RegisterBasicType(const std::type_info& type, size_t size, const std::string& aliasName, uint64_t flags);
		static const TypeData* RegisterEnumType(const char* typeName, uint32_t enumTypeHash, const std::string& aliasName, uint64_t flags);

		static void Tokenize(std::string str, std::vector<std::string>& tokens, char separator = ',');

		template<typename T>
		static void RegisterArrayType(const TypeData* pTypeData)
		{
			const ArrayTypeBase* pArrayType = new ArrayType<T>();
			m_pReflectInstance->m_pArrayTypes.emplace(pTypeData->TypeHash(), pArrayType);
		}

	private:
		std::map<uint32_t, const TypeData*> m_pTypeDatas;
		std::vector<const TypeData*> m_pManagedTypeDatas;
		std::map<std::string, uint32_t> m_StringToTypeHash;
		std::map<uint32_t, uint64_t> m_DataTypeFlags;
		std::map<const FieldData*, uint64_t> m_FieldFlags;
		std::map<uint32_t, const FactoryBase*> m_pFactories;
		std::map<uint32_t, const ArrayTypeBase*> m_pArrayTypes;
		std::map<uint32_t, EnumType*> m_pEnumTypes;
		std::map<uint32_t, std::map<size_t, const FieldData>> m_ArrayElementFieldDatas;

		static Reflect* m_pReflectInstance;
		static bool m_InstanceOwned;

		static const char* BASIC_VALUE_NAME;
	};
}