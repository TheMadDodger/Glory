#pragma once
#include "EditorApplication.h"

#include <Engine.h>
#include <ResourceType.h>
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
		static void DragAndDropSource(void* payload, size_t payloadSize, std::function<void()> previewCallback, ImGuiDragDropFlags flags=0)
		{
			std::string_view name;
			ResourceType* pResourceType = EditorApplication::GetInstance()->GetEngine()->GetResourceTypes().GetResourceType<T>();
			if (!pResourceType)
			{
				const TypeData* pType = Reflect::GetTyeData(ResourceTypes::GetHash<T>());
				name = pType->TypeName();
			}
			else
			{
				name = pResourceType->Name();
			}
			DragAndDropSource(name, payload, payloadSize, previewCallback, flags);
		}

		GLORY_EDITOR_API static void DragAndDropSource(const std::string_view name, void* payload, size_t payloadSize,
			std::function<void()> previewCallback, ImGuiDragDropFlags flags=0);

		GLORY_EDITOR_API bool IsEnabled() { return !m_AcceptedTypes.empty(); }
		GLORY_EDITOR_API bool HandleDragAndDropTarget(std::function<void(uint32_t, const ImGuiPayload*)> callback,
			std::function<void()> drawCallback=NULL);
		GLORY_EDITOR_API bool HandleDragAndDropWindowTarget(std::function<void(uint32_t, const ImGuiPayload*)> callback,
			std::function<void()> drawCallback=NULL);

		std::vector<uint32_t> m_AcceptedTypes;
		ImGuiDragDropFlags m_DNDFlags = 0;

	private:
		bool HandleTargetInternal(std::function<void(uint32_t, const ImGuiPayload*)> callback);
	};
}
