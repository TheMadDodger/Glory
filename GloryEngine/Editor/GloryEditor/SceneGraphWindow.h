#pragma once
#include <ScenesModule.h>
#include "EditorWindow.h"

namespace Glory::Editor
{
	class SceneGraphWindow : public EditorWindowTemplate<SceneGraphWindow>
	{
	public:
		SceneGraphWindow();
		virtual ~SceneGraphWindow();

		GLORY_EDITOR_API static void SetDrawObjectNameCallback(std::function<void(SceneObject*, bool)> callback);
		GLORY_EDITOR_API static void SetSearchCompareCallback(std::function<bool(std::string_view, SceneObject*)> callback);
		GLORY_EDITOR_API static void SetSearchTooltipCallback(std::function<void()> callback);

	private:
		virtual void OnGUI() override;

		void SceneDropdown(size_t index, GScene* pScene, bool isActive);
		bool ChildrenList(size_t index, SceneObject* pObject);

		bool GetExcludedObjectsFromFilterRecursive(GScene* pScene);
		bool GetExcludedObjectsFromFilterRecursive(SceneObject* pObject);

		bool IsExcluded(const UUID uuid);

	private:
		static const size_t SearchBufferSize = 1000;
		char SearchBuffer[SearchBufferSize] = "\0";
		bool m_NeedsFilter = false;

		std::vector<UUID> m_SearchResultExcludeCache;
	};
}