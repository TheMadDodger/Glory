#pragma once
#include <EntityRegistry.h>

#include <glm/matrix.hpp>

#include <map>
#include <memory>

namespace Glory
{
	class UIDocumentData;
	class UIRendererModule;
	class RenderTexture;
	class MeshData;
	class FontData;
	struct TextRenderData;

	/** @brief Renderable copy of a UI document */
	class UIDocument
	{
	public:
		UIDocument(UIDocumentData* pDocument);

		RenderTexture* GetUITexture();

		void Update();
		void Draw();

		UIRendererModule* Renderer();
		glm::mat4& Projection();
		const glm::mat4& Projection() const;

		MeshData* GetTextMesh(const TextRenderData& data, FontData* pFont);

	private:
		void CopyEntity(Utils::ECS::EntityRegistry& registry, Utils::ECS::EntityID entity, Utils::ECS::EntityID parent);

	private:
		friend class UIRendererModule;
		UUID m_ObjectID;
		UUID m_OriginalDocumentID;
		Utils::ECS::EntityRegistry m_Registry;
		RenderTexture* m_pUITexture;
		UIRendererModule* m_pRenderer;
		glm::mat4 m_Projection;

		std::map<UUID, std::unique_ptr<MeshData>> m_pTextMeshes;
	};
}
