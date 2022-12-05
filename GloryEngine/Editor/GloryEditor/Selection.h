#pragma once
#include <Object.h>
#include <vector>
#include "GloryEditor.h"

namespace Glory::Editor
{
	class Selection
	{
	public:
		static GLORY_EDITOR_API void SetActiveObject(Object* pObject);
		static GLORY_EDITOR_API void AddObjectToSelection(Object* pObject);
		static GLORY_EDITOR_API void RemoveObjectFromSelection(Object* pObject);
		static GLORY_EDITOR_API void AddObjectToSelection(UUID objectID);
		static GLORY_EDITOR_API void RemoveObjectFromSelection(UUID objectID);
		static GLORY_EDITOR_API Object* GetActiveObject();
		static GLORY_EDITOR_API void Clear();
		static GLORY_EDITOR_API bool IsObjectSelected(Object* pObject);

	private:
		static std::vector<Object*> m_pSelectedObjects;

	private:
		Selection();
		virtual ~Selection();
	};
}