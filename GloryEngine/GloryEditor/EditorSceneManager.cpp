//#include "EditorSceneManager.h"
//#include "ContentBrowser.h"
//#include "EditorApp.h"
//
//namespace Spartan
//{
//	namespace Editor
//	{
//		GameScene* EditorSceneManager::m_pActivateScene = nullptr;
//		std::vector<GameScene*> EditorSceneManager::m_pOpenScenes = std::vector<GameScene*>();
//		std::vector<std::string> EditorSceneManager::m_LoadedScenePaths = std::vector<std::string>();
//
//		void EditorSceneManager::Initialize()
//		{
//			std::function<void(const std::filesystem::path&)> onFileDoubleClick = [](const std::filesystem::path& path)
//			{
//
//			};
//			Editor::ContentBrowser::OnFileDoubleClick.Subscribe("EditorSceneLoader_onFileDoubleClick", onFileDoubleClick);
//
//			// TODO: Check if there was a last opened scene and load that one instead of the default one?
//			LoadDefaultScene();
//		}
//
//		void EditorSceneManager::Update()
//		{
//			for (auto pScene : m_pOpenScenes)
//			{
//				EditorApp::EditorUpdateScene(pScene);
//			}
//		}
//
//		void EditorSceneManager::Draw()
//		{
//			for (auto pScene : m_pOpenScenes)
//			{
//				EditorApp::EditorDrawScene(pScene);
//			}
//		}
//
//		size_t EditorSceneManager::OpenSceneCount()
//		{
//			return m_pOpenScenes.size();
//		}
//
//		GameScene* EditorSceneManager::GetOpenSceneAt(size_t index)
//		{
//			if (index < 0 || index >= OpenSceneCount()) return nullptr;
//			return m_pOpenScenes[index];
//		}
//
//		void EditorSceneManager::Cleanup()
//		{
//			Editor::ContentBrowser::OnFileDoubleClick.Unsubscribe("EditorSceneLoader_onFileDoubleClick");
//
//			for (auto pScene : m_pOpenScenes)
//			{
//				delete pScene;
//			}
//
//			m_pOpenScenes.clear();
//			m_pActivateScene = nullptr;
//			m_LoadedScenePaths.clear();
//		}
//
//		void EditorSceneManager::LoadDefaultScene()
//		{
//			GameScene* pDefaultScene = new GameScene("Unnamed Scene");
//			m_pActivateScene = pDefaultScene;
//			AddGameScene(pDefaultScene, "");
//			pDefaultScene->CreateDefaultObjects();
//		}
//
//		GameScene* EditorSceneManager::GetActiveScene()
//		{
//			return m_pActivateScene;
//		}
//
//		void EditorSceneManager::AddGameScene(GameScene* pScene, const std::string& path)
//		{
//			m_pOpenScenes.push_back(pScene);
//			m_LoadedScenePaths.push_back(path);
//		}
//
//		EditorSceneManager::EditorSceneManager() {}
//		EditorSceneManager::~EditorSceneManager() {}
//	}
//}