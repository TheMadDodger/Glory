#pragma once
#include <Object.h>
#include <vector>

namespace Glory::Editor
{
	class Selection
	{
	public:
		static void SetActiveObject(Object* pObject);
		static Object* GetActiveObject();
		static void Clear();
		static bool IsObjectSelected(Object* pObject);

	private:
		static std::vector<Object*> m_pSelectedObjects;

	private:
		Selection();
		virtual ~Selection();
	};
}