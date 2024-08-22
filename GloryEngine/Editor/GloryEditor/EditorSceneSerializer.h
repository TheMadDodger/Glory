#pragma once
#include "Entity.h"
#include "GloryEditor.h"

#include <NodeRef.h>

namespace YAML
{
    class Node;
    class Emitter;
}

namespace Glory
{
    struct UUIDRemapper;
    class GScene;
    class Engine;

namespace Editor
{
    /** @brief Editor scene serializer */
    class EditorSceneSerializer
    {
    public:
        /** @brief Deserialization flags */
        enum Flags
        {
            /** @brief Generate new UUIDs for every entity and component */
            GenerateNewUUIDs = 1,
            /** @brief Skip deserialization of prefab instances */
            IgnorePrefabs = 2,
            /** @brief Prevent component callbacks from firing */
            NoComponentCallbacks = 4,
        };

        /**
         * @brief Serialize a scene to a YAML emitter
         * @param pEngine The current engine instance
         * @param pScene The scene to serialize
         * @param node YAML node to serialize the scene to
         */
        GLORY_EDITOR_API static void SerializeScene(Engine* pEngine, GScene* pScene, Utils::NodeValueRef node);
        /**
         * @brief Deserialize a scene from a YAML node
         * @param pEngine The current engine instance
         * @param node The root yaml node of the serialized scene
         * @param uuid UUID to pass to the scenes constructor
         * @param name Name to pass to the scenes constructor
         * @param flags Deserialization flags
         */
        GLORY_EDITOR_API static GScene* DeserializeScene(Engine* pEngine, Utils::NodeValueRef node, UUID uuid, const std::string& name, Flags flags = Flags(0));

        /**
         * @brief Deserialize a scene from a YAML node
         * @param pEngine The current engine instance
         * @param pScene Scene instance to deserialize the data into
         * @param node The root yaml node of the serialized scene
         * @param uuid UUID to pass to the scenes constructor
         * @param name Name to pass to the scenes constructor
         * @param flags Deserialization flags
         */
        GLORY_EDITOR_API static void DeserializeScene(Engine* pEngine, GScene* pScene, Utils::NodeValueRef node, UUID uuid, const std::string& name, Flags flags = Flags(0));

        /**
         * @brief Serialize an entity
         * @param pEngine The current engine instance
         * @param pScene The scene in which the entity exists
         * @param entity ID of the entity
         * @param entityNode YAML node to serialize the entity to
         */
        GLORY_EDITOR_API static void SerializeEntity(Engine* pEngine, GScene* pScene, Utils::ECS::EntityID entity, Utils::NodeValueRef entityNode);
        /**
         * @brief Serialize an entity and all its children
         * @param pEngine The current engine instance
         * @param pScene The scene in which the entity exists
         * @param entity ID of the entity
         * @param entities The YAML sequence node to serialize to
         */
        GLORY_EDITOR_API static void SerializeEntityRecursive(Engine* pEngine, GScene* pScene, Utils::ECS::EntityID entity, Utils::NodeValueRef entities);
        /**
         * @brief Deserialize an entity from a YAML node
         * @param pEngine The current engine instance
         * @param pScene The scene to add the entity to
         * @param node Serialized YAML entity data
         * @param flags Deserialization flags
         */
        GLORY_EDITOR_API static Entity DeserializeEntity(Engine* pEngine, GScene* pScene, Utils::NodeValueRef node, Flags flags = Flags(0));
        /**
         * @brief Serialize a component on an entity
         * @param pEngine The current engine instance
         * @param pRegistry The registry in which the entity exists
         * @param pEntityView View of the entity
         * @param entity ID of the entity
         * @param index Index of the component to serialize
         * @param node The YAML node to serialize to
         */
        GLORY_EDITOR_API static void SerializeComponent(Engine* pEngine, Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityView* pEntityView, Utils::ECS::EntityID entity, size_t index, Utils::NodeValueRef node);
        /**
         * @brief Deserialize a component to an entity
         * @param pEngine The current engine instance
         * @param pScene The scene in which the entity exists on which to add the component
         * @param entity ID of the entity to add the component to
         * @param uuidRemapper UUID remapper to regenerate or match UUIDs with
         * @param component Serialized component data
         * @param flags Deserialization flags
         */
        GLORY_EDITOR_API static void DeserializeComponent(Engine* pEngine, GScene* pScene, Utils::ECS::EntityID entity, UUIDRemapper& uuidRemapper, Utils::NodeValueRef component, Flags flags = Flags(0));
    };
}
}