#include "AddUIElementWindow.h"
#include "UIMainWindow.h"
#include "UIEditorExtension.h"
#include "AddUIElementAction.h"
#include "SetUIParentAction.h"

#include <UIComponents.h>
#include <UIDocument.h>

#include <EditorAssets.h>
#include <EditorPlatform.h>
#include <Undo.h>
#include <EditableResource.h>
#include <EditorResourceManager.h>

#include <IconsFontAwesome6.h>

namespace Glory::Editor
{
	std::map<uint32_t, std::string_view> ElementIcons = {
		{ ResourceTypes::GetHash<UIText>(), ICON_FA_FONT },
		{ ResourceTypes::GetHash<UIImage>(), ICON_FA_IMAGE },
		{ ResourceTypes::GetHash<UIBox>(), ICON_FA_SQUARE },
		{ ResourceTypes::GetHash<UIPanel>(), ICON_FA_ADDRESS_CARD },
		{ ResourceTypes::GetHash<UIVerticalContainer>(), ICON_FA_RULER_VERTICAL },
		{ ResourceTypes::GetHash<UIScrollView>(), ICON_FA_SCROLL },
	};

	std::vector<std::pair<std::string_view, std::vector<uint32_t>>> ElementCategories = {
		{ "Graphics", { ResourceTypes::GetHash<UIText>(), ResourceTypes::GetHash<UIImage>(), ResourceTypes::GetHash<UIBox>() } },
		{ "Organization", { ResourceTypes::GetHash<UIPanel>(), ResourceTypes::GetHash<UIVerticalContainer>(), ResourceTypes::GetHash<UIScrollView>() }},
	};

	size_t SelectedCategory = 0;

	AddUIElementWindow::AddUIElementWindow() : EditorWindowTemplate("Element Library", 600.0f, 600.0f)
	{
	}

	AddUIElementWindow::~AddUIElementWindow()
	{
	}

	void DrawName(float padding, const Utils::ECS::ComponentType* type)
	{
		const ImVec2 cursorPos = ImGui::GetCursorPos();

		const float availableWidth = ImGui::GetContentRegionAvail().x - padding;
		std::string_view text = type->m_Name;
		const size_t lastNSIndex = text.rfind("::");
		if (lastNSIndex != std::string_view::npos)
			text = text.substr(lastNSIndex + 2);

		const float textWidth = ImGui::CalcTextSize(text.data()).x;
		const int wraps = std::max((int)(textWidth / (availableWidth - 8.0f)), 1);

		ImGui::SetCursorPos({ cursorPos.x + padding, cursorPos.y - wraps * padding / 1.2f });
		ImGui::PushTextWrapPos(availableWidth);
		ImGui::TextWrapped(text.data());
		ImGui::PopTextWrapPos();
	}

	void DrawItem(float iconSize, const Utils::ECS::ComponentType* type, UIMainWindow* pMainWindow)
	{
		const ImVec4 buttonColor = ImGui::GetStyleColorVec4(ImGuiCol_Button);
		const ImVec4 buttonInactiveColor = { buttonColor.x, buttonColor.y, buttonColor.z, 0.0f };

		const float padding = 10.0f;
		const float textHeight = ImGui::CalcTextSize("LABEL").y;
		const ImVec2 itemSize = { iconSize + padding * 2.0F, iconSize + padding + textHeight };

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::BeginChild("##element", itemSize, false, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		ImGui::PopStyleVar(2);

		const ImVec2 cursorPos = ImGui::GetCursorPos();

		ImGui::Button("##elementItem", itemSize);
		const std::string_view icon = ElementIcons.at(type->m_TypeHash).data();
		UIElementType payload{ true, uint64_t(type->m_TypeHash) };
		std::string_view typeName = type->m_Name;
		const size_t lastNSIndex = typeName.rfind("::");
		if (lastNSIndex != std::string_view::npos)
			typeName = typeName.substr(lastNSIndex + 2);

		DND::DragAndDropSource<UIElementType>(&payload, sizeof(UIElementType), [&]() {
			ImGui::Text("%s %s", icon.data(), typeName.data());
		});

		ImGui::SetItemAllowOverlap();

		const UUID documentID = pMainWindow->CurrentDocumentID();
		UIDocument* pDocument = pMainWindow->CurrentDocument();
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0) && documentID && pDocument)
		{
			EditorApplication* pApp = EditorApplication::GetInstance();
			Engine* pEngine = pApp->GetEngine();
			EditorResourceManager& resources = pApp->GetResourceManager();
			EditableResource* pResource = resources.GetEditableResource(documentID);
			YAMLResource<UIDocumentData>* pDocumentData = static_cast<YAMLResource<UIDocumentData>*>(pResource);
			Utils::YAMLFileRef& file = **pDocumentData;

			const size_t siblingIndex = pDocument->Registry().ChildCount(0);
			pMainWindow->SelectedEntity() =
				AddUIElementAction::AddElement(pEngine, pDocument, file, typeName, type->m_TypeHash, 0, siblingIndex);
		}

		ImGui::SetCursorPos({ cursorPos.x + padding, cursorPos.y + padding });
		ImGui::PushFont(EditorPlatform::GetHugeFont());
		ImGui::TextUnformatted(ElementIcons.at(type->m_TypeHash).data());
		ImGui::PopFont();
		DrawName(padding, type);
		ImGui::EndChild();
	}

	void DrawCategories(float height)
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
		ImGui::BeginChild("Categories", ImVec2(0, height), false, window_flags);
		for (size_t i = 0; i < ElementCategories.size(); ++i)
		{
			ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_SpanAvailWidth;
			ImGui::Selectable(ElementCategories[i].first.data(), SelectedCategory == i, selectableFlags);
			if (ImGui::IsItemClicked())
				SelectedCategory = i;
			ImGui::Separator();
		}
		ImGui::EndChild();
	}

	void DrawElements(UIMainWindow* pMainWindow, float height)
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
		ImGui::BeginChild("Elements", ImVec2(0, height), false, window_flags);

		static const float iconSize = 64.0f;

		const ImVec2 windowSize = ImGui::GetWindowSize();

		float width = windowSize.x;

		int columns = (int)(width / (iconSize + 32.0f));
		if (columns <= 0) columns = 1;

		auto& category = ElementCategories[SelectedCategory];
		for (size_t i = 0; i < category.second.size(); ++i)
		{
			auto componentType = Utils::ECS::ComponentTypes::GetComponentType(category.second[i]);
			if (componentType->m_TypeHash == ResourceTypes::GetHash<UITransform>()) continue;

			const int columnIndex = (i % columns) - 1;
			ImGui::PushID(i);
			DrawItem(iconSize, componentType, pMainWindow);
			ImGui::PopID();
			int mod = (i + 1) % columns;
			if (mod != 0) ImGui::SameLine();
		}

		ImGui::EndChild();
	}

	void AddUIElementWindow::OnGUI()
	{
		UIMainWindow* pMainWindow = GetMainWindow();
		const float height = ImGui::GetContentRegionAvail().y - 10.0f;

		if (ImGui::BeginTable("ElementsTable", 2, ImGuiTableFlags_Resizable))
		{
			ImGui::TableNextColumn();
			DrawCategories(height);
			ImGui::TableNextColumn();
			DrawElements(pMainWindow, height);
			ImGui::EndTable();
		}
	}

	UIMainWindow* AddUIElementWindow::GetMainWindow()
	{
		return static_cast<UIMainWindow*>(m_pOwner);
	}
}
