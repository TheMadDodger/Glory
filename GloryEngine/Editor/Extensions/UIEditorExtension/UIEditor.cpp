#include "UIEditor.h"
#include "UIDocumentImporter.h"
#include "UIMainWindow.h"

#include <UIDocument.h>
#include <UIDocumentData.h>
#include <UIRendererModule.h>
#include <Debug.h>
#include <GraphicsModule.h>

#include <EditorApplication.h>
#include <EditableResource.h>
#include <EditorResourceManager.h>
#include <UIDocument.h>
#include <MeshData.h>
#include <NodeRef.h>
#include <ImGuiHelpers.h>

#include <fstream>
#include <sstream>

namespace Glory::Editor
{
	UIEditor::UIEditor() : EditorWindowTemplate("UI Editor", 600.0f, 600.0f)
	{
	}

	UIEditor::~UIEditor()
	{
	}

	void UIEditor::OnGUI()
	{
		UIMainWindow* pMainWindow = GetMainWindow();
		UIDocument* pDocument = pMainWindow->CurrentDocument();

		if (pMainWindow->CurrentDocumentID() == 0 || !pDocument) return;

		RenderTexture* pUITexture = pDocument->GetUITexture();
		if (pUITexture == nullptr) return;
		Texture* pTexture = pUITexture->GetTextureAttachment(0);

		uint32_t width, height;
		pUITexture->GetDimensions(width, height);
		float textureAspect = (float)width / (float)height;

		ImVec2 pos = ImGui::GetWindowPos();
		ImVec2 vMin = ImGui::GetWindowContentRegionMin();
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

		ImGui::GetWindowDrawList()->AddImage(
			pRenderImpl->GetTextureID(pTexture), topLeft,
			bottomRight, ImVec2(0, 1), ImVec2(1, 0));
	}

	void UIEditor::Update()
	{
	}

	void UIEditor::Draw()
	{
		UIMainWindow* pMainWindow = GetMainWindow();
		UIDocument* pDocument = pMainWindow->CurrentDocument();
		const UUID documentID = pMainWindow->CurrentDocumentID();
		const glm::uvec2 resolution = pMainWindow->Resolution();
		if (documentID == 0 || !pDocument) return;
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		UIRendererModule* pRenderer = pEngine->GetOptionalModule<UIRendererModule>();

		UIRenderData data;
		data.m_DocumentID = documentID;
		data.m_ObjectID = 0;
		data.m_TargetCamera = 0;
		data.m_Resolution = glm::vec2(resolution.x, resolution.y);

		pRenderer->DrawDocument(pDocument, data);
	}

	UIMainWindow* UIEditor::GetMainWindow()
	{
		return static_cast<UIMainWindow*>(m_pOwner);
	}
}
