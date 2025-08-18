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
	class GraphicsModule;
	struct TextData;

	/** @brief Renderable copy of a UI document */
	class UIDocument
	{
	public:
		GLORY_API UIDocument(UIDocumentData* pDocument);

		GLORY_API RenderTexture* GetUITexture();

		GLORY_API void Update();
		GLORY_API void Draw(GraphicsModule* pGraphics, const glm::vec4& clearColor={0.0f, 0.0f, 0.0f, 0.0f});

		GLORY_API UUID OriginalDocumentID() const;
		GLORY_API UUID SceneID() const;
		GLORY_API UUID ObjectID() const;
		GLORY_API UIRendererModule* Renderer();
		GLORY_API glm::mat4& Projection();
		GLORY_API const glm::mat4& Projection() const;

		GLORY_API MeshData* GetTextMesh(UUID objectID, const TextData& data, FontData* pFont);

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
		GLORY_API UUID FindElement(UUID parentId, std::string_view name);
		GLORY_API const glm::vec2& GetCursorPos() const;
		GLORY_API const glm::vec2& GetCursorScrollDelta() const;
		GLORY_API bool IsCursorDown() const;
		GLORY_API bool WasCursorDown() const;
		GLORY_API bool IsEnputEnabled() const;
		GLORY_API void SetEnputEnabled(bool enabled);
		GLORY_API size_t& PanelCounter();
		GLORY_API void SetDrawDirty();
		GLORY_API UUID Instantiate(UIDocumentData* pOtherDocument, UUID parentID);
		GLORY_API void SetAllEntitiesDirty();
		GLORY_API void SetEntityActive(Utils::ECS::EntityID entity, bool active);
		GLORY_API void Start();
		GLORY_API void SetEntityDirty(Utils::ECS::EntityID entity, bool setChildrenDirty, bool setParentsDirty);

	private:
		void CopyEntity(Utils::ECS::EntityRegistry& registry, Utils::ECS::EntityID entity, Utils::ECS::EntityID parent);
		UUID CopyEntity(UIDocumentData* pOtherDocument, Utils::ECS::EntityID entity, Utils::ECS::EntityID parent);

		void UpdateEntityActiveHierarchy(Utils::ECS::EntityID entity);

	private:
		friend class UIRendererModule;
		UUID m_SceneID;
		UUID m_ObjectID;
		UUID m_OriginalDocumentID;
		Utils::ECS::EntityRegistry m_Registry;
		RenderTexture* m_pUITexture;
		UIRendererModule* m_pRenderer;
		glm::mat4 m_Projection;
		glm::vec2 m_CursorPos;
		glm::vec2 m_CursorScrollDelta;
		bool m_CursorDown;
		bool m_WasCursorDown;
		bool m_InputEnabled;
		size_t m_PanelCounter;
		bool m_DrawIsDirty;

		std::map<UUID, std::unique_ptr<MeshData>> m_pTextMeshes;
		std::string m_Name;
		std::map<UUID, Utils::ECS::EntityID> m_Ids;
		std::map<Utils::ECS::EntityID, UUID> m_UUIds;
		std::map<Utils::ECS::EntityID, std::string> m_Names;
	};
}
