//#pragma once
//#include <GameScene.h>
//
//namespace Spartan
//{
//	namespace Editor
//	{
//		class EditorSceneManager
//		{
//		public:
//			static size_t OpenSceneCount();
//			static GameScene* GetOpenSceneAt(size_t index);
//			static GameScene* GetActiveScene();
//
//		private:
//			static void Initialize();
//			static void Update();
//			static void Draw();
//
//			static void Cleanup();
//			static void LoadDefaultScene();
//
//			static void AddGameScene(GameScene* pScene, const std::string& path);
//
//		private:
//			friend class EditorApp;
//			EditorSceneManager();
//			~EditorSceneManager();
//
//		private:
//			static GameScene* m_pActivateScene;
//			static std::vector<GameScene*> m_pOpenScenes;
//			static std::vector<std::string> m_LoadedScenePaths;
//		};
//	}
//}