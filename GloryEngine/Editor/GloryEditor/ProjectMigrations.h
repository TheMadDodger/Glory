#pragma once
#include "GloryEditor.h"
#include "ProjectSpace.h"

namespace Glory::Editor
{
	GLORY_EDITOR_API void Migrate(ProjectSpace* pProject);

	GLORY_EDITOR_API void Migrate_0_1_1_AddDefaultTextureInImageDatas(ProjectSpace* pProject);
}
