#pragma once
#include "Serializer.h"
#include "GScene.h"
#include "Entity.h"

namespace Glory
{
    struct UUIDRemapper;

    class SceneSerializer : public SerializerTemplate<GScene>
    {
    public:
        SceneSerializer();
        virtual ~SceneSerializer();

        static void SerializeEntity(GScene* pScene, Utils::ECS::EntityID entity, YAML::Emitter& out);
        static void SerializeEntityRecursive(GScene* pScene, Utils::ECS::EntityID entity, YAML::Emitter& out);
        static Entity DeserializeEntity(GScene* pScene, YAML::Node& object, Flags flags = Flags(0));
        static void SerializeComponent(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityView* pEntityView, Utils::ECS::EntityID entity, size_t index, YAML::Emitter& out);
        static void DeserializeComponent(GScene* pScene, Utils::ECS::EntityID entity, UUIDRemapper& uuidRemapper, YAML::Node& object, Flags flags = Flags(0));

    private:
        virtual void Serialize(GScene* pScene, YAML::Emitter& out) override;
        virtual Object* Deserialize(Object* pParent, YAML::Node& object, UUID uuid, const std::string& name, Flags flags) override;
    };
}
