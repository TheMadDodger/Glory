#pragma once
#include "Localize.h"

#include <ComponentManager.h>

namespace Glory
{
    class LocalizeModule;
    class Debug;

    class StringTableLoaderManager : public Utils::ECS::ComponentManager<StringTableLoader>
    {
    public:
        StringTableLoaderManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity=1);
        virtual ~StringTableLoaderManager();

    public:
        void OnValidateImpl(Utils::ECS::EntityID entity, StringTableLoader& pComponent);
        void OnStopImpl(Utils::ECS::EntityID entity, StringTableLoader& pComponent);
        void GetReferencesImpl(std::vector<UUID>& references) const;
        void OnDeserialize(Utils::BinaryStream&) override;

    private:
        virtual void OnInitialize() override;

    private:
        friend class LocalizeModule;
        LocalizeModule* m_pModule;
    };

    class LocalizeManager : public Utils::ECS::ComponentManager<Localize>
    {
    public:
        LocalizeManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity=100);
        virtual ~LocalizeManager();

    public:
        void OnValidateImpl(Utils::ECS::EntityID entity, Localize& pComponent);
        void OnStartImpl(Utils::ECS::EntityID entity, Localize& pComponent);

    private:
        virtual void OnInitialize() override;

    private:
        friend class LocalizeModule;
        LocalizeModule* m_pModule;
        Debug* m_pDebug;
    };
}