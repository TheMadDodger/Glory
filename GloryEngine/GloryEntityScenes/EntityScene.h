#pragma once
#include <vector>
#include <JobManager.h>
#include <GScene.h>
#include <Glory.h>
#include "Registry.h"

namespace Glory
{
	class Entity;
	class EntitySceneObject;

	class EntityScene : public GScene
	{
	public:
		EntityScene();
		EntityScene(const std::string& sceneName = "New Scene");
		EntityScene(const std::string& sceneName, UUID uuid);
		virtual ~EntityScene();

		GLORY_API Entity CreateEntity();

		GLORY_API EntitySceneObject* GetEntitySceneObjectFromEntityID(EntityID entity);
		GLORY_API Registry* GetRegistry();

	private:
		virtual void Initialize() override;
		virtual void OnTick() override;
		virtual void OnPaint() override;

		virtual SceneObject* CreateObject(const std::string& name) override;
		virtual SceneObject* CreateObject(const std::string& name, UUID uuid) override;
		virtual void OnDeleteObject(SceneObject* pObject) override;

		virtual void OnObjectAdded(SceneObject* pObject) override;

	private:
		friend class Entity;
		friend class EntitySceneSerializer;
		Registry m_Registry;
		std::unordered_map<EntityID, EntitySceneObject*> m_EntityIDToObject;
	};
}
