#pragma once
#include <Glory.h>

#include <AssetReference.h>
#include <Reflection.h>

#include <SoundMaterialData.h>

namespace Glory
{
	struct SoundOccluder
	{
		GLORY_API SoundOccluder() {}

		REFLECTABLE(SoundOccluder,
			(AssetReference<SoundMaterialData>)(m_MaterialAsset),
			(SoundMaterial)(m_Material)
		);
	};
}