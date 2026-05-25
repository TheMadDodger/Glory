#pragma once
#include "UIComponents.h"

#include <ComponentManager.h>

namespace Glory
{
	class UIRendererModule;
	class Resources;
	class Debug;
	class InputModule;
	class LayerManager;

	class UIRenderManager : public Utils::ECS::ComponentManager<UIRenderer>
	{
	public:
		UIRenderManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity=10);
		virtual ~UIRenderManager();

	public:
		void OnStartImpl(Utils::ECS::EntityID entity, UIRenderer& pComponent);
		void OnValidateImpl(Utils::ECS::EntityID entity, UIRenderer& pComponent);
		void OnDrawImpl(Utils::ECS::EntityID entity, UIRenderer& pComponent);
		void GetReferencesImpl(std::vector<UUID>& references) const;
		void OnDeserialize(Utils::BinaryStream&) override;

	private:
		void OnInitialize() override;

	private:
		friend class UIRendererModule;
		UIRendererModule* m_pModule;
		Resources* m_pResources;
		Debug* m_pDebug;
		InputModule* m_pInput;
		LayerManager* m_pLayers;
	};
}
