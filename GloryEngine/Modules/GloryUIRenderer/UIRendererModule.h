#pragma once
#include <RendererModule.h>
#include <FileData.h>
#include <glm/glm.hpp>

namespace Glory
{
	class MaterialData;
	class RenderTexture;
	class UIDocumentData;

	namespace Utils::ECS
	{
		class ComponentTypes;
	}

	struct UIRenderData
	{
		UUID m_DocumentID;
		UUID m_ObjectID;
	};

	class UIRendererModule : public Module
	{
	public:
		UIRendererModule();
		virtual ~UIRendererModule();

		virtual void CollectReferences(std::vector<UUID>& references) override;

		virtual const std::type_info& GetModuleType() override;

		Utils::ECS::ComponentTypes* GetComponentTypes() { return m_pComponentTypes; }

		void Submit(UIRenderData&& data);

		GLORY_MODULE_VERSION_H(0,1,0);

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;
		virtual void PostInitialize() override;
		virtual void Draw() override;

		virtual void LoadSettings(ModuleSettings& settings) override;

		RenderTexture* GetRenderTexture(UUID id);

	private:
		MaterialData* m_pUIMaterial = nullptr;

		Utils::ECS::ComponentTypes* m_pComponentTypes = nullptr;

		std::map<UUID, std::unique_ptr<MeshData>> m_pTextMeshes;
		std::map<UUID, RenderTexture*> m_pRenderTextures;

		std::vector<UIRenderData> m_Frame;
	};
}
