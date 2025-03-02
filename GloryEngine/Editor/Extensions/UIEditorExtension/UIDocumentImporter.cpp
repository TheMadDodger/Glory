#include "UIDocumentImporter.h"

#include <Engine.h>
#include <Serializers.h>
#include <SceneManager.h>
#include <UIRendererModule.h>

#include <EditorApplication.h>

#include <NodeRef.h>
#include <Reflection.h>

namespace Glory::Editor
{
    std::string_view UIDocumentImporter::Name() const
    {
        return "UI Document Importer";
    }

    bool UIDocumentImporter::SupportsExtension(const std::filesystem::path& extension) const
    {
        return extension.compare(".gui") == 0;
    }

	void UIDocumentImporter::DeserializeComponent(Engine* pEngine, UIDocumentData* pDocument, Utils::ECS::EntityID entity, Utils::NodeValueRef component)
	{
		const UUID compUUID = component["UUID"].As<uint64_t>();
		Utils::NodeValueRef activeNode = component["Active"];
		const bool active = activeNode.Exists() ? activeNode.As<bool>() : true;
		const std::string typeName = component["TypeName"].As<std::string>();
		const uint32_t typeHash = component["TypeHash"].As<uint32_t>();

		Utils::ECS::EntityRegistry& pRegistry = pDocument->GetRegistry();

		void* pComponentAddress = pRegistry.CreateComponent(entity, typeHash, compUUID);
		const TypeData* pTypeData = Reflect::GetTyeData(typeHash);
		pEngine->GetSerializers().DeserializeProperty(pTypeData, pComponentAddress, component["Properties"]);

		Utils::ECS::BaseTypeView* pTypeView = pRegistry.GetTypeView(typeHash);
		pTypeView->SetActive(entity, active);
	}

	void UIDocumentImporter::DeserializeEntity(Engine* pEngine, UIDocumentData* pDocument, Utils::NodeValueRef node)
	{
		Utils::ECS::EntityRegistry& registry = pDocument->GetRegistry();

		const std::string name = node["Name"].As<std::string>();
		const UUID uuid = node["UUID"].As<uint64_t>();
		const UUID parentUuid = node["Parent"].As<uint64_t>();
		const Utils::ECS::EntityID entity = pDocument->CreateEmptyEntity(name, uuid);

		if (parentUuid != NULL)
		{
			Utils::ECS::EntityID parent = pDocument->EntityID(parentUuid);
			registry.SetParent(entity, parent);
		}

		size_t currentComponentIndex = 0;

		Utils::NodeValueRef components = node["Components"];
		for (size_t i = 0; i < components.Size(); ++i)
		{
			Utils::NodeValueRef component = components[i];
			DeserializeComponent(pEngine, pDocument, entity, component);
			++currentComponentIndex;
		}
	}

    ImportedResource UIDocumentImporter::LoadResource(const std::filesystem::path& path, void*) const
    {
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
        UIDocumentData* pNewDocument = new UIDocumentData();
		Utils::YAMLFileRef file{ path };
		auto node = file.RootNodeRef().ValueRef();

		Utils::NodeValueRef entities = node["Entities"];
		for (size_t i = 0; i < entities.Size(); ++i)
		{
			Utils::NodeValueRef entity = entities[i];
			DeserializeEntity(pEngine, pNewDocument, entity);
		}

        return { path, pNewDocument };
    }

	void SerializeComponent(Engine* pEngine, UIDocumentData* pDocument, Utils::ECS::EntityView* pEntityView, Utils::ECS::EntityID entity, size_t index, Utils::NodeValueRef node)
	{
		Utils::ECS::EntityRegistry& registry = pDocument->GetRegistry();

		node.Set(YAML::Node(YAML::NodeType::Map));
		const UUID compUUID = pEntityView->ComponentUUIDAt(index);
		node["UUID"].Set(uint64_t(compUUID));

		const uint32_t type = pEntityView->ComponentTypeAt(index);
		const Utils::Reflect::TypeData* pType = Utils::Reflect::Reflect::GetTyeData(type);

		node["TypeName"].Set(pType->TypeName());
		node["TypeHash"].Set(uint64_t(type));
		node["Active"].Set(registry.GetTypeView(type)->IsActive(entity));

		pEngine->GetSerializers().SerializeProperty(pType, registry.GetComponentAddress(entity, compUUID), node["Properties"]);
	}

	void SerializeEntity(Engine* pEngine, UIDocumentData* pDocument, Utils::ECS::EntityID entity, Utils::NodeValueRef entityNode)
	{
		Utils::ECS::EntityRegistry& registry = pDocument->GetRegistry();

		Utils::ECS::EntityView* pEntityView = registry.GetEntityView(entity);
		Utils::ECS::EntityID parent = pEntityView->Parent();

		entityNode.Set(YAML::Node(YAML::NodeType::Map));
		entityNode["UUID"].Set(uint64_t(pDocument->EntityUUID(entity)));
		entityNode["Name"].Set(std::string{ pDocument->Name(entity) });
		entityNode["Parent"].Set(uint64_t(parent ? pDocument->EntityUUID(parent) : 0));

		auto components = entityNode["Components"];
		components.Set(YAML::Node(YAML::NodeType::Sequence));
		for (size_t i = 0; i < pEntityView->ComponentCount(); ++i)
		{
			SerializeComponent(pEngine, pDocument, pEntityView, entity, i, components[i]);
		}
	}

	void UIDocumentImporter::SerializeEntityRecursive(Engine* pEngine, UIDocumentData* pDocument, Utils::ECS::EntityID entity, Utils::NodeValueRef entities)
	{
		Utils::ECS::EntityRegistry& registry = pDocument->GetRegistry();

		const size_t index = entities.Size();
		entities.PushBack(YAML::Node(YAML::NodeType::Map));

		/* Serialize entity first then its children */
		SerializeEntity(pEngine, pDocument, entity, entities[index]);

		for (size_t i = 0; i < registry.ChildCount(entity); ++i)
		{
			Utils::ECS::EntityID child = registry.Child(entity, i);
			SerializeEntityRecursive(pEngine, pDocument, child, entities);
		}
	}

	void SerializeComponent(Engine* pEngine, UIDocument* pDocument, Utils::ECS::EntityView* pEntityView, Utils::ECS::EntityID entity, size_t index, Utils::NodeValueRef node)
	{
		Utils::ECS::EntityRegistry& registry = pDocument->Registry();

		node.Set(YAML::Node(YAML::NodeType::Map));
		const UUID compUUID = pEntityView->ComponentUUIDAt(index);
		node["UUID"].Set(uint64_t(compUUID));

		const uint32_t type = pEntityView->ComponentTypeAt(index);
		const Utils::Reflect::TypeData* pType = Utils::Reflect::Reflect::GetTyeData(type);

		node["TypeName"].Set(pType->TypeName());
		node["TypeHash"].Set(uint64_t(type));
		node["Active"].Set(registry.GetTypeView(type)->IsActive(entity));

		pEngine->GetSerializers().SerializeProperty(pType, registry.GetComponentAddress(entity, compUUID), node["Properties"]);
	}

	void SerializeEntity(Engine* pEngine, UIDocument* pDocument, Utils::ECS::EntityID entity, Utils::NodeValueRef entityNode)
	{
		Utils::ECS::EntityRegistry& registry = pDocument->Registry();

		Utils::ECS::EntityView* pEntityView = registry.GetEntityView(entity);
		Utils::ECS::EntityID parent = pEntityView->Parent();

		entityNode.Set(YAML::Node(YAML::NodeType::Map));
		entityNode["UUID"].Set(uint64_t(pDocument->EntityUUID(entity)));
		entityNode["Name"].Set(std::string{ pDocument->Name(entity) });
		entityNode["Parent"].Set(uint64_t(parent ? pDocument->EntityUUID(parent) : 0));

		auto components = entityNode["Components"];
		components.Set(YAML::Node(YAML::NodeType::Sequence));
		for (size_t i = 0; i < pEntityView->ComponentCount(); ++i)
		{
			SerializeComponent(pEngine, pDocument, pEntityView, entity, i, components[i]);
		}
	}

	void UIDocumentImporter::SerializeEntityRecursive(Engine* pEngine, UIDocument* pDocument, Utils::ECS::EntityID entity, Utils::NodeValueRef entities)
	{
		Utils::ECS::EntityRegistry& registry = pDocument->Registry();

		const size_t index = entities.Size();
		entities.PushBack(YAML::Node(YAML::NodeType::Map));

		/* Serialize entity first then its children */
		SerializeEntity(pEngine, pDocument, entity, entities[index]);

		for (size_t i = 0; i < registry.ChildCount(entity); ++i)
		{
			Utils::ECS::EntityID child = registry.Child(entity, i);
			SerializeEntityRecursive(pEngine, pDocument, child, entities);
		}
	}

    bool UIDocumentImporter::SaveResource(const std::filesystem::path& path, UIDocumentData* pDocument) const
    {
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();

        Utils::YAMLFileRef file{ path };

        auto node = file.RootNodeRef().ValueRef();

        if (!node.Exists() || !node.IsMap())
            node.Set(YAML::Node(YAML::NodeType::Map));

        auto entities = node["Entities"];
        entities.Set(YAML::Node(YAML::NodeType::Sequence));

		Utils::ECS::EntityRegistry& registry = pDocument->GetRegistry();

        for (size_t i = 0; i < registry.ChildCount(0); ++i)
        {
            Utils::ECS::EntityID child = registry.Child(0, i);
            SerializeEntityRecursive(pEngine, pDocument, child, entities);
        }

        file.Save();
        return true;
    }

    void UIDocumentImporter::Initialize()
    {
    }
}
