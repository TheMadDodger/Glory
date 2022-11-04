//#include "CameraComponentEditor.h"
//#include <imgui.h>
//#include <LayerManager.h>
//
//namespace Glory::Editor
//{
//	CameraComponentEditor::CameraComponentEditor()
//	{
//	}
//
//	CameraComponentEditor::~CameraComponentEditor()
//	{
//	}
//
//	void CameraComponentEditor::Initialize()
//	{
//		EntityComponentEditor<CameraComponentEditor, CameraComponent>::Initialize();
//		m_LayerOptions.clear();
//		LayerManager::GetAllLayerNames(m_LayerOptions);
//
//		CameraComponent& camera = GetTargetComponent();
//		m_LayerText = LayerManager::LayerMaskToString(camera.m_LayerMask);
//	}
//
//	bool CameraComponentEditor::OnGUI()
//	{
//		CameraComponent& camera = GetTargetComponent();
//
//		bool change = false;
//		change |= ImGui::InputFloat("Half FOV", &camera.m_HalfFOV);
//		change |= ImGui::InputFloat("Near Plane", &camera.m_Near);
//		change |= ImGui::InputFloat("Far Plane", &camera.m_Far);
//		change |= ImGui::InputInt("Display Index", &camera.m_DisplayIndex);
//		change |= ImGui::InputInt("Priority", &camera.m_Priority);
//		change |= ImGui::InputFloat4("Clear Color", (float*)&camera.m_ClearColor);
//
//		if (ImGui::BeginCombo("Layer Mask", m_LayerText.data()))
//		{
//			for (size_t i = 0; i < m_LayerOptions.size(); i++)
//			{
//				const Layer* pLayer = LayerManager::GetLayerAtIndex(i - 1);
//
//				bool selected = pLayer == nullptr ? camera.m_LayerMask == 0 : (camera.m_LayerMask & pLayer->m_Mask) > 0;
//				if (ImGui::Selectable(m_LayerOptions[i].data(), selected))
//				{
//					if (pLayer == nullptr)
//						camera.m_LayerMask = 0;
//					else
//						camera.m_LayerMask ^= pLayer->m_Mask;
//
//					m_LayerText = LayerManager::LayerMaskToString(camera.m_LayerMask);
//					change = true;
//				}
//			}
//
//			ImGui::EndCombo();
//		}
//		return change;
//	}
//
//	std::string CameraComponentEditor::Name()
//	{
//		return "Camera";
//	}
//}
