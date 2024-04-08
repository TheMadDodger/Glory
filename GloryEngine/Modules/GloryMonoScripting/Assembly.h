#pragma once
#include <mono/jit/jit.h>
#include <string>
#include <map>
#include <vector>
#include <array>
#include <string>
#include <Glory.h>
#include <filesystem>

namespace Glory
{
	class ScriptingLib;

	enum class Visibility
	{
		VISIBILITY_PRIVATE = 1,
		VISIBILITY_PROTECTED_AND_INTERNAL,
		VISIBILITY_INTERNAL,
		VISIBILITY_PROTECTED,
		VISIBILITY_PROTECTED_OR_INTERNAL,
		VISIBILITY_PUBLIC,
	};

	struct AssemblyClassField
	{
	public:
		GLORY_API AssemblyClassField();
		GLORY_API AssemblyClassField(MonoClassField* pField);
		GLORY_API virtual ~AssemblyClassField();

	public:
		GLORY_API void SetValue(MonoObject* pObject, void* value) const;
		GLORY_API void GetValue(MonoObject* pObject, void* value) const;

		GLORY_API MonoClassField* ClassField() const;
		GLORY_API MonoType* FieldType() const;
		GLORY_API const char* Name() const;
		GLORY_API const Visibility& FieldVisibility() const;
		GLORY_API const char* TypeName() const;
		GLORY_API const int Size() const;
		GLORY_API const uint32_t TypeHash() const;
		GLORY_API const uint32_t ElementTypeHash() const;
		GLORY_API const bool IsStatic() const;

	private:
		MonoClassField* m_pMonoField;
		MonoType* m_pType;
		uint32_t m_Flags;
		const char* m_Name;
		Visibility m_Visibility;
		const char* m_TypeName;
		int m_Size;
		int m_SizeAllignment;
		uint32_t m_TypeHash;
		uint32_t m_ElementTypeHash;
		bool m_IsStatic;
	};

	struct AssemblyClass
	{
	public:
		GLORY_API AssemblyClass();
		GLORY_API AssemblyClass(const std::string& name, MonoClass* pClass);

		std::string m_Name;
		MonoClass* m_pClass;
		std::map<std::string, MonoMethod*> m_pMethods;

		GLORY_API MonoMethod* GetMethod(const std::string& name);
		GLORY_API const AssemblyClassField* GetField(const std::string& name) const;
		GLORY_API const AssemblyClassField* GetField(const size_t index) const;
		GLORY_API const size_t NumFields() const;

	private:
		MonoMethod* LoadMethod(const std::string& name);

		void LoadFields();

		std::vector<AssemblyClassField> m_Fields;
		std::map<std::string, size_t> m_NameToFieldIndex;
	};

	struct AssemblyNamespace
	{
		std::string m_Name;
		std::map<std::string, AssemblyClass> m_Classes;
	};

	class IMonoLibManager;
	class AssemblyDomain;

	class Assembly
	{
	public:
		enum AssemblyState
		{
			AS_NotLoaded,
			AS_Loading,
			AS_Loaded,
		};

	public:
		GLORY_API Assembly(AssemblyDomain* pDomain);
		virtual GLORY_API ~Assembly();

		GLORY_API AssemblyClass* GetClass(const std::string& namespaceName, const std::string& className);
		GLORY_API bool GetClass(const std::string& namespaceName, const std::string& className, AssemblyClass& c);

		GLORY_API bool Load(const ScriptingLib& lib, IMonoLibManager* pLibManager);
		GLORY_API bool Load(MonoImage* monoImage);
		GLORY_API void Unload(bool isReloading = false);
		GLORY_API MonoReflectionAssembly* GetReflectionAssembly() const;

		GLORY_API bool IsLoading() const
		{
			return m_State == AssemblyState::AS_Loading;
		}

		GLORY_API bool IsLoaded() const
		{
			return m_State == AssemblyState::AS_Loaded;
		}

		GLORY_API AssemblyDomain* GetDomain() const
		{
			return m_pDomain;
		}

		GLORY_API MonoAssembly* GetMonoAssembly() const
		{
			return m_pAssembly;
		}

		GLORY_API MonoImage* GetMonoImage()
		{
			return m_pImage;
		}

		GLORY_API const std::string& Name()
		{
			return m_Name;
		}

		GLORY_API const std::string& Location()
		{
			return m_Location;
		}

	private:
		bool LoadAssembly(const std::filesystem::path& assemblyPath);
		bool LoadAssemblyWithImage(const std::filesystem::path& assemblyPath);
		AssemblyClass* LoadClass(AssemblyNamespace* pNamespace, const std::string& className);

	private:
		friend class AssemblyDomain;
		AssemblyDomain* m_pDomain;
		MonoAssembly* m_pAssembly;
		MonoImage* m_pImage;
		std::map<std::string, AssemblyNamespace> m_Namespaces;
		IMonoLibManager* m_pLibManager;

		AssemblyState m_State = AssemblyState::AS_NotLoaded;
		bool m_Locked = false;

		mono_byte* m_DebugData;
		size_t m_DebugDataSize;

		std::string m_Name;
		std::string m_Location;
		bool m_Reloadable;
	};

	class Attributes
	{
	public:
		Attributes();
		~Attributes();

	private:
		std::map<std::string, MonoClass*> m_AttributeClasses;
	};
}
