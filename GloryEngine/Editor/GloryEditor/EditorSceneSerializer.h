#pragma once
#include "Entity.h"
#include "GloryEditor.h"

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
            NoComponentCallbacks = 3,
        };

        /** @brief Serialize a scene to a YAML emitter
         * @param pEngine The current engine instance
         * @param pScene The scene to serialize
         * @param out The emitter to serialize to
         */
        GLORY_EDITOR_API static void SerializeScene(Engine* pEngine, GScene* pScene, YAML::Emitter& out);
        /** @brief Deserialize a scene from a YAML node
         * @param pEngine The current engine instance
         * @param object The root yaml node of the serialized scene
         * @param uuid UUID to pass to the scenes constructor
         * @param name Name to pass to the scenes constructor
         * @param flags Deserialization flags
         */
        GLORY_EDITOR_API static GScene* DeserializeScene(Engine* pEngine, YAML::Node& object, UUID uuid, const std::string& name, Flags flags = Flags(0));

        /** @brief Serialize an entity
         * @param pEngine The current engine instance
         * @param pScene The scene in which the entity exists
         * @param entity ID of the entity
         * @param out The emitter to serialize to
         */
        GLORY_EDITOR_API static void SerializeEntity(Engine* pEngine, GScene* pScene, Utils::ECS::EntityID entity, YAML::Emitter& out);
        /** @brief Serialize an entity and all its children
         * @param pEngine The current engine instance
         * @param pScene The scene in which the entity exists
         * @param entity ID of the entity
         * @param out The emitter to serialize to
         */
        GLORY_EDITOR_API static void SerializeEntityRecursive(Engine* pEngine, GScene* pScene, Utils::ECS::EntityID entity, YAML::Emitter& out);
        /** @brief Deserialize an entity from a YAML node
         * @param pEngine The current engine instance
         * @param pScene The scene to add the entity to
         * @param object The root node of the serialized entity
         * @param flags Deserialization flags
         */
        GLORY_EDITOR_API static Entity DeserializeEntity(Engine* pEngine, GScene* pScene, YAML::Node& object, Flags flags = Flags(0));
        /** @brief Serialize a component on an entity
         * @param pEngine The current engine instance
         * @param pRegistry The registry in which the entity exists
         * @param pEntityView View of the entity
         * @param entity ID of the entity
         * @param index Index of the component to serialize
         * @param out The emitter to serialize to
         */
        GLORY_EDITOR_API static void SerializeComponent(Engine* pEngine, Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityView* pEntityView, Utils::ECS::EntityID entity, size_t index, YAML::Emitter& out);
        /** @brief Deserialize a component to an entity
         * @param pEngine The current engine instance
         * @param pScene The scene in which the entity exists on which to add the component
         * @param entity ID of the entity to add the component to
         * @param uuidRemapper UUID remapper to regenerate or match UUIDs with
         * @param object The root node of the serialized component
         * @param flags Deserialization flags
         */
        GLORY_EDITOR_API static void DeserializeComponent(Engine* pEngine, GScene* pScene, Utils::ECS::EntityID entity, UUIDRemapper& uuidRemapper, YAML::Node& object, Flags flags = Flags(0));
    };
}
}