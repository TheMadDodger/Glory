#include "AssemblyBinding.h"
#include "MonoAssetManager.h"
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <Debug.h>
#include <filesystem>
#include <fstream>
#include <mono/metadata/attrdefs.h>
#include <ResourceType.h>

#include <SerializedTypes.h>
#include <ModelData.h>
#include <ImageData.h>
#include <MaterialData.h>
#include <MaterialInstanceData.h>


namespace Glory
{
	std::map<std::string, size_t> m_MonoTypeToHash = {
		{"System.Single", ResourceType::GetHash<float>()},
		{"System.Double", ResourceType::GetHash<double>()},
		{"System.Int32", ResourceType::GetHash<int>()},
		{"System.Boolean", ResourceType::GetHash<bool>()},
		{"GloryEngine.Material", SerializedType::ST_Asset},
		{"GloryEngine.MaterialInstance", SerializedType::ST_Asset},
		{"GloryEngine.Model", SerializedType::ST_Asset},
		{"GloryEngine.Image", SerializedType::ST_Asset},
	};

	std::map<std::string, size_t> m_MonoTypeToElementHash = {
		{"GloryEngine.Material", ResourceType::GetHash<MaterialData>()},
		{"GloryEngine.MaterialInstance", ResourceType::GetHash<MaterialInstanceData>()},
		{"GloryEngine.Model", ResourceType::GetHash<ModelData>()},
		{"GloryEngine.Image", ResourceType::GetHash<ImageData>()},
	};

	AssemblyBinding::AssemblyBinding(const ScriptingLib& lib)
		: m_Lib(lib), m_pAssembly(nullptr), m_pImage(nullptr)
	{
	}

	AssemblyBinding::~AssemblyBinding()
	{
		m_pAssembly = nullptr;
		m_pImage = nullptr;
	}

	void AssemblyBinding::Initialize(MonoDomain* pDomain)
	{
		std::filesystem::path path = m_Lib.Location();
		const std::string& name = m_Lib.LibraryName();
		path.append(name);
		if (!std::filesystem::exists(path))
		{
			Debug::LogError("Missing assembly");
			return;
		}

		std::ifstream fileStream;
		fileStream.open(path.string(), std::ios::in | std::ios::ate | std::ios::binary);
		std::streampos size = fileStream.tellg();
		fileStream.seekg(0, std::ios::beg);
		char* data = new char[size];
		fileStream.read(data, size);
		fileStream.close();
		MonoImageOpenStatus status;
		m_pImage = mono_image_open_from_data_with_name(data, size, true, &status, false, path.string().c_str());
		if (m_pImage == nullptr) return;
		if (status != MONO_IMAGE_OK) return;

		m_pAssembly = mono_image_get_assembly(m_pImage);
		if (!m_pAssembly) m_pAssembly = mono_assembly_load_from_full(m_pImage, path.string().c_str(), &status, false);
		if (status != MONO_IMAGE_OK) return;
		mono_image_close(m_pImage);
		m_pImage = mono_assembly_get_image(m_pAssembly);
		delete[] data;

		if (m_Lib.LibraryName() == "GloryEngine.Core.dll")
		{
			MonoAssetManager::Initialize(m_pImage);
		}
	}

	void AssemblyBinding::Destroy()
	{
		m_Namespaces.clear();
		m_pAssembly = nullptr;
		m_pImage = nullptr;
	}

	MonoImage* AssemblyBinding::GetMonoImage()
	{
		return m_pImage;
	}

	AssemblyClass* AssemblyBinding::GetClass(const std::string& namespaceName, const std::string& className)
	{
		if (m_Namespaces.find(namespaceName) == m_Namespaces.end() || m_Namespaces[namespaceName].m_Classes.find(className) == m_Namespaces[namespaceName].m_Classes.end())
		{
			return LoadClass(namespaceName, className);
		}
		return &m_Namespaces[namespaceName].m_Classes[className];
	}

	bool AssemblyBinding::GetClass(const std::string& namespaceName, const std::string& className, AssemblyClass& c)
	{
		if (m_Namespaces.find(namespaceName) == m_Namespaces.end() || m_Namespaces[namespaceName].m_Classes.find(className) == m_Namespaces[namespaceName].m_Classes.end())
		{
			if (LoadClass(namespaceName, className) == nullptr) return false;
		}
		c = m_Namespaces[namespaceName].m_Classes[className];
		return true;
	}

	const std::string& AssemblyBinding::Name()
	{
		return m_Lib.LibraryName();
	}

	GLORY_API const std::string& AssemblyBinding::Location()
	{
		return m_Lib.Location();
	}

	AssemblyClass* AssemblyBinding::LoadClass(const std::string& namespaceName, const std::string& className)
	{
		MonoClass* pClass = mono_class_from_name(m_pImage, namespaceName.c_str(), className.c_str());
		if (pClass == nullptr)
		{
			Debug::LogError("Failed to load mono class");
			return nullptr;
		}

		if (mono_class_init(pClass) == false)
		{
			Debug::LogError("AssemblyBinding::LoadClass > Failed to initialize a MonoClass!");
			return nullptr;
		}

		m_Namespaces[namespaceName].m_Classes[className] = AssemblyClass(className, pClass);
		return &m_Namespaces[namespaceName].m_Classes[className];
	}

	AssemblyClass::AssemblyClass() : m_Name(""), m_pClass(nullptr) {}

	AssemblyClass::AssemblyClass(const std::string& name, MonoClass* pClass) : m_Name(name), m_pClass(pClass)
	{
		LoadFields();
	}

	MonoMethod* AssemblyClass::GetMethod(const std::string& name)
	{
		if (m_pMethods.find(name) == m_pMethods.end())
		{
			return LoadMethod(name);
		}
		return m_pMethods[name];
	}

	const AssemblyClassField* AssemblyClass::GetField(const std::string& name) const
	{
		if (m_NameToFieldIndex.find(name) == m_NameToFieldIndex.end()) return nullptr;
		const size_t& index = m_NameToFieldIndex.at(name);
		return &m_Fields[index];
	}

	const AssemblyClassField* AssemblyClass::GetField(size_t index) const
	{
		if (index >= m_Fields.size()) return nullptr;
		return &m_Fields[index];
	}

	const size_t AssemblyClass::NumFields() const
	{
		return m_Fields.size();
	}

	MonoMethod* AssemblyClass::LoadMethod(const std::string& name)
	{
		MonoMethodDesc* pMainFuncDesc = mono_method_desc_new(name.c_str(), false);
		MonoMethod* pMethod = mono_method_desc_search_in_class(pMainFuncDesc, m_pClass);
		mono_method_desc_free(pMainFuncDesc);
		return pMethod;
	}

	void AssemblyClass::LoadFields()
	{
		void* iter = NULL;
		MonoClassField* pField = nullptr;
		while ((pField = mono_class_get_fields(m_pClass, &iter)) != nullptr)
		{
			size_t index = m_Fields.size();
			m_Fields.push_back(AssemblyClassField(pField));
			m_NameToFieldIndex.emplace(m_Fields[index].Name(), index);

			//MonoCustomAttrInfo* pAttributeInfo = mono_custom_attrs_from_field(pClass, pField);
		}
	}

	Attributes::Attributes() {}
	Attributes::~Attributes() {}

	AssemblyClassField::AssemblyClassField() :
		m_pMonoField(NULL), m_pType(NULL), m_Name(NULL),
		m_Flags(NULL), m_Visibility(Visibility(0)),
		m_TypeName(NULL),
		m_SizeAllignment(0),
		m_TypeHash(0),
		m_ElementTypeHash(0),
		m_Size(0),
		m_IsStatic(false)
	{
	}
	AssemblyClassField::AssemblyClassField(MonoClassField* pField) :
		m_pMonoField(pField), m_pType(mono_field_get_type(pField)), m_Name(mono_field_get_name(pField)),
		m_Flags(mono_field_get_flags(pField)), m_Visibility(Visibility(m_Flags & MONO_FIELD_ATTR_FIELD_ACCESS_MASK)),
		m_TypeName(mono_type_get_name(m_pType)),
		m_SizeAllignment(0),
		m_Size(mono_type_size(m_pType, &m_SizeAllignment)),
		m_TypeHash(m_MonoTypeToHash[m_TypeName]),
		m_ElementTypeHash(m_MonoTypeToElementHash.find(m_TypeName) != m_MonoTypeToElementHash.end() ? m_MonoTypeToElementHash[m_TypeName] : m_TypeHash),
		m_IsStatic((m_Flags & MONO_FIELD_ATTR_STATIC) == MONO_FIELD_ATTR_STATIC)
	{
	}

	AssemblyClassField::~AssemblyClassField()
	{
	}

	void AssemblyClassField::SetValue(MonoObject* pObject, void* value) const
	{
		mono_field_set_value(pObject, m_pMonoField, value);
	}

	void AssemblyClassField::GetValue(MonoObject* pObject, void* value) const
	{
		mono_field_get_value(pObject, m_pMonoField, value);
	}

	MonoClassField* AssemblyClassField::ClassField() const
	{
		return m_pMonoField;
	}

	MonoType* AssemblyClassField::FieldType() const
	{
		return m_pType;
	}

	const char* AssemblyClassField::Name() const
	{
		return m_Name;
	}

	const Visibility& AssemblyClassField::FielddVisibility() const
	{
		return m_Visibility;
	}

	const char* AssemblyClassField::TypeName() const
	{
		return m_TypeName;
	}

	const int AssemblyClassField::Size() const
	{
		return m_Size;
	}

	const size_t AssemblyClassField::TypeHash() const
	{
		return m_TypeHash;
	}
	
	const size_t AssemblyClassField::ElementTypeHash() const
	{
		return m_ElementTypeHash;
	}

	const bool AssemblyClassField::IsStatic() const
	{
		return m_IsStatic;
	}
}
