#pragma once
#include "UIComponents.h"

#include <ComponentManager.h>

namespace Glory
{
	class UIRendererModule;
	class LocalizeModuleBase;
	class Resources;

	class UITransformManager : public Utils::ECS::ComponentManager<UITransform>
	{
	public:
		UITransformManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity=100);
        virtual ~UITransformManager();

	public:
		void OnPostUpdateImpl(Utils::ECS::EntityID entity, UITransform& pComponent, float);
		static void CalculateMatrix(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UITransform& pComponent, bool calculateParentIfDirty=true);
		static bool ProcessConstraints(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UITransform& pComponent);

	private:
		void OnInitialize() override;
	};

	class UIImageManager : public Utils::ECS::ComponentManager<UIImage>
	{
	public:
		UIImageManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity = 100);
		virtual ~UIImageManager();

	public:
		void OnDrawImpl(Utils::ECS::EntityID entity, UIImage& pComponent);
		void GetReferencesImpl(std::vector<UUID>& references) const;
		void OnDeserialize(Utils::BinaryStream&) override;
		void OnCopy(UIImage& image) override;

	private:
		void OnInitialize() override;
	};

	class UITextManager : public Utils::ECS::ComponentManager<UIText>
	{
	public:
		UITextManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity = 100);
		virtual ~UITextManager();

	public:
		void OnStartImpl(Utils::ECS::EntityID entity, UIText& pComponent);
		void OnDrawImpl(Utils::ECS::EntityID entity, UIText& pComponent);
		void OnDirtyImpl(Utils::ECS::EntityID entity, UIText& pComponent);
		void GetReferencesImpl(std::vector<UUID>& references) const;
		void OnDeserialize(Utils::BinaryStream&) override;
		void OnCopy(UIText& text) override;
		void UnpackDataInto(const void* data, UIText& newComponent) override;
		void SerializeDense(Utils::BinaryStream& stream) const override;
		void DeserializeDense(Utils::BinaryStream& stream) override;

	private:
		void OnInitialize() override;

	private:
		friend class UIRendererModule;
		UIRendererModule* m_pRenderer;
		LocalizeModuleBase* m_pLocalizeModule;
		Resources* m_pResources;
	};
	
	class UIBoxManager : public Utils::ECS::ComponentManager<UIBox>
	{
	public:
		UIBoxManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity = 100);
		virtual ~UIBoxManager();

	public:
		void OnDrawImpl(Utils::ECS::EntityID entity, UIBox& pComponent);

	private:
		void OnInitialize() override;
	};
	
	class UIInteractionManager : public Utils::ECS::ComponentManager<UIInteraction>
	{
	public:
		UIInteractionManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity = 100);
		virtual ~UIInteractionManager();

	public:
		void OnUpdateImpl(Utils::ECS::EntityID entity, UIInteraction& pComponent, float);

	private:
		void OnInitialize() override;

	private:
		friend class UIRendererModule;
		UIRendererModule* m_pRenderer;
	};

	class UIPanelManager : public Utils::ECS::ComponentManager<UIPanel>
	{
	public:
		UIPanelManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity = 100);
		virtual ~UIPanelManager();

	public:
		void OnDrawImpl(Utils::ECS::EntityID entity, UIPanel& pComponent);
		void OnPostDrawImpl(Utils::ECS::EntityID entity, UIPanel& pComponent);

	private:
		void OnInitialize() override;
	};

	class UIVerticalContainerManager : public Utils::ECS::ComponentManager<UIVerticalContainer>
	{
	public:
		UIVerticalContainerManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity = 100);
		virtual ~UIVerticalContainerManager();

	public:
		void OnPreUpdateImpl(Utils::ECS::EntityID entity, UIVerticalContainer& pComponent, float);
		void OnDirtyImpl(Utils::ECS::EntityID entity, UIVerticalContainer& pComponent);

	private:
		void OnInitialize() override;
	};
	
	class UIScrollViewManager : public Utils::ECS::ComponentManager<UIScrollView>
	{
	public:
		UIScrollViewManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity = 100);
		virtual ~UIScrollViewManager();

	public:
		void OnStartImpl(Utils::ECS::EntityID entity, UIScrollView& pComponent);
		void OnValidateImpl(Utils::ECS::EntityID entity, UIScrollView& pComponent);
		void OnPreUpdateImpl(Utils::ECS::EntityID entity, UIScrollView& pComponent, float dt);
		void OnUpdateImpl(Utils::ECS::EntityID entity, UIScrollView& pComponent, float);
		void OnDirtyImpl(Utils::ECS::EntityID entity, UIScrollView& pComponent);

	private:
		void OnInitialize() override;
	};
}
