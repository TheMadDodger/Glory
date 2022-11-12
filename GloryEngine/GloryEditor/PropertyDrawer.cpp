#include "PropertyDrawer.h"
#include "AssetReferencePropertyDrawer.h"
#include <imgui.h>
#include <algorithm>

namespace Glory::Editor
{
	std::vector<PropertyDrawer*> PropertyDrawer::m_PropertyDrawers = std::vector<PropertyDrawer*>();

	PropertyDrawer::PropertyDrawer(size_t typeHash) : m_TypeHash(typeHash)
	{
	}

	PropertyDrawer::~PropertyDrawer()
	{
	}

	bool PropertyDrawer::Draw(const SerializedProperty* serializedProperty, const std::string& label, void* data, size_t typeHash, uint32_t flags) const
	{
		ImGui::Text(label.data());
		return false;
	}

	bool PropertyDrawer::Draw(const SerializedProperty* serializedProperty, const std::string& label, std::any& data, uint32_t flags) const
	{
		ImGui::Text(label.data());
		return false;
	}

	bool PropertyDrawer::Draw(const std::string& label, std::vector<char>& buffer, size_t typeHash, size_t offset, size_t size, uint32_t flags) const
	{
		ImGui::Text(label.data());
		return false;
	}

	bool PropertyDrawer::Draw(const std::string& label, void* data, size_t typeHash, uint32_t flags) const
	{
		ImGui::Text(label.data());
		return false;
	}

	bool PropertyDrawer::Draw(const std::string& label, YAML::Node& node, size_t typeHash, uint32_t flags) const
	{
		ImGui::Text(label.data());
		return false;
	}

	bool PropertyDrawer::Draw(const SerializedProperty* serializedProperty) const
	{
		return Draw(serializedProperty, serializedProperty->Name(), serializedProperty->MemberPointer(), serializedProperty->ElementTypeHash(), serializedProperty->Flags());
	}

	void PropertyDrawer::RegisterPropertyDrawer(PropertyDrawer* pDrawer)
	{
		m_PropertyDrawers.push_back(pDrawer);
	}

	bool PropertyDrawer::DrawProperty(const SerializedProperty* serializedProperty, const std::string& label, void* data, size_t typeHash, size_t elementTypeHash, uint32_t flags)
	{
		auto it = std::find_if(m_PropertyDrawers.begin(), m_PropertyDrawers.end(), [&](PropertyDrawer* propertyDrawer)
			{
				return propertyDrawer->GetPropertyTypeHash() == typeHash;
			});

		if (it == m_PropertyDrawers.end())
		{
			//ImGui::Text(prop.m_Name.c_str());
			return false;
		}

		PropertyDrawer* drawer = *it;
		return drawer->Draw(serializedProperty, label, data, elementTypeHash, flags);
	}

	bool PropertyDrawer::DrawProperty(const std::string& label, std::any& data, uint32_t flags)
	{
		size_t typeHash = ResourceType::GetHash(data.type());

		auto it = std::find_if(m_PropertyDrawers.begin(), m_PropertyDrawers.end(), [&](PropertyDrawer* propertyDrawer)
		{
			return propertyDrawer->GetPropertyTypeHash() == typeHash;
		});

		if (it == m_PropertyDrawers.end())
		{
			//ImGui::Text(prop.m_Name.c_str());
			return false;
		}

		PropertyDrawer* drawer = *it;
		return drawer->Draw(nullptr, label, data, flags);
	}

	bool PropertyDrawer::DrawProperty(const SerializedProperty* serializedProperty)
	{
		return DrawProperty(serializedProperty, serializedProperty->Name(), serializedProperty->MemberPointer(), serializedProperty->TypeHash(), serializedProperty->ElementTypeHash(), serializedProperty->Flags());
	}

	bool PropertyDrawer::DrawProperty(const std::string& label, std::vector<char>& buffer, size_t typeHash, size_t offset, size_t size, uint32_t flags)
	{
		auto it = std::find_if(m_PropertyDrawers.begin(), m_PropertyDrawers.end(), [&](PropertyDrawer* propertyDrawer)
		{
			return propertyDrawer->GetPropertyTypeHash() == typeHash;
		});

		if (it == m_PropertyDrawers.end())
		{
			//ImGui::Text(prop.m_Name.c_str());
			return false;
		}

		PropertyDrawer* drawer = *it;
		return drawer->Draw(label, buffer, typeHash, offset, size, flags);
	}

	bool PropertyDrawer::DrawProperty(const std::string& label, void* data, size_t typeHash, uint32_t flags)
	{
		auto it = std::find_if(m_PropertyDrawers.begin(), m_PropertyDrawers.end(), [&](PropertyDrawer* propertyDrawer)
		{
			return propertyDrawer->GetPropertyTypeHash() == typeHash;
		});

		if (it == m_PropertyDrawers.end())
		{
			//ImGui::Text(prop.m_Name.c_str());
			return false;
		}

		PropertyDrawer* drawer = *it;
		return drawer->Draw(label, data, typeHash, flags);
	}

	bool PropertyDrawer::DrawProperty(const GloryReflect::FieldData* pFieldData, void* data, uint32_t flags)
	{
		size_t typeHash = pFieldData->Type();
		size_t elementTypeHash = pFieldData->ArrayElementType();

		auto it = std::find_if(m_PropertyDrawers.begin(), m_PropertyDrawers.end(), [&](PropertyDrawer* propertyDrawer)
		{
			return propertyDrawer->GetPropertyTypeHash() == typeHash;
		});

		if (it != m_PropertyDrawers.end())
		{
			PropertyDrawer* drawer = *it;
			return drawer->Draw(pFieldData->Name(), data, elementTypeHash, flags);
		}

		const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(typeHash);
		if (pTypeData)
		{
			return DrawProperty(pFieldData->Name(), pTypeData, data, flags);
		}

		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), pFieldData->Name());
		return false;
	}

	bool PropertyDrawer::DrawProperty(const std::string& label, const GloryReflect::TypeData* pTypeData, void* data, uint32_t flags)
	{
		bool change = false;

		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

		std::hash<std::string> hasher;
		size_t hash = hasher(label);

		ImGui::PushID(label.c_str());

		if (ImGui::TreeNodeEx("header", node_flags, label.data()))
		{
			for (size_t i = 0; i < pTypeData->FieldCount(); i++)
			{
				const GloryReflect::FieldData* pFieldData = pTypeData->GetFieldData(i);
				size_t offset = pFieldData->Offset();
				void* pAddress = (void*)((char*)(data)+offset);
				change |= PropertyDrawer::DrawProperty(pFieldData, pAddress, 0);
			}
			ImGui::TreePop();
		}
		ImGui::PopID();

		return change;
	}

	bool PropertyDrawer::DrawProperty(const ScriptProperty& scriptProperty, YAML::Node& node, uint32_t flags)
	{
		size_t typeHash = scriptProperty.m_TypeHash;

		auto it = std::find_if(m_PropertyDrawers.begin(), m_PropertyDrawers.end(), [&](PropertyDrawer* propertyDrawer)
		{
			return propertyDrawer->GetPropertyTypeHash() == typeHash;
		});

		if (it != m_PropertyDrawers.end())
		{
			PropertyDrawer* drawer = *it;
			return drawer->Draw(scriptProperty.m_Name, node[scriptProperty.m_Name], typeHash, flags);
		}

		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), scriptProperty.m_Name);
		return false;
	}

	PropertyDrawer* PropertyDrawer::GetPropertyDrawer(size_t typeHash)
	{
		auto it = std::find_if(m_PropertyDrawers.begin(), m_PropertyDrawers.end(), [&](PropertyDrawer* propertyDrawer)
		{
			return propertyDrawer->GetPropertyTypeHash() == typeHash;
		});

		if (it == m_PropertyDrawers.end()) return nullptr;
		return *it;
	}

	size_t PropertyDrawer::GetPropertyTypeHash() const
	{
		return m_TypeHash;
	}

	void PropertyDrawer::Cleanup()
	{
		std::for_each(m_PropertyDrawers.begin(), m_PropertyDrawers.end(), [](PropertyDrawer* pDrawer) { delete pDrawer; });
		m_PropertyDrawers.clear();
	}
}
