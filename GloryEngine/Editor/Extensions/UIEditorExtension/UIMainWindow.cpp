#include "UIMainWindow.h"
#include "UIDocumentImporter.h"

#include <EditorResourceManager.h>
#include <EditableResource.h>
#include <EditorApplication.h>
#include <EditorAssetDatabase.h>
#include <Undo.h>

#include <Engine.h>
#include <GraphicsModule.h>
#include <Serializers.h>

#include <UIDocument.h>
#include <UIDocumentData.h>

namespace Glory::Editor
{
	UIMainWindow::UIMainWindow(): m_EditingDocumentIndex(0), m_Resolution(1920.0f, 1080.0f), m_SelectedEntity(0)
	{
	}

	UIMainWindow::~UIMainWindow()
	{
		for (size_t i = 0; i < m_pDocuments.size(); ++i)
		{
			delete m_pDocuments[i];
		}
		m_pDocuments.clear();
	}

	void UIMainWindow::SetDocument(UUID documentID)
	{
		if (m_EditingDocument == documentID) return;

		for (size_t i = 0; i < m_pDocuments.size(); ++i)
		{
			if (m_pDocuments[i]->OriginalDocumentID() != documentID) continue;
			m_EditingDocumentIndex = i;
			m_EditingDocument = m_pDocuments[i]->OriginalDocumentID();
			m_SelectedEntity = 0;
			return;
		}

		m_SelectedEntity = 0;
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
		for (auto iter = entities.Begin(); iter != entities.End(); ++iter)
		{
			Utils::NodeValueRef entity = entities[*iter];
			UIDocumentImporter::DeserializeEntity(pEngine, &document, entity);
		}
		document.SetName(EditorAssetDatabase::GetAssetName(documentID));
		document.SetResourceUUID(documentID);
		m_EditingDocumentIndex = m_pDocuments.size();
		m_pDocuments.push_back(new UIDocument(&document));

		RenderTextureCreateInfo uiTextureInfo;
		uiTextureInfo.HasDepth = false;
		uiTextureInfo.Width = m_Resolution.x;
		uiTextureInfo.Height = m_Resolution.y;
		uiTextureInfo.Attachments.push_back(Attachment("UIColor", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));

		RenderTexture* pTexture = pResourceManager->CreateRenderTexture(uiTextureInfo);
		m_pDocuments[m_EditingDocumentIndex]->SetRenderTexture(pTexture);
	}

	UUID UIMainWindow::CurrentDocumentID() const
	{
		return m_EditingDocument;
	}

	UIDocument* UIMainWindow::CurrentDocument()
	{
		if (m_EditingDocumentIndex >= m_pDocuments.size()) return nullptr;
		return m_pDocuments[m_EditingDocumentIndex];
	}

	const glm::uvec2& UIMainWindow::Resolution() const
	{
		return m_Resolution;
	}

	UUID& UIMainWindow::SelectedEntity()
	{
		return m_SelectedEntity;
	}

	UIDocument* UIMainWindow::FindEditingDocument(UUID uuid) const
	{
		for (size_t i = 0; i < m_pDocuments.size(); ++i)
		{
			if (m_pDocuments[i]->OriginalDocumentID() != uuid) continue;
			return m_pDocuments[i];
		}
		return nullptr;
	}

	std::string_view UIMainWindow::Name()
	{
		return "UI Editing";
	}

	void UIMainWindow::OnGui(float height)
	{
		Dockspace(height);
	}

	void UIMainWindow::Initialize()
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		Serializers& serializers = pEngine->GetSerializers();

		Undo::RegisterChangeHandler(std::string(".gui"), std::string("Components"),
		[this, &serializers](Utils::YAMLFileRef& file, const std::filesystem::path& path) {
			std::vector<std::string> components;
			Reflect::Tokenize(path.string(), components, '\\');
			if (components.size() <= 5) return;

			const std::filesystem::path filePath = file.Path();
			const UUID documentID = EditorAssetDatabase::FindAssetUUID(filePath.string());

			const size_t entityUUID = (size_t)std::stoull(components[1]);
			const size_t index = (size_t)std::stoull(components[3].substr(2));

			UIDocument* pDocument = FindEditingDocument(documentID);
			Utils::ECS::EntityID entity = pDocument->EntityID(entityUUID);

			std::filesystem::path componentPath = components[0];
			componentPath.append(components[1]).append(components[2]).append(components[3]);
			const UUID componentID = file[componentPath]["UUID"].As<uint64_t>();
			const uint32_t componentType = file[componentPath]["TypeHash"].As<uint64_t>();
			const Utils::Reflect::TypeData* pType = Reflect::GetTyeData(componentType);

			Utils::ECS::EntityRegistry& registry = pDocument->Registry();
			void* data = registry.GetComponentAddress(entity, componentID);
			serializers.DeserializeProperty(pType, data, file[componentPath]["Properties"]);

			registry.SetEntityDirty(entity);
		});
	}
}
