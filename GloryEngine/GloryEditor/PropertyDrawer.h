#pragma once
#include <ResourceType.h>
#include <typeindex>
#include <vector>
#include <ResourceType.h>
#include <any>
#include "SerializedProperty.h"
#include "GloryEditor.h"

#define PROPERTY_DRAWER(x) Glory::Editor::PropertyDrawer::RegisterPropertyDrawer<x>()

namespace Glory::Editor
{
	class PropertyDrawer
	{
	public:
		GLORY_EDITOR_API PropertyDrawer(size_t typeHash);
		virtual GLORY_EDITOR_API ~PropertyDrawer();
		virtual GLORY_EDITOR_API bool Draw(const SerializedProperty* serializedProperty, const std::string& label, void* data, size_t typeHash, uint32_t flags) const;
		virtual GLORY_EDITOR_API bool Draw(const SerializedProperty* serializedProperty, const std::string& label, std::any& data, uint32_t flags) const;
		virtual GLORY_EDITOR_API bool Draw(const std::string& label, std::vector<char>& buffer, size_t typeHash, size_t offset, size_t size, uint32_t flags) const;
		GLORY_EDITOR_API bool Draw(const SerializedProperty* serializedProperty) const;

		template<class T>
		static void RegisterPropertyDrawer()
		{
			RegisterPropertyDrawer(new T());
		}

		static GLORY_EDITOR_API void RegisterPropertyDrawer(PropertyDrawer* pDrawer);

		static GLORY_EDITOR_API bool DrawProperty(const SerializedProperty* serializedProperty, const std::string& label, void* data, size_t typeHash, size_t elementTypeHash, uint32_t flags);
		static GLORY_EDITOR_API bool DrawProperty(const std::string& label, std::any& data, uint32_t flags);
		static GLORY_EDITOR_API bool DrawProperty(const SerializedProperty* serializedProperty);
		static GLORY_EDITOR_API bool DrawProperty(const std::string& label, std::vector<char>& buffer, size_t typeHash, size_t offset, size_t size, uint32_t flags);

	public:
		GLORY_EDITOR_API size_t GetPropertyTypeHash() const;

	private:
		static void Cleanup();

	private:
		friend class MainEditor;
		size_t m_TypeHash;

		static std::vector<PropertyDrawer*> m_PropertyDrawers;
	};

	template<typename PropertyType>
	class PropertyDrawerTemplate : public PropertyDrawer
	{
	public:
		PropertyDrawerTemplate() : PropertyDrawer(ResourceType::GetHash<PropertyType>()) {}

	protected:
		virtual bool Draw(const SerializedProperty* serializedProperty, const std::string& label, void* data, size_t typeHash, uint32_t flags) const override
		{
			return OnGUI(label, (PropertyType*)data, flags);
		}

		virtual bool Draw(const SerializedProperty* serializedProperty, const std::string& label, std::any& data, uint32_t flags) const override
		{
			PropertyType pPropertyData = std::any_cast<PropertyType>(data);
			bool result = OnGUI(label, &pPropertyData, flags);
			data = pPropertyData;
			return result;
		}

		virtual bool Draw(const std::string& label, std::vector<char>& buffer, size_t typeHash, size_t offset, size_t size, uint32_t flags) const override
		{
			PropertyType value;
			memcpy((void*)&value, (void*)&buffer[offset], size);
			PropertyType originalValue = value;
			OnGUI(label, &value, flags);
			if (originalValue == value) return false;
			memcpy((void*)&buffer[offset], (void*)&value, size);
			return true;
		}

		virtual bool OnGUI(const std::string& label, PropertyType* data, uint32_t flags) const = 0;
	};
}
