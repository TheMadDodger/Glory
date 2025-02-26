#pragma once
#include <EntityRegistry.h>

namespace Glory
{
	class UIDocumentData;
	class RenderTexture;

	/** @brief Renderable copy of a UI document */
	class UIDocument
	{
	public:
		UIDocument(UIDocumentData* pDocument);

		RenderTexture* GetUITexture();

	private:
		void CopyEntity(Utils::ECS::EntityRegistry& registry, Utils::ECS::EntityID entity, Utils::ECS::EntityID parent);

	private:
		friend class UIRendererModule;
		UUID m_OriginalDocumentID;
		Utils::ECS::EntityRegistry m_Registry;
		RenderTexture* m_pUITexture;
	};
}
