#pragma once
#include <mono/jit/jit.h>
#include <string>
#include <map>
#include <vector>
#include <array>
#include <string>
#include <Glory.h>
#include <ScriptExtensions.h>

namespace Glory
{
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
		AssemblyClassField();
		AssemblyClassField(MonoClassField* pField);
		virtual ~AssemblyClassField();

	public:
		void SetValue(MonoObject* pObject, void* value) const;
		void GetValue(MonoObject* pObject, void* value) const;

		MonoClassField* ClassField() const;
		MonoType* FieldType() const;
		const char* Name() const;
		const Visibility& FielddVisibility() const;
		const char* TypeName() const;
		const int Size() const;
		const size_t TypeHash() const;
		const bool IsStatic() const;

	private:
		MonoClassField* m_pMonoField;
		MonoType* m_pType;
		uint32_t m_Flags;
		const char* m_Name;
		Visibility m_Visibility;
		const char* m_TypeName;
		int m_Size;
		int m_SizeAllignment;
		size_t m_TypeHash;
		bool m_IsStatic;
	};

	struct AssemblyClass
	{
	public:
		AssemblyClass();
		AssemblyClass(const std::string& name, MonoClass* pClass);

		std::string m_Name;
		MonoClass* m_pClass;
		std::map<std::string, MonoMethod*> m_pMethods;

		MonoMethod* GetMethod(const std::string& name);
		const AssemblyClassField* GetField(const std::string& name) const;
		const AssemblyClassField* GetField(const size_t index) const;
		const size_t NumFields() const;

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

	class AssemblyBinding
	{
	public:
		GLORY_API AssemblyBinding(const ScriptingLib& name);
		virtual GLORY_API ~AssemblyBinding();

		GLORY_API MonoImage* GetMonoImage();
		GLORY_API AssemblyClass* GetClass(const std::string& namespaceName, const std::string& className);
		GLORY_API bool GetClass(const std::string& namespaceName, const std::string& className, AssemblyClass& c);

		GLORY_API const std::string& Name();
		GLORY_API const std::string& Location();

	private:
		AssemblyClass* LoadClass(const std::string& namespaceName, const std::string& className);

		void Initialize(MonoDomain* pDomain);
		void Destroy();

	private:
		friend class MonoLibManager;
		const ScriptingLib m_Lib;
		MonoAssembly* m_pAssembly;
		MonoImage* m_pImage;
		std::map<std::string, AssemblyNamespace> m_Namespaces;
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
