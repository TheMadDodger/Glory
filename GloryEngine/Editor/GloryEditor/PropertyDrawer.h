#pragma once
#include "GloryEditor.h"
#include "ValueChangeAction.h"
#include "NodeValueChangeAction.h"

#include <ResourceType.h>
#include <typeindex>
#include <vector>
#include <ResourceType.h>
#include <any>
#include <Reflection.h>
#include <yaml-cpp/yaml.h>
#include <YAML_GLM.h>
#include <GLORY_YAML.h>

#define PROPERTY_DRAWER(x) Glory::Editor::PropertyDrawer::RegisterPropertyDrawer<x>()

namespace Glory::Utils
{
	struct YAMLFileRef;
}

namespace Glory::Editor
{
	class PropertyDrawer
	{
	public:
		GLORY_EDITOR_API PropertyDrawer(uint32_t typeHash);
		virtual GLORY_EDITOR_API ~PropertyDrawer();
		virtual GLORY_EDITOR_API bool Draw(const std::string& label, std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, uint32_t flags) const;
		virtual GLORY_EDITOR_API bool Draw(const std::string& label, void* data, uint32_t typeHash, uint32_t flags) const;
		virtual GLORY_EDITOR_API bool Draw(const std::string& label, YAML::Node& node, uint32_t typeHash, uint32_t flags) const;
		virtual GLORY_EDITOR_API bool Draw(Utils::YAMLFileRef& file, const std::filesystem::path& path, uint32_t typeHash, uint32_t flags) const;

		template<class T>
		static void RegisterPropertyDrawer()
		{
			RegisterPropertyDrawer(new T());
		}

		static GLORY_EDITOR_API void RegisterPropertyDrawer(PropertyDrawer* pDrawer);

		static GLORY_EDITOR_API bool DrawProperty(const FieldData* pFieldData, void* data, uint32_t flags);
		static GLORY_EDITOR_API bool DrawProperty(const std::string& label, const TypeData* pTypeData, void* data, uint32_t flags);
		static GLORY_EDITOR_API void SetDisabledCheckCallback(std::function<bool(const FieldData*)> disabledCheck = NULL);

		static GLORY_EDITOR_API bool DrawProperty(const std::string& label, std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, uint32_t flags);
		static GLORY_EDITOR_API bool DrawProperty(const std::string& label, void* data, uint32_t typeHash, uint32_t flags);
		static GLORY_EDITOR_API bool DrawProperty(const std::string& label, YAML::Node& node, uint32_t typeHash, uint32_t elementTypeHash, uint32_t flags);
		static GLORY_EDITOR_API bool DrawProperty(Utils::YAMLFileRef& file, const std::filesystem::path& path, uint32_t typeHash, uint32_t elementTypeHash, uint32_t flags);

		static GLORY_EDITOR_API PropertyDrawer* GetPropertyDrawer(uint32_t typeHash);

		static GLORY_EDITOR_API void SetCurrentPropertyPath(std::string_view path);
		static GLORY_EDITOR_API void SetCurrentPropertyPath(std::filesystem::path& path);
		static GLORY_EDITOR_API const std::filesystem::path& GetCurrentPropertyPath();
		static GLORY_EDITOR_API const TypeData* GetRootTypeData();

		static GLORY_EDITOR_API void PushPath(const std::string& name);
		static GLORY_EDITOR_API void PopPath();

	public:
		GLORY_EDITOR_API size_t GetPropertyTypeHash() const;

	private:
		static void Cleanup();

	private:
		friend class MainEditor;
		uint32_t m_TypeHash;

		static std::vector<PropertyDrawer*> m_PropertyDrawers;
		static const TypeData* m_pRootTypeData;
		static std::filesystem::path m_CurrentPropertyPath;
	};

	template<typename PropertyType>
	class SimplePropertyDrawerTemplate : public PropertyDrawer
	{
	public:
		SimplePropertyDrawerTemplate() : PropertyDrawer(ResourceTypes::GetHash<PropertyType>()) {}

	protected:
		virtual bool Draw(const std::string& label, void* data, uint32_t typeHash, uint32_t flags) const override
		{
			PropertyType oldValue = *(PropertyType*)data;
			if (OnGUI(label, (PropertyType*)data, flags))
			{
				PropertyType newValue = *(PropertyType*)data;
				ValueChangeAction* pAction = new ValueChangeAction(PropertyDrawer::GetRootTypeData(), PropertyDrawer::GetCurrentPropertyPath());
				pAction->SetOldValue(&oldValue);
				pAction->SetNewValue(&newValue);
				Undo::AddAction(pAction);
				return true;
			}
			return false;
		}

		virtual bool Draw(const std::string& label, std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, uint32_t flags) const override
		{
			PropertyType value;
			memcpy((void*)&value, (void*)&buffer[offset], size);
			PropertyType originalValue = value;
			if (OnGUI(label, &value, flags))
			{
				//Undo::AddAction(new PropertyAction<PropertyType>(label, originalValue, value));
				// FIXME SER
			}
			if (originalValue == value) return false;
			memcpy((void*)&buffer[offset], (void*)&value, size);
			return true;
		}

		bool OnGUI(const std::string& label, PropertyType* data, uint32_t flags) const;
	};

	template<typename PropertyType>
	class PropertyDrawerTemplate : public PropertyDrawer
	{
	public:
		PropertyDrawerTemplate() : PropertyDrawer(ResourceTypes::GetHash<PropertyType>()) {}

		GLORY_EDITOR_API bool OnGUI(const std::string& label, PropertyType* data, uint32_t flags) const;

	protected:
		virtual bool Draw(const std::string& label, void* data, uint32_t typeHash, uint32_t flags) const override
		{
			PropertyType oldValue = *(PropertyType*)data;
			if (OnGUI(label, (PropertyType*)data, flags))
			{
				PropertyType newValue = *(PropertyType*)data;
				ValueChangeAction* pAction = new ValueChangeAction(PropertyDrawer::GetRootTypeData(), PropertyDrawer::GetCurrentPropertyPath());
				pAction->SetOldValue(&oldValue);
				pAction->SetNewValue(&newValue);
				Undo::AddAction(pAction);

				return true;
			}
			return false;
		}

		virtual bool Draw(const std::string& label, std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, uint32_t flags) const override
		{
			PropertyType value;
			memcpy((void*)&value, (void*)&buffer[offset], size);
			PropertyType originalValue = value;
			if (OnGUI(label, &value, flags))
			{
				//Undo::AddAction(new PropertyAction<PropertyType>(label, originalValue, value));

				// FIXME SER
			}
			if (originalValue == value) return false;
			memcpy((void*)&buffer[offset], (void*)&value, size);
			return true;
		}

		virtual bool Draw(const std::string& label, YAML::Node& node, uint32_t, uint32_t flags) const override
		{
			PropertyType oldValue = PropertyType();
			if (node.IsDefined()) oldValue = node.as<PropertyType>();
			else node = oldValue;
			PropertyType newValue = oldValue;
			if (OnGUI(label, &newValue, flags))
			{
				node = newValue;
				Undo::AddAction<NodeValueChangeAction>(PropertyDrawer::GetCurrentPropertyPath(), YAML::Node(oldValue), YAML::Node(newValue));
				return true;
			}
			return false;
		}

		virtual bool Draw(Utils::YAMLFileRef& file, const std::filesystem::path& path, uint32_t typeHash, uint32_t flags) const override
		{
			auto prop = file[path];
			if (!prop.Exists())
				prop.Set(PropertyType());

			PropertyType value = prop.As<PropertyType>();
			PropertyType originalValue = value;
			std::string label = path.filename().string().data();
			if (label == "Value")
				label = path.parent_path().filename().string();

			if (OnGUI(label, &value, flags))
			{
				Undo::ApplyYAMLEdit(file, path, originalValue, value);
				return true;
			}
			return false;
		}
	};
}
