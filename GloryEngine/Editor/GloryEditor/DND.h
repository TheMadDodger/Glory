#pragma once
#include <Enum.h>
#include <functional>
#include <filesystem>
#include <UUID.h>
#include <imgui.h>
#include <EntityID.h>

namespace Glory
{
	class Resource;
}

namespace Glory::Editor
{
	struct ObjectPayload
	{
		Utils::ECS::EntityID EntityID;
		UUID SceneID;
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
			DragAndDropSource(pType->Name(), payload, payloadSize, previewCallback, flags);
		}

		static void DragAndDropSource(const std::string_view name, void* payload, size_t payloadSize, std::function<void()> previewCallback, ImGuiDragDropFlags flags = 0);

		bool IsEnabled() { return !m_AcceptedTypes.empty(); }
		bool HandleDragAndDropTarget(std::function<void(uint32_t, const ImGuiPayload*)> callback);
		bool HandleDragAndDropWindowTarget(std::function<void(uint32_t, const ImGuiPayload*)> callback);

		std::vector<uint32_t> m_AcceptedTypes;
		ImGuiDragDropFlags m_DNDFlags = 0;

	private:
		bool HandleTargetInternal(std::function<void(uint32_t, const ImGuiPayload*)> callback);
	};
}
