#include "SkeletonEditorComponent.h"
//#include <Skeleton.h>
//#include "SkeletonObject.h"
//
//SkeletonEditorComponent::SkeletonEditorComponent() : BaseComponent("Skeleton")
//{
//	m_CanTickInEditor = true;
//	m_CanTickInGame = false;
//}
//
//SkeletonEditorComponent::~SkeletonEditorComponent()
//{
//}
//
//void SkeletonEditorComponent::CustomEditor()
//{
//	if (ImGui::Button("Save"))
//	{
//		// Save the rig
//	}
//	if (ImGui::Button("Update"))
//	{
//		auto skeleton = dynamic_cast<SkeletonObject*>(GetGameObject());
//		if (!skeleton) return;
//
//		skeleton->GetSkeleton()->Initialize();
//	}
//}