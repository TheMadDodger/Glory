//#include "StandardPropertyDrawers.h"
//
//namespace Spartan::Editor
//{
//	bool FloatDrawer::OnGUI(Serialization::SerializedProperty& prop) const
//	{
//		return ImGui::InputFloat(prop.m_Name.c_str(), (float*)prop.m_MemberPointer);
//	}
//
//	bool IntDrawer::OnGUI(Serialization::SerializedProperty& prop) const
//	{
//		return ImGui::InputInt(prop.m_Name.c_str(), (int*)prop.m_MemberPointer);
//	}
//
//	bool BoolDrawer::OnGUI(Serialization::SerializedProperty& prop) const
//	{
//		return ImGui::Checkbox(prop.m_Name.c_str(), (bool*)prop.m_MemberPointer);
//	}
//
//	bool DoubleDrawer::OnGUI(Serialization::SerializedProperty& prop) const
//	{
//		return ImGui::InputDouble(prop.m_Name.c_str(), (double*)prop.m_MemberPointer);
//	}
//
//	bool Vector2Drawer::OnGUI(Serialization::SerializedProperty& prop) const
//	{
//		return ImGui::InputFloat2(prop.m_Name.c_str(), (float*)prop.m_MemberPointer);
//	}
//	bool Vector3Drawer::OnGUI(Serialization::SerializedProperty& prop) const
//	{
//		return ImGui::InputFloat3(prop.m_Name.c_str(), (float*)prop.m_MemberPointer);
//	}
//	bool Vector4Drawer::OnGUI(Serialization::SerializedProperty& prop) const
//	{
//		return ImGui::InputFloat4(prop.m_Name.c_str(), (float*)prop.m_MemberPointer);
//	}
//	bool ColorDrawer::OnGUI(Serialization::SerializedProperty& prop) const
//	{
//		return ImGui::ColorPicker4(prop.m_Name.c_str(), (float*)prop.m_MemberPointer);
//	}
//	bool QuaternionDrawer::OnGUI(Serialization::SerializedProperty& prop) const
//	{
//		return ImGui::InputFloat4(prop.m_Name.c_str(), (float*)prop.m_MemberPointer);
//	}
//}