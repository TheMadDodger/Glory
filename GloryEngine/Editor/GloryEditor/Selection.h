#pragma once
#include <Object.h>
#include <vector>
#include <functional>
#include <map>
#include "GloryEditor.h"

namespace Glory::Editor
{
	class Selection
	{
	public:
		static GLORY_EDITOR_API void SetActiveObject(Object* pObject);
		static GLORY_EDITOR_API void SetActiveObjectNoUndo(Object* pObject);
		static GLORY_EDITOR_API void AddObjectToSelection(Object* pObject);
		static GLORY_EDITOR_API void RemoveObjectFromSelection(Object* pObject);
		static GLORY_EDITOR_API void AddObjectToSelection(UUID objectID);
		static GLORY_EDITOR_API void RemoveObjectFromSelection(UUID objectID);
		static GLORY_EDITOR_API Object* GetActiveObject();
		static GLORY_EDITOR_API void Clear();
		static GLORY_EDITOR_API bool IsObjectSelected(Object* pObject);
		static GLORY_EDITOR_API UUID SubscribeToSelectionChange(std::function<void()> callback);
		static GLORY_EDITOR_API void UnsubscribeToSelectionChange(UUID uuid);

	private:
		static void TriggerSelectionChangeCallback();

	private:
		static std::vector<Object*> m_pSelectedObjects;
		static std::map<UUID, std::function<void()>> m_SelectionChangeCallback;

	private:
		Selection();
		virtual ~Selection();
	};
}