#pragma once
#include <ResourceType.h>
#include <typeindex>
#include <vector>
#include <ResourceType.h>
#include <any>
#include <Reflection.h>
#include <ScriptProperty.h>
#include <yaml-cpp/yaml.h>
#include <YAML_GLM.h>
#include <GLORY_YAML.h>
#include "SerializedProperty.h"
#include "GloryEditor.h"
#include "PropertyAction.h"

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
		virtual GLORY_EDITOR_API bool Draw(const std::string& label, void* data, size_t typeHash, uint32_t flags) const;
		virtual GLORY_EDITOR_API bool Draw(const std::string& label, YAML::Node& node, size_t typeHash, uint32_t flags) const;
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
		static GLORY_EDITOR_API bool DrawProperty(const std::string& label, void* data, size_t typeHash, uint32_t flags);
		static GLORY_EDITOR_API bool DrawProperty(const GloryReflect::FieldData* pFieldData, void* data, uint32_t flags);
		static GLORY_EDITOR_API bool DrawProperty(const std::string& label, const GloryReflect::TypeData* pTypeData, void* data, uint32_t flags);
		static GLORY_EDITOR_API bool DrawProperty(const ScriptProperty& scriptProperty, YAML::Node& node, uint32_t flags);

		static GLORY_EDITOR_API PropertyDrawer* GetPropertyDrawer(size_t typeHash);

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
			PropertyType oldValue = *(PropertyType*)data;
			if (OnGUI(label, (PropertyType*)data, flags))
			{
				PropertyType newValue = *(PropertyType*)data;
				Undo::AddAction(new PropertyAction<PropertyType>(label, oldValue, newValue));
				return true;
			}
			return false;
		}

		virtual bool Draw(const std::string& label, void* data, size_t typeHash, uint32_t flags) const override
		{
			PropertyType oldValue = *(PropertyType*)data;
			if (OnGUI(label, (PropertyType*)data, flags))
			{
				PropertyType newValue = *(PropertyType*)data;
				Undo::AddAction(new PropertyAction<PropertyType>(label, oldValue, newValue));
				return true;
			}
			return false;
		}

		virtual bool Draw(const SerializedProperty* serializedProperty, const std::string& label, std::any& data, uint32_t flags) const override
		{
			PropertyType pPropertyData = std::any_cast<PropertyType>(data);
			PropertyType oldValue = pPropertyData;
			bool result = OnGUI(label, &pPropertyData, flags);
			if (result)
			{
				Undo::AddAction(new PropertyAction<PropertyType>(label, oldValue, pPropertyData));
				data = pPropertyData;
			}
			return result;
		}

		virtual bool Draw(const std::string& label, std::vector<char>& buffer, size_t typeHash, size_t offset, size_t size, uint32_t flags) const override
		{
			PropertyType value;
			memcpy((void*)&value, (void*)&buffer[offset], size);
			PropertyType originalValue = value;
			if (OnGUI(label, &value, flags))
			{
				Undo::AddAction(new PropertyAction<PropertyType>(label, originalValue, value));
			}
			if (originalValue == value) return false;
			memcpy((void*)&buffer[offset], (void*)&value, size);
			return true;
		}

		virtual bool Draw(const std::string& label, YAML::Node& node, size_t typeHash, uint32_t flags) const override
		{
			PropertyType oldValue = PropertyType();
			if (node.IsDefined()) oldValue = node.as<PropertyType>();
			else node = oldValue;
			PropertyType newValue = oldValue;
			if (OnGUI(label, &newValue, flags))
			{
				node = newValue;
				Undo::AddAction(new PropertyAction<PropertyType>(label, oldValue, newValue));
				return true;
			}
			return false;
		}

		virtual bool OnGUI(const std::string& label, PropertyType* data, uint32_t flags) const = 0;
	};
}
