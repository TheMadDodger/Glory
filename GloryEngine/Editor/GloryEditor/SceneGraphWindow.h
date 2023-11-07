#pragma once
#include "EditorWindow.h"

namespace Glory
{
	class GScene;

namespace Editor
{
	class SceneGraphWindow : public EditorWindowTemplate<SceneGraphWindow>
	{
	public:
		SceneGraphWindow();
		virtual ~SceneGraphWindow();

	private:
		virtual void OnGUI() override;

		void SceneDropdown(size_t index, GScene* pScene, bool isActive);
		bool ChildrenList(size_t index, SceneObject* pObject);

		bool GetExcludedObjectsFromFilterRecursive(GScene* pScene);
		bool GetExcludedObjectsFromFilterRecursive(SceneObject* pObject);

		bool IsExcluded(const UUID uuid);

		bool HandleAssetDragAndDrop(SceneObject* pParent, uint32_t dndHash, const ImGuiPayload* pPayload);

	private:
		static const size_t SearchBufferSize = 1000;
		char SearchBuffer[SearchBufferSize] = "\0";
		bool m_NeedsFilter = false;

		std::vector<UUID> m_SearchResultExcludeCache;
	};
}
}