#include "UIEditor.h"
#include "UIDocumentImporter.h"
#include "UIMainWindow.h"

#include <UIDocument.h>
#include <UIDocumentData.h>
#include <UIRendererModule.h>
#include <Debug.h>
#include <GraphicsModule.h>
#include <UIComponents.h>
#include <YAML_GLM.h>

#include <EditorApplication.h>
#include <EditableResource.h>
#include <EditorResourceManager.h>
#include <UIDocument.h>
#include <MeshData.h>
#include <NodeRef.h>
#include <ImGuiHelpers.h>
#include <Undo.h>
#include <EditorUI.h>

#include <fstream>
#include <sstream>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/glm.hpp>

namespace Glory::Editor
{
	/* 1080p selected by default */
	size_t SelectedAspect = 1;
	size_t SelectedResolution = 3;
	glm::vec2 CustomResolution{ 1920.0f, 1080.0f };

	UIEditor::UIEditor() : EditorWindowTemplate("UI Editor", 600.0f, 600.0f)
	{
		m_WindowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar;
	}

	UIEditor::~UIEditor()
	{
	}

	void CalculateTransform(Utils::ECS::EntityID entity, Utils::ECS::EntityRegistry& registry, glm::mat4& parentTransform, glm::mat4& result, const glm::vec2& sizeFactor)
	{
		const Utils::ECS::EntityID parent = registry.GetParent(entity);
		if (parent && registry.IsValid(parent))
		{
			CalculateTransform(parent, registry, parentTransform, result, sizeFactor);
		}

		UITransform& transform = registry.GetComponent<UITransform>(entity);
		const glm::vec2 size{ transform.m_Width, transform.m_Height };
		const glm::mat4 rotation = glm::rotate(glm::identity<glm::mat4>(), glm::radians(transform.m_Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
		const glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), glm::vec3(float(transform.m_X) * sizeFactor.x, float(transform.m_Y) * sizeFactor.y, 0.0f));
		const glm::mat4 parentTranslation = glm::translate(glm::identity<glm::mat4>(), glm::vec3(float(transform.m_X), float(transform.m_Y), 0.0f));
		const glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), glm::vec3(size.x*sizeFactor.x, size.y*sizeFactor.y, 0.0f));
		const glm::mat4 pivotOffset = glm::translate(glm::identity<glm::mat4>(), glm::vec3(transform.m_Pivot.x * sizeFactor.x * size.x, transform.m_Pivot.y * sizeFactor.y * size.y, 0.0f));

		result = parentTransform*translation*rotation*glm::inverse(pivotOffset)*scale;
		parentTransform = parentTransform*translation*rotation;
	}

	void UIEditor::OnGUI()
	{
		EditorApplication* pApp = EditorApplication::GetInstance();
		Engine* pEngine = pApp->GetEngine();
		UIRendererModule* pUIRenderer = pEngine->GetOptionalModule<UIRendererModule>();
		RendererModule* pRenderer = pEngine->GetMainModule<RendererModule>();
		GraphicsDevice* pDevice = pEngine->ActiveGraphicsDevice();

		UIMainWindow* pMainWindow = GetMainWindow();
		UIDocument* pDocument = pMainWindow->CurrentDocument();

		MenuBar(pMainWindow);

		if (pMainWindow->CurrentDocumentID() == 0 || !pDocument) return;

		TextureHandle texture = pDocument->GetUITexture(pDevice, pRenderer->GetCurrentFrameInFlight());

		uint32_t width, height;
		pDocument->GetResolution(width, height);
		float textureAspect = (float)width / (float)height;

		ImVec2 pos = ImGui::GetWindowPos();
		const ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		pos = pos + vMin;
		ImVec2 vMax = ImGui::GetWindowContentRegionMax();
		vMax = vMax - vMin;
		float maxWidth = vMax.x;
		float maxHeight = vMax.y;

		float actualHeight = maxWidth / textureAspect;

		if (actualHeight > maxHeight)
		{
			float diff = actualHeight - maxHeight;
			float widthOffset = diff * textureAspect;
			maxWidth -= widthOffset;
			actualHeight = maxWidth / textureAspect;
		}

		const ImVec2 halfMax = vMax / 2.0f;
		const ImVec2 center = pos + halfMax;

		const ImVec2 halfOffsets = ImVec2(maxWidth / 2.0f, actualHeight / 2.0f);

		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform().GetRenderImpl();

		const ImVec2 topLeft = center - halfOffsets;
		const ImVec2 bottomRight = center + halfOffsets;
		const ImVec2 bottomLeft{ topLeft.x, bottomRight.y };

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddImage(
			pRenderImpl->GetTextureID(texture), topLeft,
			bottomRight, ImVec2(0, 1), ImVec2(1, 0));

		/* Selection box */
		const UUID selected = pMainWindow->SelectedEntity();
		if (!selected || !pDocument->EntityExists(selected)) return;

		const ImVec2 drawnSize = bottomRight - topLeft;
		const glm::vec2 sizeFactor{ drawnSize.x/width , drawnSize.y/height };

		const Utils::ECS::EntityID entity = pDocument->EntityID(selected);
		glm::mat4 parentTransform{ glm::identity<glm::mat4>() };
		glm::mat4 finalTransform{ glm::identity<glm::mat4>() };
		CalculateTransform(entity, pDocument->Registry(), parentTransform, finalTransform, sizeFactor);

		const bool mouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left) && m_IsFocused;
		static bool dragging = false;
		static ImVec2 mousePosLastFrame{};
		static glm::vec4 startRect;

		MeshData* pMesh = pUIRenderer->GetImageMesh();
		const float* vertices = pMesh->Vertices();

		std::vector<ImVec2> points(4);
		for (size_t i = 0; i < pMesh->VertexCount(); ++i)
		{
			auto v = vertices + pMesh->VertexSize()/sizeof(float)*i;
			const VertexPosColorTex* vertex = reinterpret_cast<const VertexPosColorTex*>(v);
			glm::vec4 point{ vertex->Pos, 0.0f, 1.0f };
			point = finalTransform * point;
			points[i] = ImVec2{ topLeft.x + point.x, topLeft.y + point.y };
		}

		drawList->AddLine(points[0], points[1], ImColor(255, 255, 255, 255));
		drawList->AddLine(points[1], points[2], ImColor(255, 255, 255, 255));
		drawList->AddLine(points[2], points[3], ImColor(255, 255, 255, 255));
		drawList->AddLine(points[3], points[0], ImColor(255, 255, 255, 255));
		drawList->AddLine(points[0], points[2], ImColor(255, 255, 255, 255));
		drawList->AddLine(points[1], points[3], ImColor(255, 255, 255, 255));

		for (size_t i = 0; i < points.size(); ++i)
		{
			drawList->AddCircleFilled(points[i], 6.5f, ImColor(0, 0, 255, 255), 100);
			drawList->AddCircle(points[i], 6.5f, ImColor(255, 255, 255, 255), 100);
		}

		UITransform& transform = pDocument->Registry().GetComponent<UITransform>(entity);
		const glm::vec4 pivotPoint = finalTransform*glm::vec4(transform.m_Pivot, 0.0f, 1.0f);
		const ImVec2 pivot{ topLeft.x + pivotPoint.x, topLeft.y + pivotPoint.y };
		drawList->AddCircle(pivot, 6.5f, ImColor(255, 255, 255, 255), 100);

		/*if (mouseClicked && ImGui::IsMouseHoveringRect(points[0], points[2]))
		{
			dragging = true;
			mousePosLastFrame = ImGui::GetMousePos();
			startRect = transform.m_Rect.m_Value;
		}
		if (dragging && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			dragging = false;
			auto rectValue = file["Entities"][std::to_string(selected)]["Components"][0]["Properties\\m_Rect\\m_Value"];
			Undo::StartRecord("UI Property Change", pDocument->OriginalDocumentID());
			Undo::ApplyYAMLEdit(file, rectValue.Path(), startRect, transform.m_Rect.m_Value);
			Undo::StopRecord();
		}

		if (dragging)
		{
			const ImVec2 mousePos = ImGui::GetMousePos();
			const ImVec2 delta = mousePos - mousePosLastFrame;
			mousePosLastFrame = mousePos;
			transform.m_Rect.m_Value += glm::vec4{ delta.x*1.0f/sizeFactor.x, -delta.y*1.0f/sizeFactor.y, delta.x*1.0f/sizeFactor.x, -delta.y*1.0f/sizeFactor.y };
			pDocument->Registry().SetEntityDirty(entity);
		}*/
	}

	void UIEditor::Update()
	{
	}

	void UIEditor::Draw()
	{
	}

	void UIEditor::MenuBar(UIMainWindow* pMainWindow)
	{
		if (ImGui::BeginMenuBar())
		{
			const glm::uvec2& selectedResolution = Resolutions[SelectedAspect][SelectedResolution];
			std::stringstream str;
			str << selectedResolution.x << "x" << selectedResolution.y;

			if (ImGui::BeginMenu(str.str().data()))
			{
				for (size_t i = 0; i < AspectRatioCount; ++i)
				{
					if (ImGui::BeginMenu(AspectRatios[i]))
					{
						for (size_t j = 0; j < Resolutions[i].size(); ++j)
						{
							const glm::uvec2& resolution = Resolutions[i][j];
							str.str("");
							str << resolution.x << "x" << resolution.y;
							if (ImGui::MenuItem(str.str().data(), NULL, SelectedAspect == i && SelectedResolution == j))
							{
								SelectedAspect = i;
								SelectedResolution = j;
								CustomResolution = resolution;
								pMainWindow->SetResolution(resolution);
							}
						}
						ImGui::EndMenu();
					}
				}
				if (ImGui::BeginMenu("Custom"))
				{
					EditorUI::PushFlag(EditorUI::Flag::NoLabel);
					const bool change = EditorUI::InputFloat2("Resolution", &CustomResolution, 1, FLT_MAX, 1);
					EditorUI::PopFlag();
					if (change)
					{
						pMainWindow->SetResolution(CustomResolution);
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}

	UIMainWindow* UIEditor::GetMainWindow()
	{
		return static_cast<UIMainWindow*>(m_pOwner);
	}
}
