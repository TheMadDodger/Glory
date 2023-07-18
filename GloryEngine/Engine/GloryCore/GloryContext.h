#pragma once
#include "Game.h"
#include "CameraManager.h"
#include "ThreadedVar.h"

namespace Glory::Utils::Reflect
{
	class Reflect;
}

namespace Glory
{
	class AssetDatabase;
	class AssetManager;
	class ResourceTypes;
	class Serializers;
	class DisplayManager;
	class Console;
	class LayerManager;

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
		static Console* GetConsole();
		static LayerManager* GetLayerManager();
		static Debug* GetDebug();
		static void SetContext(GloryContext* pContext);
		static GloryContext* GetContext();
		static GloryContext* CreateContext();
		static void DestroyContext();

		template<class T>
		static void AddUserContext(T* pUserContext)
		{
			AddUserContext(ResourceType::GetHash(typeid(T)), (void*)pUserContext);
		}

		template<class T>
		static T* GetUserContext()
		{
			return (T*)GetUserContext(ResourceType::GetHash(typeid(T)));
		}

		static void AddUserContext(uint32_t hash, void* pUserContext);
		static void* GetUserContext(uint32_t hash);

		std::map<UUID, UUID> m_UUIDRemapper;

	private:
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
		Console* m_pConsole;
		LayerManager* m_pLayerManager;
		Reflect* m_pReflection;
		ThreadedVector<Object*> m_pAllObjects;
		Debug* m_pDebug;
		std::map<size_t, void*> m_pUserContexts;
	};
}