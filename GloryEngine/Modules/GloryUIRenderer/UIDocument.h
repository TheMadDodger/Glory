#pragma once
#include <Glory.h>
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
		GLORY_API UIDocument(UIDocumentData* pDocument);

		GLORY_API RenderTexture* GetUITexture();

		GLORY_API void Update();
		GLORY_API void Draw();

		GLORY_API UIRendererModule* Renderer();
		GLORY_API glm::mat4& Projection();
		GLORY_API const glm::mat4& Projection() const;

		GLORY_API MeshData* GetTextMesh(const TextRenderData& data, FontData* pFont);

		GLORY_API void SetRenderTexture(RenderTexture* pTexture);

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
