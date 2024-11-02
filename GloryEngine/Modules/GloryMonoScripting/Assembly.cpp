#include "Assembly.h"
#include "AssemblyDomain.h"
#include "IMonoLibManager.h"
#include "MonoManager.h"
#include "GloryMonoScipting.h"

#include <Debug.h>
#include <filesystem>
#include <fstream>
#include <ResourceType.h>
#include <SerializedTypes.h>
#include <ModelData.h>
#include <ImageData.h>
#include <MaterialData.h>
#include <MaterialInstanceData.h>
#include <PrefabData.h>
#include <AudioData.h>

#include <glm/detail/type_quat.hpp>

#include <mono/metadata/mono-debug.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/attrdefs.h>

#define ENGINE MonoManager::Instance()->Module()->GetEngine()

namespace Glory
{
	std::map<std::string, uint32_t> MonoTypeToHash = {
		{"System.Single", ResourceTypes::GetHash<float>()},
		{"System.Double", ResourceTypes::GetHash<double>()},
		{"System.Int32", ResourceTypes::GetHash<int>()},
		{"System.Boolean", ResourceTypes::GetHash<bool>()},
		{"GloryEngine.Material", SerializedType::ST_Asset},
		{"GloryEngine.MaterialInstance", SerializedType::ST_Asset},
		{"GloryEngine.Model", SerializedType::ST_Asset},
		{"GloryEngine.Image", SerializedType::ST_Asset},
		{"GloryEngine.Prefab", SerializedType::ST_Asset},
		{"GloryEngine.Audio", SerializedType::ST_Asset},
		{"GloryEngine.Quaternion", ResourceTypes::GetHash<glm::quat>()},
		{"GloryEngine.Vector2", ResourceTypes::GetHash<glm::vec2>()},
		{"GloryEngine.Vector3", ResourceTypes::GetHash<glm::vec3>()},
		{"GloryEngine.Vector4", ResourceTypes::GetHash<glm::vec4>()},
		{"GloryEngine.SceneManagement.SceneObject", SerializedType::ST_Object},
	};

	std::map<std::string, uint32_t> MonoTypeToElementHash = {
		{"GloryEngine.Material", ResourceTypes::GetHash<MaterialData>()},
		{"GloryEngine.MaterialInstance", ResourceTypes::GetHash<MaterialInstanceData>()},
		{"GloryEngine.Model", ResourceTypes::GetHash<ModelData>()},
		{"GloryEngine.Image", ResourceTypes::GetHash<ImageData>()},
		{"GloryEngine.Prefab", ResourceTypes::GetHash<PrefabData>()},
		{"GloryEngine.Audio", ResourceTypes::GetHash<AudioData>()},
	};

	Assembly::Assembly(AssemblyDomain* pDomain)
		: m_pDomain(pDomain), m_pAssembly(nullptr), m_pImage(nullptr), m_pLibManager(nullptr), m_Reloadable(false), m_DebugData(nullptr), m_DebugDataSize(0)
	{
	}

	Assembly::~Assembly()
	{
		if (!m_DebugData) return;

		delete[] m_DebugData;
		m_DebugData = nullptr;
		m_DebugDataSize = 0;
	}

	AssemblyClass* Assembly::GetClass(const std::string& namespaceName, const std::string& className)
	{
		auto namespaceItor = m_Namespaces.find(namespaceName);
		if (namespaceItor == m_Namespaces.end())
		{
			auto newItor = m_Namespaces.emplace(namespaceName, AssemblyNamespace{ namespaceName }).first;
			return LoadClass(&newItor->second, className);
		}
		auto classItor = namespaceItor->second.m_Classes.find(className);
		if (classItor == namespaceItor->second.m_Classes.end())
		{
			return LoadClass(&namespaceItor->second, className);
		}

		return &classItor->second;
	}

	bool Assembly::GetClass(const std::string& namespaceName, const std::string& className, AssemblyClass& c)
	{
		auto namespaceItor = m_Namespaces.find(namespaceName);
		if (namespaceItor == m_Namespaces.end())
		{
			auto newItor = m_Namespaces.emplace(namespaceName, AssemblyNamespace{ namespaceName }).first;
			if (LoadClass(&newItor->second, className) == nullptr) return false;
		}
		auto classItor = namespaceItor->second.m_Classes.find(className);
		if (classItor == namespaceItor->second.m_Classes.end())
		{
			if (LoadClass(&namespaceItor->second, className) == nullptr) return false;
		}
		c = classItor->second;
		return true;
	}

	AssemblyClass* Assembly::LoadClass(AssemblyNamespace* pNamespace, const std::string& className)
	{
		MonoClass* pClass = mono_class_from_name(m_pImage, pNamespace->m_Name.c_str(), className.c_str());
		if (pClass == nullptr)
		{
			ENGINE->GetDebug().LogError("Failed to load mono class");
			return nullptr;
		}

		if (mono_class_init(pClass) == false)
		{
			ENGINE->GetDebug().LogError("Assembly::LoadClass > Failed to initialize a MonoClass!");
			return nullptr;
		}

		auto itor = pNamespace->m_Classes.emplace(className, AssemblyClass{ className, pClass }).first;
		return &itor->second;
	}

	AssemblyClass::AssemblyClass() : m_Name(""), m_pClass(nullptr) {}

	AssemblyClass::AssemblyClass(const std::string& name, MonoClass* pClass) : m_Name(name), m_pClass(pClass)
	{
		LoadFields();
	}

	MonoMethod* AssemblyClass::GetMethod(const std::string& name)
	{
		auto itor = m_pMethods.find(name);
		if (itor == m_pMethods.end())
			return LoadMethod(name);

		return itor->second;
	}

	const AssemblyClassField* AssemblyClass::GetField(const std::string& name) const
	{
		auto itor = m_NameToFieldIndex.find(name);
		if (itor == m_NameToFieldIndex.end()) return nullptr;
		const size_t& index = itor->second;
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
		m_TypeHash(MonoTypeToHash[m_TypeName]),
		m_ElementTypeHash(MonoTypeToElementHash.find(m_TypeName) != MonoTypeToElementHash.end() ? MonoTypeToElementHash[m_TypeName] : m_TypeHash),
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

	const Visibility& AssemblyClassField::FieldVisibility() const
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

	const uint32_t AssemblyClassField::TypeHash() const
	{
		return m_TypeHash;
	}

	const uint32_t AssemblyClassField::ElementTypeHash() const
	{
		return m_ElementTypeHash;
	}

	const bool AssemblyClassField::IsStatic() const
	{
		return m_IsStatic;
	}

    bool Assembly::Load(const ScriptingLib& lib, IMonoLibManager* pLibManager)
    {
        if (IsLoaded())
            return false;

		m_State = AssemblyState::AS_Loading;

		std::filesystem::path path = lib.Location();
		const std::string& name = lib.LibraryName();
		path.append(name);
        if (!std::filesystem::exists(path))
        {
			ENGINE->GetDebug().LogError("Failed to open assembly, file \"" + path.string() + "\" not found!");
            return true;
        }

		if (!lib.Reloadable() && !LoadAssembly(path)) return false;
		else if (!LoadAssemblyWithImage(path)) return false;

		m_Name = lib.LibraryName();
		m_Location = lib.Location();
		m_Reloadable = lib.Reloadable();
		m_pLibManager = pLibManager;
		Engine* pEngine = MonoManager::Instance()->Module()->GetEngine();
		if (m_pLibManager) m_pLibManager->Initialize(pEngine, this);
		m_State = AssemblyState::AS_Loaded;
		return true;
    }

    bool Assembly::Load(MonoImage* monoImage)
    {
        if (IsLoaded())
            return false;

        Unload();

		m_State = AssemblyState::AS_Loading;

        m_pAssembly = mono_image_get_assembly(monoImage);
		if (m_pAssembly == nullptr) return false;

        m_pImage = monoImage;
        //m_IsDependency = true;

		Engine* pEngine = MonoManager::Instance()->Module()->GetEngine();
		if (m_pLibManager) m_pLibManager->Initialize(pEngine, this);
		m_State = AssemblyState::AS_Loaded;
        return true;
    }

    void Assembly::Unload(bool isReloading)
    {
        if (!IsLoaded())
            return;

		if (m_pImage && !isReloading)
		{
			mono_assembly_close(m_pAssembly);
			mono_debug_close_image(m_pImage);
		}

		m_Locked = false;
        m_pAssembly = nullptr;
        m_pImage = nullptr;
		m_Name = "";
		m_Location = "";
		m_Reloadable = false;
		m_State = AssemblyState::AS_NotLoaded;

		Engine* pEngine = MonoManager::Instance()->Module()->GetEngine();
		if (m_pLibManager) m_pLibManager->Cleanup(pEngine);
		m_pLibManager = nullptr;
		m_Namespaces.clear();

		if (m_DebugData)
		{
			delete[] m_DebugData;
			m_DebugData = nullptr;
			m_DebugDataSize = 0;
		}
    }

    MonoReflectionAssembly* Assembly::GetReflectionAssembly() const
    {
        if (!m_pAssembly)
			return nullptr;
        return mono_assembly_get_object(mono_domain_get(), m_pAssembly);
    }

    bool Assembly::LoadAssembly(const std::filesystem::path& assemblyPath)
    {
		m_Locked = true;

        MonoAssembly* pAssembly = mono_domain_assembly_open(m_pDomain->GetMonoDomain(), assemblyPath.string().c_str());
        if (!pAssembly) return false;

        MonoImage* pAssemblyImage = mono_assembly_get_image(pAssembly);
        if (!pAssembly)
        {
            mono_assembly_close(pAssembly);
            return false;
        }

        m_pAssembly = pAssembly;
        m_pImage = pAssemblyImage;

        return true;
    }

    bool Assembly::LoadAssemblyWithImage(const std::filesystem::path& assemblyPath)
    {
		m_Locked = true;

		const std::string assemblyPathStr = assemblyPath.string();

		std::ifstream assemblyFileStream;
		assemblyFileStream.open(assemblyPathStr, std::ios::in | std::ios::ate | std::ios::binary);
		std::streampos size = assemblyFileStream.tellg();
		assemblyFileStream.seekg(0, std::ios::beg);

		char* data = new char[size];
		assemblyFileStream.read(data, size);
		assemblyFileStream.close();

        /* Load image */
        MonoImageOpenStatus status;
        MonoImage* pAssemblyImage = mono_image_open_from_data_with_name(data, (uint32_t)size, true, &status, false, assemblyPathStr.c_str());
		delete[] data;
        if (status != MONO_IMAGE_OK || pAssemblyImage == nullptr)
        {
			std::stringstream log;
			log << "Mono assembly image is invalid at " << assemblyPath;
			ENGINE->GetDebug().LogError(log.str());
            return false;
        }

        /* Load assembly */
        MonoAssembly* pAssembly = mono_assembly_load_from_full(pAssemblyImage, assemblyPathStr.c_str(), &status, false);
        mono_image_close(pAssemblyImage);
        if (status != MONO_IMAGE_OK || pAssembly == nullptr)
        {
			std::stringstream log;
			log << "Mono assembly image is corrupted at " << assemblyPath;
			ENGINE->GetDebug().LogError(log.str());
            return false;
        }

		/* Get the image from the assembly */
		pAssemblyImage = mono_assembly_get_image(pAssembly);

		/* Load debug symbols if they exist */
		std::filesystem::path pdbPath = assemblyPath;
		pdbPath.replace_extension(".pdb");

		if (MonoManager::Instance()->DebuggingEnabled())
		{
			if (std::filesystem::exists(pdbPath))
			{
				std::ifstream pdbFileStream;
				pdbFileStream.open(assemblyPathStr, std::ios::in | std::ios::ate | std::ios::binary);
				m_DebugDataSize = (size_t)pdbFileStream.tellg();
				pdbFileStream.seekg(0, std::ios::beg);

				m_DebugData = new mono_byte[m_DebugDataSize];
				pdbFileStream.read((char*)m_DebugData, m_DebugDataSize);
				pdbFileStream.close();

				if (m_DebugDataSize)
					mono_debug_open_image_from_memory(pAssemblyImage, m_DebugData, (uint32_t)m_DebugDataSize);
			}
			else
			{
				std::stringstream log;
				log << "No pdb file found for " << assemblyPath << " debugging this assembly will not be possible.";
				ENGINE->GetDebug().LogError(log.str());
			}
		}

        m_pAssembly = pAssembly;
        m_pImage = pAssemblyImage;
        m_Locked = false;
        return true;
    }
}
