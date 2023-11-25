#pragma once
#include "EntityComponentPopup.h"
#include "EditableEntity.h"

#include <Editor.h>
#include <EntityComponentObject.h>
#include <filesystem>

namespace Glory
{
	class Entity;
namespace Editor
{
	enum ObjectMenuType : unsigned int;

	class EntitySceneObjectEditor : public EditorTemplate<EntitySceneObjectEditor, EditableEntity>
	{
	public:
		GLORY_EDITOR_API EntitySceneObjectEditor();
		GLORY_EDITOR_API virtual ~EntitySceneObjectEditor();
		GLORY_EDITOR_API virtual bool OnGUI() override;

		GLORY_EDITOR_API void Refresh();

		template<typename T>
		static std::string_view GetComponentIcon()
		{
			return GetComponentIcon(ResourceType::GetHash<T>());
		}

		GLORY_EDITOR_API static std::string_view GetComponentIcon(uint32_t typeHash);
		GLORY_EDITOR_API static void DrawObjectNodeName(Entity& entity, bool isPrefab);
		GLORY_EDITOR_API static bool SearchCompare(std::string_view search, Entity& entity);

		GLORY_EDITOR_API static void ConvertToPrefabMenuItem(Object* pObject, const ObjectMenuType&);
		GLORY_EDITOR_API static void ConvertToPrefab(Entity& entity, std::filesystem::path path);
		GLORY_EDITOR_API static void UnpackPrefabMenuItem(Object* pObject, const ObjectMenuType&);

		template<typename T>
		static void AddComponentIcon(std::string_view icon)
		{
			AddComponentIcon(ResourceType::GetHash<T>(), icon);
		}
		GLORY_EDITOR_API static void AddComponentIcon(uint32_t hash, std::string_view icon);

	private:
		void Initialize();
		bool NameGUI();
		bool ComponentGUI();

	private:
		std::vector<Editor*> m_pComponentEditors;
		bool m_Initialized;
		static const int MAXNAMESIZE = 250;
		char m_NameBuff[MAXNAMESIZE];
		bool m_AddingComponent;

		EditableEntity* m_pObject;
		std::vector<EntityComponentObject*> m_pComponents;

		EntityComponentPopup m_ComponentPopup;
	};
}
}
