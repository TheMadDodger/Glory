#pragma once
#include <GScene.h>
#include <Glory.h>
#include <EntityRegistry.h>

using namespace Glory::Utils::ECS;

namespace Glory
{
	class Entity;
	class EntitySceneObject;
	class EntityPrefabData;
	struct PrefabNode;

	class EntityScene : public GScene
	{
	public:
		EntityScene();
		EntityScene(const std::string& sceneName = "New Scene");
		EntityScene(const std::string& sceneName, UUID uuid);
		virtual ~EntityScene();

		GLORY_API Entity CreateEntity(UUID uuid, UUID transUUID);

		GLORY_API EntitySceneObject* GetEntitySceneObjectFromEntityID(EntityID entity);
		GLORY_API EntityRegistry* GetRegistry();

		GLORY_API bool IsValid() const;

		GLORY_API SceneObject* InstantiatePrefab(const std::map<UUID, UUID>& idRemap, EntityPrefabData* pPrefab);

	private:
		virtual void Initialize() override;
		virtual void OnTick() override;
		virtual void OnPaint() override;

		virtual SceneObject* CreateObject(const std::string& name) override;
		virtual SceneObject* CreateObject(const std::string& name, UUID uuid, UUID uuid2) override;
		virtual void OnDeleteObject(SceneObject* pObject) override;

		virtual void OnObjectAdded(SceneObject* pObject) override;

		virtual void Start() override;
		virtual void Stop() override;

		EntitySceneObject* InstantiatePrefabNode(const std::map<UUID, UUID>& idRemap, EntitySceneObject* pParent, const PrefabNode& node);

	private:
		friend class Entity;
		friend class EntitySceneSerializer;
		EntityRegistry m_Registry;
		bool m_Valid;
		std::unordered_map<EntityID, EntitySceneObject*> m_EntityIDToObject;
	};
}
