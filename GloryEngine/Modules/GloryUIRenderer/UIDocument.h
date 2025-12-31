#pragma once
#include <Glory.h>
#include <EntityRegistry.h>
#include <TypeData.h>
#include <GraphicsHandles.h>
#include <MeshData.h>

#include <glm/matrix.hpp>

#include <map>
#include <memory>

namespace Glory
{
	class UIDocumentData;
	class UIRendererModule;
	class MeshData;
	class FontData;
	class GraphicsDevice;
	struct TextData;

	struct UIBatch
	{
		UIBatch() {};
		virtual ~UIBatch()
		{
			m_TextMeshes.clear();
			m_Worlds.clear();
			m_TextureIDs.clear();
			m_UniqueColors.clear();
			m_ColorIndices.clear();
		}

		void Reset()
		{
			m_TextMeshes.clear();
			m_Worlds.clear();
			m_TextureIDs.clear();
			m_UniqueColors.clear();
			m_ColorIndices.clear();
		}

		std::vector<UUID> m_TextMeshes;
		std::vector<glm::mat4> m_Worlds;
		std::vector<UUID> m_TextureIDs;
		Utils::BitSet m_MaskIncrements;
		Utils::BitSet m_MaskDecrements;

		std::vector<glm::vec4> m_UniqueColors;
		std::vector<uint32_t> m_ColorIndices;
	};

	/** @brief Renderable copy of a UI document */
	class UIDocument
	{
	public:
		GLORY_API UIDocument(UIDocumentData* pDocument);

		GLORY_API void Update();
		GLORY_API void Draw();

		GLORY_API UUID OriginalDocumentID() const;
		GLORY_API UUID SceneID() const;
		GLORY_API UUID ObjectID() const;
		GLORY_API UIRendererModule* Renderer();
		GLORY_API glm::mat4& Projection();
		GLORY_API const glm::mat4& Projection() const;

		GLORY_API UUID GetTextMesh(UUID objectID, const TextData& data, FontData* pFont);

		GLORY_API const glm::uvec2& GetResolution() const;
		GLORY_API void GetResolution(uint32_t& width, uint32_t& height) const;

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

		GLORY_API void AddRender(UUID textMeshID, UUID textureID, glm::mat4&& world, const glm::vec4& color);
		GLORY_API void BeginMask(glm::mat4&& world);
		GLORY_API void EndMask();
		GLORY_API void CreateRenderPasses(GraphicsDevice* pDevice, size_t imageCount, const glm::uvec2& resolution, UIRendererModule* pUIRenderer);
		GLORY_API void ResizeRenderTexture(GraphicsDevice* pDevice, size_t imageCount, const glm::uvec2& resolution, UIRendererModule* pUIRenderer);
		GLORY_API void SetClearColor(const glm::vec4& color);

		GLORY_API TextureHandle GetUITexture(GraphicsDevice* pDevice, uint32_t frameIndex);

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
		std::vector<RenderPassHandle> m_UIPasses;
		std::vector<DescriptorSetHandle> m_UIOverlaySets;
		glm::uvec2 m_Resolution;
		UIRendererModule* m_pRenderer;
		glm::mat4 m_Projection;
		glm::vec2 m_CursorPos;
		glm::vec2 m_CursorScrollDelta;
		glm::vec4 m_ClearColor{ 0.0f, 0.0f, 0.0f, 0.0f };
		bool m_CursorDown;
		bool m_WasCursorDown;
		bool m_InputEnabled;
		size_t m_PanelCounter;
		Utils::BitSet m_DrawIsDirty;

		std::map<UUID, std::unique_ptr<MeshData>> m_pTextMeshes;
		std::string m_Name;
		std::map<UUID, Utils::ECS::EntityID> m_Ids;
		std::map<Utils::ECS::EntityID, UUID> m_UUIds;
		std::map<Utils::ECS::EntityID, std::string> m_Names;

		UIBatch m_UIBatch;
	};
}
