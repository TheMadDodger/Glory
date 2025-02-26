#include "UIDocument.h"
#include "UIDocumentData.h"

namespace Glory
{
	void UIDocument::CopyEntity(Utils::ECS::EntityRegistry& registry, Utils::ECS::EntityID entity, Utils::ECS::EntityID parent)
	{
		const Utils::ECS::EntityID newEntity = registry.CopyEntityToOtherRegistry(entity, parent, &m_Registry);
		Utils::ECS::EntityView* pEntityView = registry.GetEntityView(entity);
		for (size_t i = 0; i < pEntityView->ChildCount(); ++i)
		{
			const Utils::ECS::EntityID child = pEntityView->Child(i);
			CopyEntity(registry, child, newEntity);
		}
	}

	UIDocument::UIDocument(UIDocumentData* pDocument): m_OriginalDocumentID(pDocument->GetUUID()), m_pUITexture(nullptr)
	{
		Utils::ECS::EntityRegistry& registry = pDocument->GetRegistry();
		for (size_t i = 0; i < registry.ChildCount(0); ++i)
		{
			const Utils::ECS::EntityID child = registry.Child(0, i);
			CopyEntity(registry, child, 0);
		}
	}

	RenderTexture* UIDocument::GetUITexture()
	{
		return m_pUITexture;
	}
}
