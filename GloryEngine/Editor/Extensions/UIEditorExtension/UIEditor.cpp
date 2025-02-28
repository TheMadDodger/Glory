#include "UIEditor.h"
#include "UIDocumentImporter.h"

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
	UIEditor::UIEditor() : EditorWindowTemplate("UI Editor", 600.0f, 600.0f), m_pDocument(nullptr), m_Resolution(1920.0f, 1080.0f)
	{
	}

	UIEditor::~UIEditor()
	{
		if (m_pDocument)
		{
			delete m_pDocument;
			m_pDocument = nullptr;
		}
	}

	void UIEditor::SetDocument(UUID documentID)
	{
		m_EditingDocument = documentID;
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		GraphicsModule* pGraphics = pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();
		EditorResourceManager& resources = EditorApplication::GetInstance()->GetResourceManager();
		EditableResource* pResource = resources.GetEditableResource(documentID);
		YAMLResource<UIDocumentData>* pDocument = static_cast<YAMLResource<UIDocumentData>*>(pResource);
		Utils::YAMLFileRef& file = **pDocument;

		UIDocumentData document;
		auto node = file.RootNodeRef().ValueRef();

		Utils::NodeValueRef entities = node["Entities"];
		for (size_t i = 0; i < entities.Size(); ++i)
		{
			Utils::NodeValueRef entity = entities[i];
			UIDocumentImporter::DeserializeEntity(pEngine, &document, entity);
		}
		m_pDocument = new UIDocument(&document);

		RenderTextureCreateInfo uiTextureInfo;
		uiTextureInfo.HasDepth = false;
		uiTextureInfo.Width = m_Resolution.x;
		uiTextureInfo.Height = m_Resolution.y;
		uiTextureInfo.Attachments.push_back(Attachment("UIColor", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));

		RenderTexture* pTexture = pResourceManager->CreateRenderTexture(uiTextureInfo);
		m_pDocument->SetRenderTexture(pTexture);
	}

	void UIEditor::OnGUI()
	{
		if (m_EditingDocument == 0 || !m_pDocument) return;

		RenderTexture* pUITexture = m_pDocument->GetUITexture();
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
		if (m_EditingDocument == 0 || !m_pDocument) return;
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		UIRendererModule* pRenderer = pEngine->GetOptionalModule<UIRendererModule>();

		UIRenderData data;
		data.m_DocumentID = m_EditingDocument;
		data.m_ObjectID = 0;
		data.m_TargetCamera = 0;
		data.m_Resolution = glm::vec2(m_Resolution.x, m_Resolution.y);

		pRenderer->DrawDocument(m_pDocument, data);
	}
}
