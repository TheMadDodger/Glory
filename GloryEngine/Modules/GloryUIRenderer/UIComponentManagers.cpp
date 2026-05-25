#include "UIComponentManagers.h"
#include "UIComponents.h"
#include "UIDocument.h"
#include "UIRendererModule.h"
#include "Constraints.h"

#include <GScene.h>
#include <IEngine.h>
#include <GameTime.h>
#include <FontData.h>
#include <SceneManager.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/glm.hpp>
#include <LocalizeModuleBase.h>

namespace Glory
{
	UITransformManager::UITransformManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity):
		ComponentManager(pRegistry, capacity)
	{
	}

	UITransformManager::~UITransformManager()
	{
	}

	void UITransformManager::OnPostUpdateImpl(Utils::ECS::EntityID entity, UITransform& pComponent, float)
	{
        if (!m_pRegistry->IsEntityDirty(entity)) return;
        CalculateMatrix(m_pRegistry, entity, pComponent);
	}

	bool UITransformManager::ProcessConstraints(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UITransform& pComponent)
	{
		bool change = false;

		UIDocument* pDocument = pRegistry->GetUserData<UIDocument>();
		uint32_t width, height;
		pDocument->GetResolution(width, height);
		const glm::vec2 screenSize{ float(width), float(height) };

		pComponent.m_ParentSize = screenSize;

		const Utils::ECS::EntityID parent = pRegistry->GetParent(entity);

		if (pRegistry->EntityValid(parent))
		{
			UITransform& parentTransform = pRegistry->GetComponent<UITransform>(parent);
			pComponent.m_ParentSize = { parentTransform.m_Width, parentTransform.m_Height };
		}

		change |= Constraints::ProcessConstraint(pComponent.m_Width, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, pComponent.m_ParentSize, screenSize);
		change |= Constraints::ProcessConstraint(pComponent.m_Height, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, pComponent.m_ParentSize, screenSize);
		change |= Constraints::ProcessConstraint(pComponent.m_Width, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, pComponent.m_ParentSize, screenSize);
		change |= Constraints::ProcessConstraint(pComponent.m_X, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, pComponent.m_ParentSize, screenSize);
		change |= Constraints::ProcessConstraint(pComponent.m_Y, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, pComponent.m_ParentSize, screenSize);
		change |= Constraints::ProcessConstraint(pComponent.m_X, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, pComponent.m_ParentSize, screenSize);
		return change;
	}

	void UITransformManager::OnInitialize()
	{
		Bind(DoPostUpdate, &UITransformManager::OnPostUpdateImpl);
	}

    void UITransformManager::CalculateMatrix(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UITransform& pComponent, bool calculateParentIfDirty)
    {
		UIDocument* pDocument = pRegistry->GetUserData<UIDocument>();
		glm::mat4 startTransform = glm::identity<glm::mat4>();
		glm::mat4 startInteractionTransform = glm::identity<glm::mat4>();

        const Utils::ECS::EntityID parent = pRegistry->GetParent(entity);

        if (pRegistry->EntityValid(parent))
        {
			if (pRegistry->IsEntityDirty(parent) && calculateParentIfDirty)
				CalculateMatrix(pRegistry, parent, pRegistry->GetComponent<UITransform>(parent));

			UITransform& parentTransform = pRegistry->GetComponent<UITransform>(parent);
			startTransform = parentTransform.m_TransformNoScaleNoPivot;
			startInteractionTransform = parentTransform.m_InteractionTransformNoPivot;
        }

		ProcessConstraints(pRegistry, entity, pComponent);

		/* Conversion top to bottom rather than bottom to top */
		const float actualY = parent ? -float(pComponent.m_Y) : pComponent.m_ParentSize.y - float(pComponent.m_Y);
		const float actualYPivot = 1.0f - pComponent.m_Pivot.y;

		const glm::vec2 size{ pComponent.m_Width, pComponent.m_Height };
		const glm::mat4 rotation = glm::rotate(glm::identity<glm::mat4>(), -glm::radians(pComponent.m_Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
		const glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), glm::vec3(pComponent.m_X, actualY, 0.0f));
		const glm::mat4 interactionTranslation = glm::translate(glm::identity<glm::mat4>(), glm::vec3(pComponent.m_X, pComponent.m_Y, 0.0f));
		const glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), glm::vec3(size.x, size.y, 1.0f));
		const glm::mat4 selfScale = glm::scale(glm::identity<glm::mat4>(), glm::vec3(pComponent.m_Scale.x, pComponent.m_Scale.y, 1.0f));
		const glm::mat4 pivotOffset = glm::translate(glm::identity<glm::mat4>(), glm::vec3(pComponent.m_Pivot.x*size.x, actualYPivot*size.y, 0.0f));
		const glm::mat4 interactionPivotOffset = glm::translate(glm::identity<glm::mat4>(), glm::vec3(pComponent.m_Pivot.x*size.x, pComponent.m_Pivot.y*size.y, 0.0f));
        pComponent.m_Transform = startTransform*translation*rotation*selfScale*glm::inverse(pivotOffset)*scale;
        pComponent.m_TransformNoScale = startTransform*translation*rotation*selfScale*glm::inverse(pivotOffset);
        pComponent.m_TransformNoScaleNoPivot = startTransform*translation*rotation*selfScale;
        pComponent.m_InteractionTransform = startInteractionTransform*interactionTranslation*rotation*glm::inverse(interactionPivotOffset)*selfScale;
        pComponent.m_InteractionTransformNoPivot = startInteractionTransform*interactionTranslation*rotation*selfScale;

        pRegistry->SetEntityDirty(entity, false);
		pDocument->SetDrawDirty();
    }

	UIImageManager::UIImageManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity):
		ComponentManager(pRegistry, capacity)
	{
	}

	UIImageManager::~UIImageManager()
	{
	}

	void UIImageManager::OnDrawImpl(Utils::ECS::EntityID entity, UIImage& pComponent)
	{
		UIDocument* pDocument = m_pRegistry->GetUserData<UIDocument>();
		const UITransform& transform = m_pRegistry->GetComponent<UITransform>(entity);
		glm::mat4 world = transform.m_Transform;
		pDocument->AddRender(0ull, pComponent.m_Image.GetUUID(), std::move(world), pComponent.m_Color);
	}

	void UIImageManager::GetReferencesImpl(std::vector<UUID>& references) const
	{
		for (size_t i = 0; i < Size(); ++i)
		{
			const UIImage& uiImage = GetAt(i);
			const UUID image = uiImage.m_Image.GetUUID();
			if (image) references.push_back(image);
		}
	}

	void UIImageManager::OnDeserialize(Utils::BinaryStream&)
	{
		for (size_t i = 0; i < Size(); ++i)
		{
			const UIImage& image = GetAt(i);
			image.m_Image.ManualRegisterReference();
		}
	}

	void UIImageManager::OnInitialize()
	{
		Bind(DoDraw, &UIImageManager::OnDrawImpl);
		Bind(DoGetReferences, &UIImageManager::GetReferencesImpl);
	}

	UITextManager::UITextManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity):
		ComponentManager(pRegistry, capacity), m_pRenderer(nullptr),
		m_pLocalizeModule(nullptr), m_pResources(nullptr)
	{
	}

	UITextManager::~UITextManager()
	{
	}

	void UITextManager::OnStartImpl(Utils::ECS::EntityID entity, UIText& pComponent)
	{
		if (pComponent.m_LocalizeTerm.empty()) return;

		UIDocument* pDocument = m_pRegistry->GetUserData<UIDocument>();
		if (!m_pLocalizeModule) return;

		const std::string_view fullTerm = pComponent.m_LocalizeTerm;
		const size_t firstDot = fullTerm.find('.');
		if (firstDot == std::string::npos) return;
		const std::string_view tableName = fullTerm.substr(0, firstDot);
		const std::string_view term = fullTerm.substr(firstDot + 1);
		if (m_pLocalizeModule->FindString(tableName, term, pComponent.m_Text))
		{
			pComponent.m_Dirty = true;
			pDocument->SetDrawDirty();
		}
	}

	void UITextManager::OnDrawImpl(Utils::ECS::EntityID entity, UIText& pComponent)
    {
		/* No need to do anything if there is no text */
		if (pComponent.m_Text.empty()) return;
        UIDocument* pDocument = m_pRegistry->GetUserData<UIDocument>();
		const UUID objectID = pDocument->EntityUUID(entity);

		const UITransform& transform = m_pRegistry->GetComponent<UITransform>(entity);
		const glm::vec2 size{ transform.m_Width, transform.m_Height };

		uint32_t screenWidth, screenHeight;
		pDocument->GetResolution(screenWidth, screenHeight);
		const glm::vec2 screenSize{ float(screenWidth), float(screenHeight) };

		Constraints::ProcessConstraint(pComponent.m_Scale, size, transform.m_ParentSize, screenSize);

		TextData textData;
		textData.m_Text = pComponent.m_Text;
		textData.m_TextDirty = pComponent.m_Dirty;
		textData.m_Scale = float(pComponent.m_Scale);
		textData.m_Alignment = pComponent.m_Alignment;
		textData.m_TextWrap = float(transform.m_Width)*float(pComponent.m_Scale);
		textData.m_Color = pComponent.m_Color;
		pComponent.m_Dirty = false;

		FontData* pFont = pComponent.m_Font.Get(m_pResources);
		if (!pFont) return;
		const UUID meshID = pDocument->GetTextMesh(objectID, textData, pFont);

		const uint32_t height = pFont->FontHeight();
		glm::vec2 textOffset{ 0.0f, height*textData.m_Scale - size.y };
		switch (textData.m_Alignment)
		{
		case Alignment::Left:
			break;
		case Alignment::Center:
			textOffset.x = -size.x/2.0f;
			break;
		case Alignment::Right:
			textOffset.x = -size.x;
			break;
		default:
			break;
		}

		const glm::mat4 matTextOffset = glm::translate(glm::identity<glm::mat4>(), glm::vec3(textOffset, 0.0f));
		glm::mat4 world = transform.m_TransformNoScale*glm::inverse(matTextOffset);
		pDocument->AddRender(meshID, pFont->Texture(), std::move(world), textData.m_Color);
    }

	void UITextManager::OnDirtyImpl(Utils::ECS::EntityID entity, UIText& pComponent)
	{
		pComponent.m_Dirty = true;
	}

	void UITextManager::GetReferencesImpl(std::vector<UUID>& references) const
	{
		for (size_t i = 0; i < Size(); ++i)
		{
			const UIText& uiText = GetAt(i);
			const UUID font = uiText.m_Font.GetUUID();
			if (font) references.push_back(font);
		}
	}

	void UITextManager::OnDeserialize(Utils::BinaryStream&)
	{
		for (size_t i = 0; i < Size(); ++i)
		{
			const UIText& text = GetAt(i);
			text.m_Font.ManualRegisterReference();
		}
	}

	void UITextManager::OnInitialize()
	{
		Bind(DoStart, &UITextManager::OnStartImpl);
		Bind(DoDraw, &UITextManager::OnDrawImpl);
		Bind(DoOnDirty, &UITextManager::OnDirtyImpl);
		Bind(DoGetReferences, &UITextManager::GetReferencesImpl);
	}

	UIBoxManager::UIBoxManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity) :
		ComponentManager(pRegistry, capacity)
	{
	}

	UIBoxManager::~UIBoxManager()
	{
	}

	void UIBoxManager::OnDrawImpl(Utils::ECS::EntityID entity, UIBox& pComponent)
	{
		UIDocument* pDocument = m_pRegistry->GetUserData<UIDocument>();
		const UITransform& transform = m_pRegistry->GetComponent<UITransform>(entity);
		glm::mat4 world = transform.m_Transform;
		pDocument->AddRender(0, 0ull, std::move(world), pComponent.m_Color);
	}

	void UIBoxManager::OnInitialize()
	{
		Bind(DoDraw, &UIBoxManager::OnDrawImpl);
	}

	UIInteractionManager::UIInteractionManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity):
		ComponentManager(pRegistry, capacity), m_pRenderer(nullptr)
	{
	}

	UIInteractionManager::~UIInteractionManager()
	{
	}

	void UIInteractionManager::OnUpdateImpl(Utils::ECS::EntityID entity, UIInteraction& pComponent, float)
	{
		if (!pComponent.m_Enabled) return;

		UIDocument* pDocument = m_pRegistry->GetUserData<UIDocument>();
		const bool inputAllowed = pDocument->IsEnputEnabled();
		if (!inputAllowed) return;

		const UITransform& transform = m_pRegistry->GetComponent<UITransform>(entity);

		const glm::vec4 cursor{ pDocument->GetCursorPos(), 0.0f, 1.0f };
		const glm::mat4 inverse = glm::inverse(transform.m_InteractionTransform);
		const glm::mat4 screenScaleTransform = glm::scale(glm::identity<glm::mat4>(), {});
		const glm::vec4 transformedCursor = inverse*cursor;

		const bool isMouseInRect = transformedCursor.x > 0.0f && transformedCursor.x < float(transform.m_Width) &&
			transformedCursor.y > 0.0f && transformedCursor.y < float(transform.m_Height);

		const UUID entityUUID = pDocument->EntityUUID(entity);
		const UUID componentID = m_pRegistry->EntityComponentHashToID(entity, UIInteraction::GetTypeData()->TypeHash());
		const UUID sceneID = pDocument->SceneID();
		const UUID objectID = pDocument->ObjectID();

		const bool wasDown = pDocument->WasCursorDown();
		const bool isCursorDown = pDocument->IsCursorDown();

		IEngine* pEngine = m_pRenderer->GetEngine();
		if (isMouseInRect && !pComponent.m_Hovered)
		{
			pComponent.m_Hovered = true;
			if (!m_pRenderer->OnElementHover_Callback) return;
			m_pRenderer->OnElementHover_Callback(pEngine, sceneID, objectID, entityUUID, componentID);
		}
		else if (!isMouseInRect && pComponent.m_Hovered)
		{
			pComponent.m_Hovered = false;
			if (!m_pRenderer->OnElementUnHover_Callback) return;
			m_pRenderer->OnElementUnHover_Callback(pEngine, sceneID, objectID, entityUUID, componentID);
		}

		if (!wasDown && isCursorDown && pComponent.m_Hovered && !pComponent.m_Down)
		{
			pComponent.m_Down = true;
			if (!m_pRenderer->OnElementDown_Callback) return;
			m_pRenderer->OnElementDown_Callback(pEngine, sceneID, objectID, entityUUID, componentID);
		}
		else if (!isCursorDown && pComponent.m_Down || pComponent.m_Down && !pComponent.m_Hovered)
		{
			pComponent.m_Down = false;
			if (!m_pRenderer->OnElementUp_Callback) return;
			m_pRenderer->OnElementUp_Callback(pEngine, sceneID, objectID, entityUUID, componentID);
		}
	}

	void UIInteractionManager::OnInitialize()
	{
		Bind(DoUpdate, &UIInteractionManager::OnUpdateImpl);
	}

	UIPanelManager::UIPanelManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity):
		ComponentManager(pRegistry, capacity)
	{
	}

	UIPanelManager::~UIPanelManager()
	{
	}

	void UIPanelManager::OnDrawImpl(Utils::ECS::EntityID entity, UIPanel& pComponent)
	{
		if (!pComponent.m_Crop) return;
		UIDocument* pDocument = m_pRegistry->GetUserData<UIDocument>();
		const UITransform& transform = m_pRegistry->GetComponent<UITransform>(entity);
		glm::mat4 world = transform.m_Transform;
		pDocument->BeginMask(std::move(world));
	}

	void UIPanelManager::OnPostDrawImpl(Utils::ECS::EntityID entity, UIPanel& pComponent)
	{
		if (!pComponent.m_Crop) return;
		UIDocument* pDocument = m_pRegistry->GetUserData<UIDocument>();
		pDocument->EndMask();
	}

	void UIPanelManager::OnInitialize()
	{
		Bind(DoDraw, &UIPanelManager::OnDrawImpl);
		Bind(DoPostDraw, &UIPanelManager::OnPostDrawImpl);
	}

	//template<typename Comp>
	//static void UpdateEntity(Utils::ECS::EntityID entity, Utils::ECS::EntityRegistry& registry, Utils::ECS::InvocationType invocation)
	//{
	//	Utils::ECS::TypeView<Comp>* pTypeView = registry.GetTypeView<Comp>();
	//	pTypeView->InvokeAll(invocation, &registry, { entity });
	//	for (size_t i = 0; i < registry.ChildCount(entity); ++i)
	//	{
	//		const Utils::ECS::EntityID child = registry.Child(entity, i);
	//		UpdateEntity<Comp>(child, registry, invocation);
	//	}
	//}

	//static void UpdateEntity(Utils::ECS::EntityID entity, Utils::ECS::EntityRegistry& registry, Utils::ECS::InvocationType invocation)
	//{
	//	registry.InvokeAll(invocation, { entity });
	//	for (size_t i = 0; i < registry.ChildCount(entity); ++i)
	//	{
	//		const Utils::ECS::EntityID child = registry.Child(entity, i);
	//		UpdateEntity(child, registry, invocation);
	//	}
	//}

	UIVerticalContainerManager::UIVerticalContainerManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity):
		ComponentManager(pRegistry, capacity)
	{
	}

	UIVerticalContainerManager::~UIVerticalContainerManager()
	{
	}

	void UIVerticalContainerManager::OnPreUpdateImpl(Utils::ECS::EntityID entity, UIVerticalContainer& pComponent, float)
	{
		if (!pComponent.m_Dirty) return;
		float height = 0.0f;

		/* Make sure all children were processed first in case they will be resized */
		//for (size_t i = 0; i < pEntity->ChildCount(); ++i)
		//{
		//	const Utils::ECS::EntityID child = pEntity->Child(i);
		//	UpdateEntity(child, *pRegistry, Utils::ECS::InvocationType::PreUpdate);
		//}

		/* Update child sizes */
		for (size_t i = 0; i < m_pRegistry->ChildCount(entity); ++i)
		{
			const Utils::ECS::EntityID child = m_pRegistry->Child(entity, i);
			UITransform& transform = m_pRegistry->GetComponent<UITransform>(child);
			const float elementHeight = transform.m_Height.m_FinalValue;
			transform.m_Y.m_Constraint = 0;
			transform.m_Y.m_Value = height;
			height += elementHeight + pComponent.m_Seperation;
			if (UITransformManager::ProcessConstraints(m_pRegistry, child, transform))
				m_pRegistry->SetEntityDirty(child);
		}

		/* Resize self */
		UITransform& transform = m_pRegistry->GetComponent<UITransform>(entity);
		if (pComponent.m_AutoResizeHeight && transform.m_Height.m_FinalValue != height)
		{
			transform.m_Height.m_Constraint = 0;
			transform.m_Height = height;
			if (UITransformManager::ProcessConstraints(m_pRegistry, entity, transform))
				m_pRegistry->SetEntityDirty(entity);
		}
		pComponent.m_Dirty = false;
	}

	void UIVerticalContainerManager::OnDirtyImpl(Utils::ECS::EntityID entity, UIVerticalContainer& pComponent)
	{
		pComponent.m_Dirty = true;
	}

	void UIVerticalContainerManager::OnInitialize()
	{
		Bind(DoPreUpdate, &UIVerticalContainerManager::OnPreUpdateImpl);
		Bind(DoOnDirty, &UIVerticalContainerManager::OnDirtyImpl);
	}

	UIScrollViewManager::UIScrollViewManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity):
		ComponentManager(pRegistry, capacity)
	{
	}

	UIScrollViewManager::~UIScrollViewManager()
	{
	}

	void UIScrollViewManager::OnStartImpl(Utils::ECS::EntityID entity, UIScrollView& pComponent)
	{
		pComponent.m_ScrollPosition = pComponent.m_StartScrollPosition;
		pComponent.m_Dirty = true;
	}

	void UIScrollViewManager::OnValidateImpl(Utils::ECS::EntityID entity, UIScrollView& pComponent)
	{
		pComponent.m_ScrollPosition = pComponent.m_StartScrollPosition;
		pComponent.m_Dirty = true;
	}

	void UIScrollViewManager::OnPreUpdateImpl(Utils::ECS::EntityID entity, UIScrollView& pComponent, float dt)
	{
		if (!m_pRegistry->HasComponent<UIInteraction>(entity)) return;
		UIInteraction& interaction = m_pRegistry->GetComponent<UIInteraction>(entity);
		UIDocument* pDocument = m_pRegistry->GetUserData<UIDocument>();

		const glm::vec2& cursorScrollDelta = pDocument->GetCursorScrollDelta();
		const float length = glm::length(cursorScrollDelta);
		if ((!pComponent.m_RequireHover || interaction.m_Hovered) && length != 0.0f)
		{
			if (!pComponent.m_LockHorizontal)
				pComponent.m_DesiredScrollPosition.x += pDocument->GetCursorScrollDelta().x*pComponent.m_ScrollSpeeds.x;
			if (!pComponent.m_LockVertical)
				pComponent.m_DesiredScrollPosition.y += pDocument->GetCursorScrollDelta().y*pComponent.m_ScrollSpeeds.y;
			pComponent.m_Dirty = true;
		}

		switch (pComponent.m_ScrollEdgeMode)
		{
		case ScrollEdgeMode::Clamp:
			pComponent.m_DesiredScrollPosition = glm::clamp(pComponent.m_DesiredScrollPosition, glm::vec2{ 0.0f, 0.0f }, pComponent.m_MaxScroll);
			break;
		case ScrollEdgeMode::Innertia:
		{
			const glm::vec2 target = glm::clamp(pComponent.m_DesiredScrollPosition, glm::vec2{ 0.0f, 0.0f }, pComponent.m_MaxScroll);
			pComponent.m_DesiredScrollPosition = glm::mix(pComponent.m_DesiredScrollPosition, target,
				pComponent.m_Innertia*dt);
			break;
		}
		default:
			break;
		}

		if (pComponent.m_ScrollPosition != pComponent.m_DesiredScrollPosition)
		{
			switch (pComponent.m_ScrollMode)
			{
			case ScrollMode::Snap:
				pComponent.m_ScrollPosition = pComponent.m_DesiredScrollPosition;
				pComponent.m_Dirty = true;
				break;
			case ScrollMode::Lerp:
				pComponent.m_ScrollPosition = glm::mix(pComponent.m_ScrollPosition, pComponent.m_DesiredScrollPosition,
					pComponent.m_LerpSpeed*dt);
				pComponent.m_Dirty = true;
				break;
			default:
				break;
			}
		}
	}

	void UIScrollViewManager::OnUpdateImpl(Utils::ECS::EntityID entity, UIScrollView& pComponent, float)
	{
		if (!pComponent.m_Dirty) return;

		/* Calculate content size */
		glm::vec2 contentSize{ 0.0f };
		glm::vec2 startPos{ 0.0f };
		for (size_t i = 0; i < m_pRegistry->ChildCount(entity); ++i)
		{
			const Utils::ECS::EntityID child = m_pRegistry->Child(entity, i);
			UITransform& childTransform = m_pRegistry->GetComponent<UITransform>(child);
			if (i == 0) startPos = glm::vec2{ childTransform.m_X.m_FinalValue, childTransform.m_Y.m_FinalValue };
			contentSize.x = glm::max(contentSize.x, childTransform.m_X.m_FinalValue - startPos.x + childTransform.m_Width.m_FinalValue);
			contentSize.y = glm::max(contentSize.y, childTransform.m_Y.m_FinalValue - startPos.y + childTransform.m_Height.m_FinalValue);
		}
		UITransform& transform = m_pRegistry->GetComponent<UITransform>(entity);

		const glm::vec2 tempMaxScroll = pComponent.m_MaxScroll;
		pComponent.m_MaxScroll = { glm::max(glm::vec2{0.0f}, contentSize - glm::vec2{ transform.m_Width.m_FinalValue, transform.m_Height.m_FinalValue }) };
		if (tempMaxScroll != pComponent.m_MaxScroll && pComponent.m_AutoScroll)
			pComponent.m_DesiredScrollPosition = pComponent.m_MaxScroll;

		/* Position children */
		for (size_t i = 0; i < m_pRegistry->ChildCount(entity); ++i)
		{
			const Utils::ECS::EntityID child = m_pRegistry->Child(entity, i);
			UITransform& transform = m_pRegistry->GetComponent<UITransform>(child);
			transform.m_X.m_Constraint = 0;
			transform.m_X.m_Value = -pComponent.m_ScrollPosition.x;
			transform.m_Y.m_Constraint = 0;
			transform.m_Y.m_Value = -pComponent.m_ScrollPosition.y;
			if (UITransformManager::ProcessConstraints(m_pRegistry, child, transform))
				m_pRegistry->SetEntityDirty(child);
		}

		pComponent.m_Dirty = false;
	}

	void UIScrollViewManager::OnDirtyImpl(Utils::ECS::EntityID entity, UIScrollView& pComponent)
	{
		pComponent.m_Dirty = true;
	}

	void UIScrollViewManager::OnInitialize()
	{
		Bind(DoStart, &UIScrollViewManager::OnStartImpl);
		Bind(DoValidate, &UIScrollViewManager::OnValidateImpl);
		Bind(DoPreUpdate, &UIScrollViewManager::OnPreUpdateImpl);
		Bind(DoUpdate, &UIScrollViewManager::OnUpdateImpl);
		Bind(DoOnDirty, &UIScrollViewManager::OnDirtyImpl);
	}
}
