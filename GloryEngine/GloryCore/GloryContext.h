#pragma once
#include "Game.h"
#include "CameraManager.h"

namespace Glory
{
	class AssetDatabase;
	class AssetManager;
	class ResourceTypes;
	class Serializers;
	class DisplayManager;
	class SerializedPropertyManager;

	class GloryContext
	{
	public:
		static Game* GetGame();
		static CameraManager* GetCameraManager();
		static AssetDatabase* GetAssetDatabase();
		static AssetManager* GetAssetManager();
		static ResourceTypes* GetResourceTypes();
		static Serializers* GetSerializers();
		static DisplayManager* GetDisplayManager();
		static SerializedPropertyManager* GetSerializedPropertyManager();
		static void SetContext(GloryContext* pContext);
		static GloryContext* GetContext();
		static GloryContext* CreateContext();

	private:
		static void DestroyContext();
		void Initialize();

	private:
		GloryContext();
		virtual ~GloryContext();

	private:
		friend class Engine;
		friend class Object;
		static GloryContext* m_pContext;
		Game* m_Game;
		CameraManager m_CameraManager;
		AssetDatabase* m_pAssetDatabase;
		AssetManager* m_pAssetManager;
		ResourceTypes* m_pResourceTypes;
		Serializers* m_pSerializers;
		DisplayManager* m_pDisplayManager;

		std::vector<Object*> m_pAllObjects;

		SerializedPropertyManager* m_pSerializedPropertyManager;
	};
}