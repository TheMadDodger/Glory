#pragma once
#include "UIDocumentData.h"

#include <RendererModule.h>
#include <FileData.h>
#include <glm/glm.hpp>

namespace Glory
{
	class MaterialData;
	class RenderTexture;

	namespace Utils::ECS
	{
		class ComponentTypes;
	}

	class UIRendererModule : public Module
	{
	public:
		UIRendererModule();
		virtual ~UIRendererModule();

		virtual void CollectReferences(std::vector<UUID>& references) override;

		virtual const std::type_info& GetModuleType() override;

		GLORY_MODULE_VERSION_H(0,1,0);

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;
		virtual void PostInitialize() override;
		virtual void Draw() override;

		virtual void LoadSettings(ModuleSettings& settings) override;

	private:
		MaterialData* m_pUIMaterial = nullptr;
		RenderTexture* m_pUITexture = nullptr;

		Utils::ECS::ComponentTypes* m_pComponentTypes = nullptr;

		std::map<UUID, std::unique_ptr<MeshData>> m_pTextMeshes;

		UIDocumentData m_TestDocument;
	};
}
