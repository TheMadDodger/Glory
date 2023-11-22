#pragma once
#include "Serializer.h"
#include "GScene.h"

namespace Glory
{
    class SceneSerializer : public SerializerTemplate<GScene>
    {
    public:
        SceneSerializer();
        virtual ~SceneSerializer();

        static void SerializeComponent(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityView* pEntityView, Utils::ECS::EntityID entity, size_t index, YAML::Emitter& out);
        static void DeserializeComponent();

    private:
        virtual void Serialize(GScene* pScene, YAML::Emitter& out) override;
        virtual Object* Deserialize(Object* pParent, YAML::Node& object, const std::string& name, Flags flags) override;
    };
}
