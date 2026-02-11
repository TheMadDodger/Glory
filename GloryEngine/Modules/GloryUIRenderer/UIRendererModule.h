#pragma once
#include "UIComponents.h"

#include <Module.h>
#include <Renderer.h>
#include <Glory.h>
#include <FileData.h>

#include <glm/glm.hpp>
#include <CameraRef.h>

namespace Glory
{
	class MaterialData;
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
		std::string m_MaterialTextureName = "texSampler";

		glm::vec4 m_ClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
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

		GLORY_API void DrawDocument(GraphicsDevice* pDevice, CommandBufferHandle commandBuffer,
			uint32_t frameIndex, UIDocument* pDocument, const UIRenderData& data);

		GLORY_API MaterialData* PrepassStencilMaterial();
		GLORY_API MaterialData* PrepassMaterial();
		GLORY_API MaterialData* TextPrepassMaterial();
		GLORY_API MeshData* GetImageMesh();

		GLORY_API UIDocument* FindDocument(UUID uuid);
		GLORY_API const DescriptorSetLayoutHandle& UIOverlaySetLayout() const;

		GLORY_MODULE_VERSION_H(0, 3, 0);

	private:
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void Update() override;
		virtual void Cleanup() override;
		virtual void Load() override;

		virtual void UIPrepass(GraphicsDevice* pDevice, CommandBufferHandle commandBuffer, uint32_t frameIndex);
		virtual void UIDataPass(GraphicsDevice* pDevice, Renderer* pRenderer);
		virtual bool UIOverlayPass(GraphicsDevice* pDevice, CameraRef camera, CommandBufferHandle commandBuffer,
			size_t frameIndex, RenderPassHandle renderPass, DescriptorSetHandle ds);

		virtual void LoadSettings(ModuleSettings& settings) override;

		UIDocument& GetDocument(const UIRenderData& data, UIDocumentData* pDocument, bool forceCreate=false);

		MeshData* GetDocumentQuadMesh(const UIRenderData& data);
		std::vector<TextureData*>& GetDocumentTexture(GraphicsDevice* pDevice,
			const UIRenderData& data, UIDocument& document, size_t imageCount);

		void CheckCachedPipelines(GraphicsDevice* pDevice);
		void CheckCachedOverlayPipeline(RenderPassHandle renderPass, GraphicsDevice* pDevice);

	private:
		MaterialData* m_pUIPrepassStencilMaterial = nullptr;
		MaterialData* m_pUIPrepassMaterial = nullptr;
		MaterialData* m_pUITextPrepassMaterial = nullptr;
		MaterialData* m_pUIOverlayMaterial = nullptr;

		Utils::ECS::ComponentTypes* m_pComponentTypes = nullptr;

		std::map<UUID, UIDocument> m_Documents;

		std::vector<UIRenderData> m_Frame;
		std::unique_ptr<MeshData> m_pImageMesh;
		MeshHandle m_ImageMesh = 0;
		std::map<UUID, MeshData*> m_pDocumentQuads;
		std::map<UUID, std::vector<TextureData*>> m_pDocumentTextures;

		struct UIBatchData
		{
			UIBatchData() {};
			virtual ~UIBatchData() {};

			CPUBuffer<glm::vec4> m_Colors;
			CPUBuffer<glm::mat4> m_Worlds;

			BufferHandle m_ColorsBuffers = 0;
			BufferHandle m_WorldsBuffers = 0;
			DescriptorSetHandle m_BuffersSet = 0;
			std::vector<DescriptorSetHandle> m_TextureSets;
			std::vector<UUID> m_LastTextureIDs;
		};

		std::map<UUID, UIBatchData> m_BatchDatas;

		DescriptorSetLayoutHandle m_UIBuffersLayout = 0;
		DescriptorSetLayoutHandle m_UISamplerLayout = 0;
		DescriptorSetLayoutHandle m_UIOverlaySamplerLayout = 0;

		RenderPassHandle m_DummyRenderPass = 0;

		PipelineHandle m_UIPipeline = 0;
		PipelineHandle m_UITextPipeline = 0;
		PipelineHandle m_UIStencilPipeline = 0;
		PipelineHandle m_UIOverlayPipeline = 0;
	};
}
