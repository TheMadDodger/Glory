#pragma once
#include "GloryEditor.h"
#include "ProjectSpace.h"

namespace Glory::Editor
{
	GLORY_EDITOR_API void Migrate(ProjectSpace* pProject);

	/** @brief Add default texture assets for every image data
	 * and update material texture properties to point to the default texture
	 * @param pProject Project to migrate assets in
	 */
	GLORY_EDITOR_API void Migrate_0_1_1_DefaultTextureAndMaterialProperties(ProjectSpace* pProject);

	/** @brief Add gravity value to PhysicsSettings
	 * @param pProject Project to migrate settings in
	 */
	GLORY_EDITOR_API void Migrate_0_1_1_AddGravityToPhysicsSettings(ProjectSpace* pProject);

	/** @brief Convert material property arrays to maps
	 * @param pProject Project to migrate assets in
	 */
	GLORY_EDITOR_API void Migrate_0_3_0_ConvertMaterialPropertiesToMap(ProjectSpace* pProject);

	/** @brief Update paths to ASSIMP assets to their new location
	 * @param pProject Project to migrate paths in
	 */
	GLORY_EDITOR_API void Migrate_0_3_0_ASSIMPAssets(ProjectSpace* pProject);

	/** @brief Update asset IDs from built-in module assets to the default ID
	 * @param pProject Project to migrate IDs in
	 *
	 * Also updates any materials still using old IDs
	 */
	GLORY_EDITOR_API void Migrate_0_3_0_ModuleAssetIDS(ProjectSpace* pProject);

	/** @brief Update materials to use a pipeline instead of shaders directly
	 * @param pProject Project to migrate materials in
	 */
	GLORY_EDITOR_API void Migrate_0_3_0_MaterialPipelines(ProjectSpace* pProject);

	/** @brief Load prefabs and rewrite them to match scene serialization
	 * @param pProject Project to migrate prefabs in
	 */
	GLORY_EDITOR_API void Migrate_0_3_0_PrefabScenes(ProjectSpace* pProject);

	/** @brief Convert material instances to regular materials
	 * @param pProject Project to migrate material instances in
	 */
	GLORY_EDITOR_API void Migrate_0_6_0_MaterialInstances(ProjectSpace* pProject);

	/** @brief Updates the paths to shaders and pipelines from the old renderer to the new renderer
	 * @param pProject Project to migrate assets in
	 */
	GLORY_EDITOR_API void Migrate_0_6_0_MoveShaderAndPipelineAssets(ProjectSpace* pProject);

	/** @brief Remove the shaders and pipelines that no longer exist after the renderer rewrite from the project
	 * @param pProject Project to remove shaders and pipelines in
	 */
	GLORY_EDITOR_API void Migrate_0_6_0_RemoveShaderAndPipelineAssets(ProjectSpace* pProject);
}
