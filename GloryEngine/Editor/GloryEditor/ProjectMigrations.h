#pragma once
#include "GloryEditor.h"
#include "ProjectSpace.h"

namespace Glory::Editor
{
	GLORY_EDITOR_API void Migrate(ProjectSpace* pProject);

	/** @brief
	 * Add default texture assets for every image data
	 * and update material texture properties to point to the default texture
	 * @param pProject Project to migrate assets in
	 */
	GLORY_EDITOR_API void Migrate_0_1_1_DefaultTextureAndMaterialProperties(ProjectSpace* pProject);
}
