#pragma once
#include "EntityComponentPopup.h"

#include <Editor.h>
#include <SceneObject.h>
#include <EntityComponentObject.h>
#include <filesystem>

namespace Glory::Editor
{
	enum ObjectMenuType : unsigned int;

    class EntitySceneObjectEditor : public EditorTemplate<EntitySceneObjectEditor, SceneObject>
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
		GLORY_EDITOR_API static void DrawObjectNodeName(SceneObject* pObject, bool isPrefab);
		GLORY_EDITOR_API static bool SearchCompare(std::string_view search, SceneObject* pObject);

		GLORY_EDITOR_API static void ConvertToPrefabMenuItem(Object* pObject, const ObjectMenuType&);
		GLORY_EDITOR_API static void ConvertToPrefab(SceneObject* pObject, std::filesystem::path path);
		GLORY_EDITOR_API static void UnpackPrefabMenuItem(Object* pObject, const ObjectMenuType&);

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

		SceneObject* m_pObject;
		std::vector<EntityComponentObject*> m_pComponents;

		EntityComponentPopup m_ComponentPopup;
    };
}
