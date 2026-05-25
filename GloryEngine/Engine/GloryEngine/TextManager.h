#pragma once
#include "Components.h"

#include <ComponentManager.h>

namespace Glory
{
    class TextManager : public Utils::ECS::ComponentManager<TextComponent>
    {
    public:
        GLORY_ENGINE_API TextManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity=100);
        GLORY_ENGINE_API virtual ~TextManager();

    public:
        GLORY_ENGINE_API void OnDrawImpl(Utils::ECS::EntityID entity, TextComponent& pComponent);
        GLORY_ENGINE_API void GetReferencesImpl(std::vector<UUID>& references) const;
        GLORY_ENGINE_API void OnDeserialize(Utils::BinaryStream&) override;

    private:
        virtual void OnInitialize() override;

    private:
        friend class SceneManager;
        SceneManager* m_pSceneManager;
        Resources* m_pResources;
        LayerManager* m_pLayerManager;
    };
}