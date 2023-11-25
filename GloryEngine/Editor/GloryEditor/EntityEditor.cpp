#include "EntityEditor.h"
#include "EditableEntity.h"

#include <GScene.h>
#include <map>
#include <vector>
#include <memory>
#include <EditorSceneManager.h>

namespace Glory::Editor
{
    std::map<UUID, std::vector<std::unique_ptr<EditableEntity>>> editableEntities;

    EditableEntity* Create(Utils::ECS::EntityID entity, UUID sceneID, std::vector<std::unique_ptr<EditableEntity>>& entities)
    {
        if (entities.size() <= entity)
            entities.resize(entity + 1);

        UUID entityID = EditorSceneManager::GetOpenScene(sceneID)->GetEntityUUID(entity);
        entities[entity].reset(new EditableEntity(entity, entityID, sceneID));

        return entities[entity].get();
    }

    EditableEntity* GetEditableEntity(Utils::ECS::EntityID entity, UUID sceneID)
    {
        auto itor = editableEntities.find(sceneID);
        if (itor == editableEntities.end())
        {
            editableEntities.emplace(sceneID, std::vector<std::unique_ptr<EditableEntity>>{1});
            return Create(entity, sceneID, editableEntities.at(sceneID));
        }

        if (itor->second.size() <= entity || !itor->second[entity])
        {
            return Create(entity, sceneID, itor->second);
        }

        return itor->second[entity].get();
    }

    EditableEntity* GetEditableEntity(Utils::ECS::EntityID entity, GScene* pScene)
    {
        return GetEditableEntity(entity, pScene->GetUUID());
    }
}
