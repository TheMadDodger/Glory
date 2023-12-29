#include "PropertyDrawer.h"
#include "AssetReferencePropertyDrawer.h"
#include "ValueChangeAction.h"
#include <imgui.h>
#include <algorithm>

namespace Glory::Editor
{
	std::vector<PropertyDrawer*> PropertyDrawer::m_PropertyDrawers = std::vector<PropertyDrawer*>();
	const TypeData* PropertyDrawer::m_pRootTypeData = nullptr;
	std::filesystem::path PropertyDrawer::m_CurrentPropertyPath = "";

	PropertyDrawer::PropertyDrawer(uint32_t typeHash) : m_TypeHash(typeHash)
	{
	}

	PropertyDrawer::~PropertyDrawer()
	{
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

	void PropertyDrawer::RegisterPropertyDrawer(PropertyDrawer* pDrawer)
	{
		m_PropertyDrawers.push_back(pDrawer);
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

	struct PathGuard
	{
		PathGuard(const std::string name)
		{
			PropertyDrawer::PushPath(name);
		}
		~PathGuard()
		{
			PropertyDrawer::PopPath();
		}
	};

	bool PropertyDrawer::DrawProperty(const FieldData* pFieldData, void* data, uint32_t flags)
	{
		PathGuard p(pFieldData->Name());

		if (pFieldData->Type() == ST_Array)
		{
			return GetPropertyDrawer(ST_Array)->Draw(pFieldData->Name(), data, pFieldData->ArrayElementType(), flags);
		}

		const TypeData* pTypeData = Reflect::GetTyeData(pFieldData->ArrayElementType());
		if (pTypeData)
		{
			const bool change = DrawProperty(pFieldData->Name(), pTypeData, data, flags);
			return change;
		}

		PropertyDrawer* pDrawer = GetPropertyDrawer(pFieldData->Type());
		if (pDrawer)
		{
			return pDrawer->Draw(pFieldData->Name(), data, pFieldData->Type(), flags);
		}

		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), pFieldData->Name());
		return false;
	}

	bool PropertyDrawer::DrawProperty(const std::string& label, const TypeData* pTypeData, void* data, uint32_t flags)
	{
		const bool setRootType = !m_pRootTypeData;
		if (setRootType)
		{
			m_pRootTypeData = pTypeData;
		}

		bool change = false;
		PropertyDrawer* pDrawer = GetPropertyDrawer(pTypeData->TypeHash());
		PropertyDrawer* pInternalDrawer = GetPropertyDrawer(pTypeData->InternalTypeHash());
		if (pDrawer)
		{
			change = pDrawer->Draw(label, data, pTypeData->TypeHash(), flags);
		}
		else if (pInternalDrawer)
		{
			change = pInternalDrawer->Draw(label, data, pTypeData->TypeHash(), flags);
		}
		else
		{
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), label.c_str());
		}

		if (setRootType)
		{
			m_pRootTypeData = nullptr;
			m_CurrentPropertyPath = "";
		}
		return change;
	}

	/*bool PropertyDrawer::DrawProperty(const ScriptProperty& scriptProperty, YAML::Node& node, uint32_t flags)
	{
		PathGuard p{ scriptProperty.m_Name };
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
	}*/

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

		const TypeData* pTypeData = Reflect::GetTyeData(typeHash);
		if (pTypeData)
		{
			/* TODO */
			throw new std::exception("Not yet implemented!");
		}

		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", label.c_str());
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

	void PropertyDrawer::SetCurrentPropertyPath(std::string_view path)
	{
		m_CurrentPropertyPath = path;
	}

	void PropertyDrawer::SetCurrentPropertyPath(std::filesystem::path& path)
	{
		m_CurrentPropertyPath = path;
	}

	const std::filesystem::path& PropertyDrawer::GetCurrentPropertyPath()
	{
		return m_CurrentPropertyPath;
	}

	const TypeData* PropertyDrawer::GetRootTypeData()
	{
		return m_pRootTypeData;
	}

	void PropertyDrawer::PushPath(const std::string& name)
	{
		m_CurrentPropertyPath.append(name);
	}

	void PropertyDrawer::PopPath()
	{
		m_CurrentPropertyPath = m_CurrentPropertyPath.parent_path();
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
