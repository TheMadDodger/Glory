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

#include <fstream>
#include <sstream>

namespace Glory::Editor
{
	UIEditor::UIEditor() : EditorWindowTemplate("UI Editor", 600.0f, 600.0f), m_pDocument(nullptr)
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
		uiTextureInfo.Width = uint32_t(1920.0f);
		uiTextureInfo.Height = uint32_t(1080.0f);
		uiTextureInfo.Attachments.push_back(Attachment("UIColor", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));

		RenderTexture* pTexture = pResourceManager->CreateRenderTexture(uiTextureInfo);
		m_pDocument->SetRenderTexture(pTexture);
	}

	void UIEditor::OnGUI()
	{
		//if (m_EditingDocument == 0 || !m_pDocument) return;
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
		data.m_Resolution = glm::vec2(1920.0f, 1080.0f);

		pRenderer->DrawDocument(m_pDocument, data);
	}
}
