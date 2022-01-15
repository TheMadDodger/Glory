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

	bool PropertyDrawer::Draw(const std::string& label, void* data, uint32_t flags) const
	{
		ImGui::Text(label.data());
		return false;
	}

	bool PropertyDrawer::Draw(const std::string& label, std::any& data, uint32_t flags) const
	{
		ImGui::Text(label.data());
		return false;
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
		return drawer->Draw(label, data, flags);
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
		return drawer->Draw(label, data, flags);
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
