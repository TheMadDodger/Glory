#pragma once
#include <Enum.h>
#include <functional>
#include <filesystem>
#include <UUID.h>
#include <imgui.h>

namespace Glory
{
	class SceneObject;
	class Resource;
}

namespace Glory::Editor
{
	struct ObjectPayload
	{
		SceneObject* pObject;
	};

	struct AssetPayload
	{
		UUID ResourceID;
	};

	struct DND
	{
		template<typename T>
		static void DragAndDropSource(void* payload, size_t payloadSize, std::function<void()> previewCallback, ImGuiDragDropFlags flags = 0)
		{
			ResourceType* pType = ResourceType::GetResourceType<T>();
			//if(pType->SubTypeCount()) pType = pType->GetSubType(0);

			DragAndDropSource(pType->Name(), payload, payloadSize, previewCallback, flags);
		}

		static void DragAndDropSource(const std::string_view name, void* payload, size_t payloadSize, std::function<void()> previewCallback, ImGuiDragDropFlags flags = 0);

		bool IsEnabled() { return !m_AcceptedTypes.empty(); }
		bool HandleDragAndDropTarget(std::function<void(uint32_t, const ImGuiPayload*)> callback);

		std::vector<uint32_t> m_AcceptedTypes;
		ImGuiDragDropFlags m_DNDFlags = 0;
	};
}
