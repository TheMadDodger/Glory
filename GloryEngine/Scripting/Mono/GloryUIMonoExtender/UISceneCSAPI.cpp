#include "UISceneCSAPI.h"

#include <UIDocument.h>
#include <UIRendererModule.h>
#include <UIComponents.h>
#include <cstdint>
#include <GloryMonoScipting.h>
#include <Debug.h>
#include <Engine.h>
#include <ComponentTypes.h>
#include <AssetManager.h>

namespace Glory
{
	Engine* UIScene_EngineInstance;
#define UI_MODULE UIScene_EngineInstance->GetOptionalModule<UIRendererModule>()

#pragma region UI Scene

    uint64_t UIScene_NewEmptyObject(uint64_t sceneID)
    {
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (!pDocument) return 0;
        const Utils::ECS::EntityID entity = pDocument->CreateEntity("New Empty Element");
        return entity ? pDocument->EntityUUID(entity) : 0;
    }

    uint64_t UIScene_NewEmptyObjectWithName(uint64_t sceneID, MonoString* name)
    {
        const std::string_view nameStr = mono_string_to_utf8(name);
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (!pDocument) return 0;
        const Utils::ECS::EntityID entity = pDocument->CreateEntity(nameStr);
        return entity ? pDocument->EntityUUID(entity) : 0;
    }

    uint32_t UIScene_ObjectsCount(uint64_t sceneID)
    {
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (!pDocument) return 0;
        return pDocument->ElementCount();
    }

    uint64_t UIScene_FindElement(uint64_t sceneID, MonoString* name)
    {
        const std::string_view nameStr = mono_string_to_utf8(name);
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (!pDocument) return 0;
        return pDocument->FindElement(0, nameStr);
    }

    uint64_t UIScene_Instantiate(uint64_t sceneID, uint64_t documentID, uint64_t parentID)
    {
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (!pDocument) return 0;
        Resource* pResource = UIScene_EngineInstance->GetAssetManager().FindResource(documentID);
        if (!pResource) return 0;
        UIDocumentData* pDocumentData = static_cast<UIDocumentData*>(pResource);
        const UUID instantiated = pDocument->Instantiate(pDocumentData, parentID);
        pDocument->SetEntityDirty(pDocument->EntityID(instantiated), true, true);
        pDocument->SetDrawDirty();
        return instantiated;
    }

#pragma endregion

#pragma region UI Element

    bool UIElement_GetActive(uint64_t sceneID, uint64_t objectID)
    {
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (!pDocument) return false;
        const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
        return pDocument->Registry().GetEntityView(entity)->Active();
    }

    void UIElement_SetActive(uint64_t sceneID, uint64_t objectID, bool active)
    {
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (!pDocument) return;
        const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
        pDocument->SetEntityActive(entity, active);
    }
    
    MonoString* UIElement_GetName(uint64_t sceneID, uint64_t objectID)
    {
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (!pDocument) return 0;
        const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
        return mono_string_new(mono_domain_get(), pDocument->Name(entity).data());
    }

    void UIElement_SetName(uint64_t sceneID, uint64_t objectID, MonoString* name)
    {
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (!pDocument) return;
        const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
        pDocument->SetName(entity, mono_string_to_utf8(name));
    }

    uint32_t UIElement_GetSiblingIndex(uint64_t sceneID, uint64_t objectID)
    {
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (!pDocument) return 0;
        const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
        return pDocument->Registry().SiblingIndex(entity);
    }

    void UIElement_SetSiblingIndex(uint64_t sceneID, uint64_t objectID, uint32_t index)
    {
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (!pDocument) return;
        const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
        pDocument->Registry().SetSiblingIndex(entity, index);
        pDocument->SetEntityDirty(entity, true, true);
        pDocument->SetDrawDirty();
    }

    uint32_t UIElement_GetChildCount(uint64_t sceneID, uint64_t objectID)
    {
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (!pDocument) return 0;
        const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
        return pDocument->Registry().ChildCount(entity);
    }

    uint64_t UIElement_GetChild(uint64_t sceneID, uint64_t objectID, uint32_t index)
    {
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (!pDocument) return 0;
        const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
        const size_t childCount = pDocument->Registry().ChildCount(entity);
        if (index >= childCount) return 0;
        const Utils::ECS::EntityID child = pDocument->Registry().Child(entity, index);
        return child ? pDocument->EntityUUID(child) : 0;
    }

    uint64_t UIElement_GetParent(uint64_t sceneID, uint64_t objectID)
    {
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (!pDocument) return 0;
        const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
        const Utils::ECS::EntityID parent = pDocument->Registry().GetParent(entity);
        return parent ? pDocument->EntityUUID(parent) : 0;
    }

    void UIElement_SetParent(uint64_t sceneID, uint64_t objectID, uint64_t parentID)
    {
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (!pDocument) return;
        const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
        const Utils::ECS::EntityID oldParent = pDocument->Registry().GetParent(entity);
        pDocument->Registry().SetParent(entity, parentID ? pDocument->EntityID(parentID) : 0);
        pDocument->SetEntityDirty(entity, true, true);
        if (oldParent) pDocument->SetEntityDirty(oldParent, true, true);
        pDocument->SetDrawDirty();
    }

    uint64_t UIElement_GetComponentID(uint64_t sceneID, uint64_t objectID, MonoString* name)
    {
        const std::string componentName{ mono_string_to_utf8(name) };
        if (objectID == 0 || sceneID == 0) return 0;
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (pDocument == nullptr) return 0;
        const uint32_t componentHash = Glory::ComponentTypes::GetComponentHash(componentName);
        const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
        Utils::ECS::EntityRegistry& registry = pDocument->Registry();
        Utils::ECS::EntityView* pEntityView = registry.GetEntityView(entity);

        for (auto iter = pEntityView->GetIterator(); iter != pEntityView->GetIteratorEnd(); iter++)
        {
            if (iter->second != componentHash) continue;
            return iter->first;
        }
        return 0;
    }

    uint64_t UIElement_AddComponent(uint64_t sceneID, uint64_t objectID, MonoString* name)
    {
        const std::string componentName{ mono_string_to_utf8(name) };
        if (objectID == 0 || sceneID == 0) return 0;
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (pDocument == nullptr) return 0;
        const uint32_t componentHash = Glory::ComponentTypes::GetComponentHash(componentName);
        const UUID uuid{};

        const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
        Utils::ECS::EntityRegistry& registry = pDocument->Registry();
        Utils::ECS::EntityView* pEntityView = registry.GetEntityView(entity);

        if (registry.HasComponent(entity, componentHash))
        {
            UIScene_EngineInstance->GetDebug().LogError("Mutliple components of the same type on one entity is currently not supported");
            return 0;
        }

        void* pNewComponent = registry.CreateComponent(entity, componentHash, uuid);
        Utils::ECS::BaseTypeView* pTypeView = registry.GetTypeView(componentHash);
        pTypeView->Invoke(Utils::ECS::InvocationType::OnValidate, &registry, entity, pNewComponent);
        pTypeView->Invoke(Utils::ECS::InvocationType::Start, &registry, entity, pNewComponent);
        /* We can assume the component is active, but is the entity active? */
        if (pEntityView->IsActive())
            pTypeView->Invoke(Utils::ECS::InvocationType::OnEnable, &registry, entity, pNewComponent);

        pDocument->SetEntityDirty(entity, true, true);
        pDocument->SetDrawDirty();

        return uuid;
    }

    uint64_t UIElement_RemoveComponent(uint64_t sceneID, uint64_t objectID, MonoString* name)
    {
        const std::string componentName{ mono_string_to_utf8(name) };
        if (objectID == 0 || sceneID == 0) return 0;
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (pDocument == nullptr) return 0;
        const uint32_t componentHash = Glory::ComponentTypes::GetComponentHash(componentName);
        const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
        Utils::ECS::EntityRegistry& registry = pDocument->Registry();
        if (!componentHash) return 0;
        const UUID componentID = registry.RemoveComponent(entity, componentHash);
        pDocument->SetEntityDirty(entity, true, true);
        pDocument->SetDrawDirty();
        return componentID;
    }

    void UIElement_RemoveComponentByID(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
    {
        if (objectID == 0 || sceneID == 0) return;
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (pDocument == nullptr) return;
        const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
        Utils::ECS::EntityRegistry& registry = pDocument->Registry();
        Utils::ECS::EntityView* pEntityView = registry.GetEntityView(entity);
        const uint32_t hash = pEntityView->ComponentType(componentID);
        if (!hash) return;
        registry.RemoveComponent(entity, hash);
        pDocument->SetEntityDirty(entity, true, true);
        pDocument->SetDrawDirty();
    }

    uint64_t UIElement_FindElement(uint64_t sceneID, uint64_t elementId, MonoString* name)
    {
        const std::string_view nameStr = mono_string_to_utf8(name);
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (!pDocument) return 0;
        return pDocument->FindElement(elementId, nameStr);
    }

#pragma endregion

#pragma region UI Component

    bool UIComponent_GetActive(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
    {
        if (objectID == 0 || sceneID == 0) return false;
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (pDocument == nullptr) return false;
        const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
        Utils::ECS::EntityRegistry& registry = pDocument->Registry();
        Utils::ECS::EntityView* pEntityView = registry.GetEntityView(entity);
        const uint32_t type = pEntityView->ComponentType(componentID);
        Utils::ECS::BaseTypeView* pTypeView = registry.GetTypeView(type);
        return pTypeView->IsActive(entity);
    }

    void UIComponent_SetActive(uint64_t sceneID, uint64_t objectID, uint64_t componentID, bool active)
    {
        if (objectID == 0 || sceneID == 0) return;
        UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
        if (pDocument == nullptr) return;
        const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
        Utils::ECS::EntityRegistry& registry = pDocument->Registry();
        Utils::ECS::EntityView* pEntityView = registry.GetEntityView(entity);
        const uint32_t type = pEntityView->ComponentType(componentID);
        Utils::ECS::BaseTypeView* pTypeView = registry.GetTypeView(type);
        pTypeView->SetActive(entity, active);
        pDocument->SetEntityDirty(entity, true, true);
        pDocument->SetDrawDirty();
    }

#pragma endregion

#pragma region Binding

	void UISceneCSAPI::AddInternalCalls(std::vector<InternalCall>& internalCalls)
	{
        /* UI Scene */
        BIND("GloryEngine.UI.UIScene::UIScene_NewEmptyObject", UIScene_NewEmptyObject);
        BIND("GloryEngine.UI.UIScene::UIScene_NewEmptyObjectWithName", UIScene_NewEmptyObjectWithName);
        BIND("GloryEngine.UI.UIScene::UIScene_ObjectsCount", UIScene_ObjectsCount);
        BIND("GloryEngine.UI.UIScene::UIScene_FindElement", UIScene_FindElement);
        BIND("GloryEngine.UI.UIScene::UIScene_Instantiate", UIScene_Instantiate);

        /* UI Element */
        BIND("GloryEngine.UI.UIElement::UIElement_GetActive", UIElement_GetActive);
        BIND("GloryEngine.UI.UIElement::UIElement_SetActive", UIElement_SetActive);
        BIND("GloryEngine.UI.UIElement::UIElement_GetName", UIElement_GetName);
        BIND("GloryEngine.UI.UIElement::UIElement_SetName", UIElement_SetName);
        BIND("GloryEngine.UI.UIElement::UIElement_GetSiblingIndex", UIElement_GetSiblingIndex);
        BIND("GloryEngine.UI.UIElement::UIElement_SetSiblingIndex", UIElement_SetSiblingIndex);
        BIND("GloryEngine.UI.UIElement::UIElement_GetChildCount", UIElement_GetChildCount);
        BIND("GloryEngine.UI.UIElement::UIElement_GetChild", UIElement_GetChild);
        BIND("GloryEngine.UI.UIElement::UIElement_GetParent", UIElement_GetParent);
        BIND("GloryEngine.UI.UIElement::UIElement_SetParent", UIElement_SetParent);
        BIND("GloryEngine.UI.UIElement::UIElement_GetComponentID", UIElement_GetComponentID);
        BIND("GloryEngine.UI.UIElement::UIElement_AddComponent", UIElement_AddComponent);
        BIND("GloryEngine.UI.UIElement::UIElement_RemoveComponent", UIElement_RemoveComponent);
        BIND("GloryEngine.UI.UIElement::UIElement_RemoveComponentByID", UIElement_RemoveComponentByID);
        BIND("GloryEngine.UI.UIElement::UIElement_FindElement", UIElement_FindElement);

        /* UI Component */
        BIND("GloryEngine.UI.UIComponent::UIComponent_GetActive", UIComponent_GetActive);
        BIND("GloryEngine.UI.UIComponent::UIComponent_SetActive", UIComponent_SetActive);
	}

	void UISceneCSAPI::SetEngine(Engine* pEngine)
	{
		UIScene_EngineInstance = pEngine;
	}

#pragma endregion

}