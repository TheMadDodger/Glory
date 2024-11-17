#include "ComponentHelpers.h"

namespace Glory::Components
{
    void Activate(Entity entity, Utils::ECS::BaseTypeView* pTypeView, size_t componentIndex)
    {
        /* If the component is already active, we don't need to do anything */
        if (pTypeView->IsActiveByIndex(componentIndex)) return;

        /* Enable the component */
        pTypeView->SetActiveByIndex(componentIndex, true);

        /* If the entity is also active, we should send the OnEnable event */
        if (!entity.IsActive()) return;
        void* pComponent = pTypeView->GetComponentAddressFromIndex(componentIndex);
        pTypeView->Invoke(Utils::ECS::InvocationType::OnEnable, entity.GetRegistry(), entity.GetEntityID(), pComponent);
    }

    void CallOnEnable(Entity entity)
    {
        /* If the entity is inactive, we don't need to do anything */
        if (!entity.IsActive()) return;

        Utils::ECS::EntityView* pEntity = entity.GetEntityView();
        for (size_t i = 0; i < pEntity->ComponentCount(); ++i)
        {
            const uint32_t type = pEntity->ComponentTypeAt(i);
            Utils::ECS::BaseTypeView* pTypeView = entity.GetRegistry()->GetTypeView(type);
            const size_t index = pTypeView->GetComponentIndex(entity.GetEntityID());
            CallOnEnable(entity, pTypeView, index);
        }
    }

    void CallOnEnable(Entity entity, Utils::ECS::BaseTypeView* pTypeView, size_t componentIndex)
    {
        /* If the entity is inactive, we don't need to do anything */
        if (!entity.IsActive()) return;

        /* If the component is inactive, we don't need to do anything */
        if (!pTypeView->IsActiveByIndex(componentIndex)) return;

        void* pComponent = pTypeView->GetComponentAddressFromIndex(componentIndex);
        pTypeView->Invoke(Utils::ECS::InvocationType::OnEnable, entity.GetRegistry(), entity.GetEntityID(), pComponent);
    }

    void Deactivate(Entity entity, Utils::ECS::BaseTypeView* pTypeView, size_t componentIndex)
    {
        /* If the component is already inactive, we don't need to do anything */
        if (!pTypeView->IsActiveByIndex(componentIndex)) return;

        /* Disable the component */
        pTypeView->SetActiveByIndex(componentIndex, false);

        /* If the entity was active, we should send the OnDisable event, otherwise we should assume it was already sent */
        if (!entity.IsActive()) return;
        void* pComponent = pTypeView->GetComponentAddressFromIndex(componentIndex);
        pTypeView->Invoke(Utils::ECS::InvocationType::OnDisable, entity.GetRegistry(), entity.GetEntityID(), pComponent);
    }

    void CallOnDisable(Entity entity, Utils::ECS::BaseTypeView* pTypeView, size_t componentIndex)
    {
        /* If the component is active and the entity is active, we don't need to do anything */
        if (pTypeView->IsActiveByIndex(componentIndex) && entity.IsActive()) return;

        void* pComponent = pTypeView->GetComponentAddressFromIndex(componentIndex);
        pTypeView->Invoke(Utils::ECS::InvocationType::OnDisable, entity.GetRegistry(), entity.GetEntityID(), pComponent);
    }

    bool Destroy(Entity entity, Utils::ECS::BaseTypeView* pTypeView, size_t componentIndex)
    {
        /* Deactivate the component */
        Deactivate(entity, pTypeView, componentIndex);

        void* pComponent = pTypeView->GetComponentAddressFromIndex(componentIndex);

        /* Send stop and remove events */
        pTypeView->Invoke(Utils::ECS::InvocationType::Stop, entity.GetRegistry(), entity.GetEntityID(), &pComponent);
        pTypeView->Invoke(Utils::ECS::InvocationType::OnRemove, entity.GetRegistry(), entity.GetEntityID(), &pComponent);

        /* Remove the component */
        return entity.GetRegistry()->RemoveComponent(entity.GetEntityID(), pTypeView->ComponentTypeHash());
    }
}
