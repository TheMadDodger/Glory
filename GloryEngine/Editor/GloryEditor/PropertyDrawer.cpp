#include "PropertyDrawer.h"
#include "AssetReferencePropertyDrawer.h"
#include "ValueChangeAction.h"
#include <imgui.h>
#include <algorithm>

namespace Glory::Editor
{
	std::vector<PropertyDrawer*> PropertyDrawer::m_PropertyDrawers = std::vector<PropertyDrawer*>();
	const GloryReflect::TypeData* PropertyDrawer::m_pRootTypeData = nullptr;
	std::vector<const GloryReflect::FieldData*> PropertyDrawer::m_pCurrentFieldDataStack;
	std::filesystem::path PropertyDrawer::m_CurrentPropertyPath = "";

	PropertyDrawer::PropertyDrawer(uint32_t typeHash) : m_TypeHash(typeHash)
	{
	}

	PropertyDrawer::~PropertyDrawer()
	{
	}

	bool PropertyDrawer::Draw(const SerializedProperty* serializedProperty, const std::string& label, void* data, uint32_t typeHash, uint32_t flags) const
	{
		ImGui::Text(label.data());
		return false;
	}

	bool PropertyDrawer::Draw(const SerializedProperty* serializedProperty, const std::string& label, std::any& data, uint32_t flags) const
	{
		ImGui::Text(label.data());
		return false;
	}

	bool PropertyDrawer::Draw(const std::string& label, std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, uint32_t flags) const
	{
		ImGui::Text(label.data());
		return false;
	}

	bool PropertyDrawer::Draw(const std::string& label, void* data, uint32_t typeHash, uint32_t flags) const
	{
		ImGui::Text(label.data());
		return false;
	}

	bool PropertyDrawer::Draw(const std::string& label, YAML::Node& node, uint32_t typeHash, uint32_t flags) const
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

	bool PropertyDrawer::DrawProperty(const SerializedProperty* serializedProperty, const std::string& label, void* data, uint32_t typeHash, uint32_t elementTypeHash, uint32_t flags)
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
		uint32_t typeHash = ResourceType::GetHash(data.type());

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

	bool PropertyDrawer::DrawProperty(const std::string& label, std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, uint32_t flags)
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

	bool PropertyDrawer::DrawProperty(const std::string& label, void* data, uint32_t typeHash, uint32_t flags)
	{
		auto it = std::find_if(m_PropertyDrawers.begin(), m_PropertyDrawers.end(), [&](PropertyDrawer* propertyDrawer)
		{
			return propertyDrawer->GetPropertyTypeHash() == typeHash;
		});

		if (it == m_PropertyDrawers.end())
		{
			ImGui::Text(label.c_str());
			return false;
		}

		PropertyDrawer* drawer = *it;
		return drawer->Draw(label, data, typeHash, flags);
	}

	bool PropertyDrawer::DrawProperty(const GloryReflect::FieldData* pFieldData, void* data, uint32_t flags)
	{
		m_pCurrentFieldDataStack.push_back(pFieldData);
		uint32_t typeHash = pFieldData->Type();
		uint32_t elementTypeHash = pFieldData->ArrayElementType();

		std::string_view name = pFieldData->Name();
		m_CurrentPropertyPath.append(name);

		auto it = std::find_if(m_PropertyDrawers.begin(), m_PropertyDrawers.end(), [&](PropertyDrawer* propertyDrawer)
		{
			return propertyDrawer->GetPropertyTypeHash() == typeHash;
		});

		if (it != m_PropertyDrawers.end())
		{
			PropertyDrawer* drawer = *it;
			const bool change = drawer->Draw(pFieldData->Name(), data, elementTypeHash, flags);
			m_pCurrentFieldDataStack.pop_back();
			return change;
		}

		const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(typeHash);
		if (pTypeData)
		{
			const bool change = DrawProperty(pFieldData->Name(), pTypeData, data, flags);
			m_pCurrentFieldDataStack.pop_back();
			return change;
		}

		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), pFieldData->Name());
		m_pCurrentFieldDataStack.pop_back();
		return false;
	}

	bool PropertyDrawer::DrawProperty(const std::string& label, const GloryReflect::TypeData* pTypeData, void* data, uint32_t flags)
	{
		const bool setRootType = !m_pRootTypeData;
		if (setRootType)
		{
			m_pRootTypeData = pTypeData;
			m_CurrentPropertyPath = "";
		}

		bool change = false;

		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

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

		if (setRootType)
			m_pRootTypeData = nullptr;
		return change;
	}

	bool PropertyDrawer::DrawProperty(const ScriptProperty& scriptProperty, YAML::Node& node, uint32_t flags)
	{
		m_CurrentPropertyPath = scriptProperty.m_Name;
		uint32_t typeHash = scriptProperty.m_TypeHash;
		uint32_t elementTypeHash = scriptProperty.m_ElementTypeHash;

		auto it = std::find_if(m_PropertyDrawers.begin(), m_PropertyDrawers.end(), [&](PropertyDrawer* propertyDrawer)
		{
			return propertyDrawer->GetPropertyTypeHash() == typeHash;
		});

		if (it != m_PropertyDrawers.end())
		{
			PropertyDrawer* drawer = *it;
			return drawer->Draw(scriptProperty.m_Name, node[scriptProperty.m_Name], elementTypeHash, flags);
		}

		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), scriptProperty.m_Name);
		return false;
	}

	bool PropertyDrawer::DrawProperty(const std::string& label, YAML::Node& node, uint32_t typeHash, uint32_t elementTypeHash, uint32_t flags)
	{
		auto it = std::find_if(m_PropertyDrawers.begin(), m_PropertyDrawers.end(), [&](PropertyDrawer* propertyDrawer)
		{
			return propertyDrawer->GetPropertyTypeHash() == typeHash;
		});

		if (it != m_PropertyDrawers.end())
		{
			PropertyDrawer* drawer = *it;
			return drawer->Draw(label, node[label], elementTypeHash, flags);
		}

		const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(typeHash);
		if (pTypeData)
		{
			/* TODO */
			throw new std::exception("Not yet implemented!");
		}

		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), label.c_str());
		return false;
	}

	PropertyDrawer* PropertyDrawer::GetPropertyDrawer(uint32_t typeHash)
	{
		auto it = std::find_if(m_PropertyDrawers.begin(), m_PropertyDrawers.end(), [&](PropertyDrawer* propertyDrawer)
		{
			return propertyDrawer->GetPropertyTypeHash() == typeHash;
		});

		if (it == m_PropertyDrawers.end()) return nullptr;
		return *it;
	}

	const std::filesystem::path& PropertyDrawer::GetCurrentPropertyPath()
	{
		return m_CurrentPropertyPath;
	}

	const GloryReflect::TypeData* PropertyDrawer::GetRootTypeData()
	{
		return m_pRootTypeData;
	}

	const std::vector<const GloryReflect::FieldData*>& PropertyDrawer::GetCurrentFieldStack()
	{
		return m_pCurrentFieldDataStack;
	}

	void PropertyDrawer::PushFieldType(const GloryReflect::FieldData* pField)
	{
		m_pCurrentFieldDataStack.push_back(pField);
	}

	void PropertyDrawer::PopFieldType()
	{
		m_pCurrentFieldDataStack.pop_back();
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
