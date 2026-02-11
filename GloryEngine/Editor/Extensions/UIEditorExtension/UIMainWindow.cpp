#include "UIMainWindow.h"
#include "UIDocumentImporter.h"
#include "DeleteUIElementAction.h"
#include "AddUIElementAction.h"

#include <EditorResourceManager.h>
#include <EditorAssetManager.h>
#include <EditableResource.h>
#include <EditorApplication.h>
#include <EditorAssetDatabase.h>
#include <Undo.h>
#include <AssetCompiler.h>
#include <Dispatcher.h>

#include <Engine.h>
#include <Serializers.h>

#include <UIDocument.h>
#include <UIDocumentData.h>
#include <Shortcuts.h>
#include <Renderer.h>
#include <UIRendererModule.h>

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
			m_pDocuments[i]->SetDrawDirty();
			m_SelectedEntity = 0;
			return;
		}

		m_SelectedEntity = 0;
		m_EditingDocument = documentID;
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		Renderer* pRenderer = pEngine->ActiveRenderer();
		UIRendererModule* pUIRenderer = pEngine->GetOptionalModule<UIRendererModule>();
		GraphicsDevice* pDevice = pEngine->ActiveGraphicsDevice();
		EditorResourceManager& resources = EditorApplication::GetInstance()->GetResourceManager();
		EditableResource* pResource = resources.GetEditableResource(documentID);
		YAMLResource<UIDocumentData>* pDocument = static_cast<YAMLResource<UIDocumentData>*>(pResource);
		Utils::NodeValueRef rootNode = **pDocument;

		UIDocumentData document;
		Utils::NodeValueRef entities = rootNode["Entities"];
		for (auto iter = entities.Begin(); iter != entities.End(); ++iter)
		{
			Utils::NodeValueRef entity = entities[*iter];
			UIDocumentImporter::DeserializeEntity(pEngine, &document, entity);
		}
		document.SetName(EditorAssetDatabase::GetAssetName(documentID));
		document.SetResourceUUID(documentID);
		m_EditingDocumentIndex = m_pDocuments.size();
		m_pDocuments.push_back(new UIDocument(&document));
		m_pDocuments[m_EditingDocumentIndex]->CreateRenderPasses(pDevice, pRenderer->GetNumFramesInFlight(), m_Resolution, pUIRenderer);
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

	void UIMainWindow::SetResolution(const glm::uvec2& resolution)
	{
		m_Resolution = resolution;
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		Renderer* pRenderer = pEngine->ActiveRenderer();
		UIRendererModule* pUIRenderer = pEngine->GetOptionalModule<UIRendererModule>();
		GraphicsDevice* pDevice = pEngine->ActiveGraphicsDevice();

		for (size_t i = 0; i < m_pDocuments.size(); ++i)
		{
			m_pDocuments[i]->ResizeRenderTexture(pDevice, pRenderer->GetNumFramesInFlight(), resolution, pUIRenderer);
			Utils::ECS::EntityRegistry& registry = m_pDocuments[i]->Registry();
			for (size_t i = 0; i < registry.ChildCount(0); ++i)
			{
				registry.SetEntityDirty(registry.Child(0, i));
			}
			registry.InvokeAll(Utils::ECS::InvocationType::OnDirty, NULL);
		}
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
		EditorApplication* pApp = EditorApplication::GetInstance();
		Engine* pEngine = pApp->GetEngine();
		Serializers& serializers = pEngine->GetSerializers();
		EditorResourceManager& resources = pApp->GetResourceManager();

		Undo::RegisterChangeHandler(std::string(".gui"), std::string("Entities"),
		[this, pEngine](Utils::YAMLFileRef& file, const std::filesystem::path& path) {
			std::vector<std::string> components;
			Reflect::Tokenize(path.string(), components, '\\');
			if (components.size() != 2) return;
			const std::filesystem::path filePath = file.Path();
			const UUID documentID = EditorAssetDatabase::FindAssetUUID(filePath.string());
			UIDocument* pDocument = FindEditingDocument(documentID);
			const UUID entityUUID = (UUID)std::stoull(components[1]);

			auto entity = file[path];
			if (!entity.Exists())
			{
				/* Remove it */
				if (!pDocument->EntityExists(entityUUID)) return;
				const Utils::ECS::EntityID entityID = pDocument->EntityID(entityUUID);
				const Utils::ECS::EntityID parentID = pDocument->Registry().GetParent(entityID);
				pDocument->DestroyEntity(entityUUID);
				pDocument->SetEntityDirty(parentID, true, true);
				pDocument->SetDrawDirty();
				return;
			}

			/* Add it */
			UIDocumentImporter::DeserializeEntity(pEngine, pDocument, file[path]);
			const Utils::ECS::EntityID newEntityID = pDocument->EntityID(entityUUID);
			pDocument->SetEntityDirty(newEntityID, true, true);
			pDocument->SetDrawDirty();
		});

		Undo::RegisterChangeHandler(std::string(".gui"), std::string("Components"),
		[this, &serializers](Utils::YAMLFileRef& file, const std::filesystem::path& path) {
			std::vector<std::string> components;
			Reflect::Tokenize(path.string(), components, '\\');
			if (components.size() <= 5) return;

			const std::filesystem::path filePath = file.Path();
			const UUID documentID = EditorAssetDatabase::FindAssetUUID(filePath.string());

			const UUID entityUUID = (UUID)std::stoull(components[1]);
			const size_t index = (size_t)std::stoull(components[3].substr(2));

			UIDocument* pDocument = FindEditingDocument(documentID);
			const Utils::ECS::EntityID entity = pDocument->EntityID(entityUUID);
			const Utils::ECS::EntityID parent = pDocument->Registry().GetParent(entity);

			std::filesystem::path componentPath = components[0];
			componentPath.append(components[1]).append(components[2]).append(components[3]);
			const UUID componentID = file[componentPath]["UUID"].As<uint64_t>();
			const uint32_t componentType = file[componentPath]["TypeHash"].As<uint64_t>();
			const Utils::Reflect::TypeData* pType = Reflect::GetTyeData(componentType);

			Utils::ECS::EntityRegistry& registry = pDocument->Registry();
			void* data = registry.GetComponentAddress(entity, componentID);
			serializers.DeserializeProperty(pType, data, file[componentPath]["Properties"]);

			pDocument->SetEntityDirty(entity, true, true);
			pDocument->SetDrawDirty();
		});

		Undo::RegisterChangeHandler(std::string(".gui"), std::string("Name"),
		[this, &serializers](Utils::YAMLFileRef& file, const std::filesystem::path& path) {
			std::vector<std::string> components;
			Reflect::Tokenize(path.string(), components, '\\');
			if (components.size() != 3) return;

			const std::filesystem::path filePath = file.Path();
			const UUID documentID = EditorAssetDatabase::FindAssetUUID(filePath.string());
			const UUID entityUUID = (UUID)std::stoull(components[1]);

			UIDocument* pDocument = FindEditingDocument(documentID);
			const Utils::ECS::EntityID entity = pDocument->EntityID(entityUUID);
			const std::string newName = file[path].As<std::string>();
			pDocument->SetName(entity, newName);
		});

		Undo::RegisterChangeHandler(std::string(".gui"), std::string("Active"),
		[this, &serializers](Utils::YAMLFileRef& file, const std::filesystem::path& path) {
			std::vector<std::string> components;
			Reflect::Tokenize(path.string(), components, '\\');

			switch (components.size())
			{
			case 3:
			{
				const std::filesystem::path filePath = file.Path();
				const UUID documentID = EditorAssetDatabase::FindAssetUUID(filePath.string());
				const UUID entityUUID = (UUID)std::stoull(components[1]);

				UIDocument* pDocument = FindEditingDocument(documentID);
				const Utils::ECS::EntityID entity = pDocument->EntityID(entityUUID);
				const bool active = file[path].As<bool>();
				pDocument->SetEntityActive(entity, active);
				break;
			}
			case 4:

				break;

			default:
				return;
			}
		});

		Shortcuts::AddMainWindowAction("Delete", m_MainWindowIndex, [this, pEngine, &resources]() {
			UIDocument* pDocument = CurrentDocument();
			if (!m_EditingDocument || !pDocument || !m_SelectedEntity) return;

			EditableResource* pResource = resources.GetEditableResource(m_EditingDocument);
			YAMLResource<UIDocumentData>* pDocumentData = static_cast<YAMLResource<UIDocumentData>*>(pResource);
			DeleteUIElementAction::DeleteElement(pEngine, pDocument, pDocumentData->File(), m_SelectedEntity);
		});

		Shortcuts::AddMainWindowAction("Duplicate", m_MainWindowIndex, [this, pEngine, &resources]() {
			UIDocument* pDocument = CurrentDocument();
			if (!m_EditingDocument || !pDocument || !m_SelectedEntity) return;

			EditableResource* pResource = resources.GetEditableResource(m_EditingDocument);
			YAMLResource<UIDocumentData>* pDocumentData = static_cast<YAMLResource<UIDocumentData>*>(pResource);
			m_SelectedEntity = AddUIElementAction::DuplicateElement(pEngine, pDocument, pDocumentData->File(), m_SelectedEntity);
		});

		Shortcuts::AddMainWindowAction("Save Scene", m_MainWindowIndex, [this, pApp, &resources]() {
			UIDocument* pDocument = CurrentDocument();
			if (!m_EditingDocument || !pDocument) return;

			EditableResource* pResource = resources.GetEditableResource(m_EditingDocument);
			YAMLResource<UIDocumentData>* pDocumentData = static_cast<YAMLResource<UIDocumentData>*>(pResource);
			pDocumentData->Save();
		});

		AssetCompiler::GetAssetCompilerEventDispatcher().AddListener([this](const AssetCompilerEvent&) {
			UIDocument* pDocument = CurrentDocument();
			if (!pDocument) return;
			pDocument->SetDrawDirty();
		});

		pEngine->ActiveRenderer()->InjectPreRenderPass([this](GraphicsDevice* pDevice, CommandBufferHandle commandBuffer, uint32_t frameIndex) {
			UIDocument* pDocument = CurrentDocument();
			const UUID documentID = CurrentDocumentID();
			const glm::uvec2 resolution = Resolution();
			if (documentID == 0 || !pDocument) return;
			Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
			UIRendererModule* pRenderer = pEngine->GetOptionalModule<UIRendererModule>();

			UIRenderData data;
			data.m_DocumentID = documentID;
			data.m_ObjectID = 0;
			data.m_TargetCamera = 0;
			data.m_Resolution = glm::vec2(resolution.x, resolution.y);
			data.m_ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

			pRenderer->DrawDocument(pDevice, commandBuffer, frameIndex, pDocument, data);
		});
	}

	void UIMainWindow::OnUpdate()
	{
		EditorApplication* pApp = EditorApplication::GetInstance();
		EditorResourceManager& resources = pApp->GetResourceManager();
		Engine* pEngine = pApp->GetEngine();

		for (size_t i = 0; i < m_pDocuments.size(); ++i)
		{
			const UUID docID = m_pDocuments[i]->OriginalDocumentID();
			EditableResource* pResource = resources.GetEditableResource(docID);

			if (pResource->WasSaved())
			{
				pResource->WasSaved() = false;
				Resource* pDocumentResource = pApp->GetAssetManager().FindResource(docID);
				if (!pDocumentResource) return;
				UIDocumentData* pUIDcoumentData = static_cast<UIDocumentData*>(pDocumentResource);

				YAMLResource<UIDocumentData>* pDocumentData = static_cast<YAMLResource<UIDocumentData>*>(pResource);
				pUIDcoumentData->Reset();

				Utils::NodeValueRef rootNode = **pDocumentData;
				Utils::NodeValueRef entities = rootNode["Entities"];
				for (auto iter = entities.Begin(); iter != entities.End(); ++iter)
				{
					Utils::NodeValueRef entity = entities[*iter];
					UIDocumentImporter::DeserializeEntity(pEngine, pUIDcoumentData, entity);
				}
			}
		}
	}
}
