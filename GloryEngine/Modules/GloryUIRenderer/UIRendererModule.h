#pragma once
#include "UIComponents.h"

#include <RendererModule.h>
#include <Glory.h>
#include <FileData.h>

#include <glm/glm.hpp>

namespace Glory
{
	class MaterialData;
	class RenderTexture;
	class UIDocumentData;
	class FontData;
	class UIDocument;

	namespace Utils::ECS
	{
		class ComponentTypes;
	}

	struct UIRenderData
	{
		UUID m_DocumentID;
		UUID m_SceneID;
		UUID m_ObjectID;

		UITarget m_Target;
		UUID m_TargetCamera;
		glm::uvec2 m_Resolution;
		glm::vec2 m_WorldSize;
		glm::vec2 m_CursorPos;
		glm::vec2 m_CursorScrollDelta;
		bool m_CursorDown;
		glm::mat4 m_WorldTransform;
		LayerMask m_LayerMask;
		UUID m_MaterialID;
		bool m_WorldDirty;
		bool m_InputEnabled;
	};

	class UIRendererModule : public Module
	{
	public:
		GLORY_API UIRendererModule();
		GLORY_API virtual ~UIRendererModule();

		GLORY_API virtual void CollectReferences(std::vector<UUID>& references) override;

		GLORY_API virtual const std::type_info& GetModuleType() override;

		GLORY_API Utils::ECS::ComponentTypes* GetComponentTypes() { return m_pComponentTypes; }

		GLORY_API void Submit(UIRenderData&& data);
		GLORY_API void Create(const UIRenderData& data, UIDocumentData* pDocument);

		GLORY_API void DrawDocument(UIDocument* pDocument, const UIRenderData& data);

		GLORY_API MaterialData* PrepassStencilMaterial();
		GLORY_API MaterialData* PrepassMaterial();
		GLORY_API MaterialData* TextPrepassMaterial();
		GLORY_API MeshData* GetImageMesh();

		GLORY_API UIDocument* FindDocument(UUID uuid);

		GLORY_MODULE_VERSION_H(0,2,0);

	private:
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void Update() override;
		virtual void Cleanup() override;
		virtual void UIPrepass(CameraRef, const RenderFrame&);
		virtual void UIWorldSpaceQuadPass(CameraRef camera, const RenderFrame&);
		virtual void UIOverlayPass(CameraRef camera, const RenderFrame&);

		virtual void LoadSettings(ModuleSettings& settings) override;

		UIDocument& GetDocument(const UIRenderData& data, UIDocumentData* pDocument, bool forceCreate=false);

		MeshData* GetDocumentQuadMesh(const UIRenderData& data);

	private:
		MaterialData* m_pUIPrepassStencilMaterial = nullptr;
		MaterialData* m_pUIPrepassMaterial = nullptr;
		MaterialData* m_pUITextPrepassMaterial = nullptr;
		MaterialData* m_pUIOverlayMaterial = nullptr;

		Utils::ECS::ComponentTypes* m_pComponentTypes = nullptr;

		std::map<UUID, UIDocument> m_Documents;

		std::vector<UIRenderData> m_Frame;
		std::unique_ptr<MeshData> m_pImageMesh;
		std::map<UUID, std::unique_ptr<MeshData>> m_pDocumentQuads;
	};
}
