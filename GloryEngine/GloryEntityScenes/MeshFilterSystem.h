#pragma once
#include "EntitySystemTemplate.h"
#include "Components.h"

namespace Glory
{
	class MeshFilterSystem : public EntitySystemTemplate<MeshFilter>
	{
	public:
		MeshFilterSystem(Registry* pRegistry);
		virtual ~MeshFilterSystem();

	private:
		virtual void OnAcquireSerializedProperties(std::vector<SerializedProperty>& properties, MeshFilter& pComponent) override;
		virtual std::string Name() override;
	};
}
