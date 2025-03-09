#pragma once
#include <Glory.h>
#include <EntityRegistry.h>
#include <TypeData.h>

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

		GLORY_API UUID OriginalDocumentID() const;
		GLORY_API UIRendererModule* Renderer();
		GLORY_API glm::mat4& Projection();
		GLORY_API const glm::mat4& Projection() const;

		GLORY_API MeshData* GetTextMesh(const TextRenderData& data, FontData* pFont);

		GLORY_API void SetRenderTexture(RenderTexture* pTexture);

		GLORY_API Utils::ECS::EntityRegistry& Registry();
		GLORY_API std::string_view Name() const;
		GLORY_API std::string_view Name(Utils::ECS::EntityID entity) const;
		GLORY_API void SetName(Utils::ECS::EntityID entity, std::string_view name);

		GLORY_API UUID EntityUUID(Utils::ECS::EntityID entity) const;
		GLORY_API Utils::ECS::EntityID EntityID(UUID uuid) const;

		GLORY_API Utils::ECS::EntityID CreateEmptyEntity(std::string_view name, UUID uuid = UUID());
		GLORY_API Utils::ECS::EntityID CreateEntity(std::string_view name, UUID uuid = UUID());
		GLORY_API void DestroyEntity(UUID uuid);
		GLORY_API bool EntityExists(UUID uuid);
		GLORY_API size_t ElementCount();

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
		std::string m_Name;
		std::map<UUID, Utils::ECS::EntityID> m_Ids;
		std::map<Utils::ECS::EntityID, UUID> m_UUIds;
		std::map<Utils::ECS::EntityID, std::string> m_Names;
	};
}
