#include "JoltEditorExtension.h"
#include "PhysicsSettings.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhase.h>

#include <BroadPhaseImpl.h>

#include <EditorPlayer.h>
#include <LayerRef.h>
#include <JoltPhysicsModule.h>
#include <GLORY_YAML.h>

EXTENSION_CPP(JoltEditorExtension)

namespace Glory::Editor
{
	PhysicsSettings Physics;

	JoltEditorExtension::JoltEditorExtension()
	{
	}

	JoltEditorExtension::~JoltEditorExtension()
	{
	}

	void JoltEditorExtension::Initialize()
	{
		EditorPlayer::RegisterLoopHandler(this);

		ProjectSettings::Add(&Physics);
	}

	const char* JoltEditorExtension::ModuleName()
	{
		return "Jolt Physics";
	}

	void JoltEditorExtension::HandleBeforeStart(Module* pModule)
	{
		JoltPhysicsModule* pPhysics = static_cast<JoltPhysicsModule*>(pModule);

		std::map<JPH::ObjectLayer, JPH::BroadPhaseLayer> bpLayersMapping;
		bpLayersMapping.emplace(0, 0);
		for (size_t i = 0; i < pPhysics->Settings().ArraySize("BroadPhaseLayerMapping"); ++i)
		{
			const std::string valueStr = pPhysics->Settings().ArrayValue<std::string>("BroadPhaseLayerMapping", i);
			BPLayer bpLayer;
			Enum<BPLayer>().FromString(valueStr, bpLayer);
			bpLayersMapping.emplace((uint16_t)i + 1, JPH::BroadPhaseLayer(JPH::uint8(bpLayer)));
		}
		pPhysics->BPLayerImpl().SetObjectToBroadphase(std::move(bpLayersMapping));

		const size_t bpLayerCount = Enum<BPLayer>().NumValues();
		std::vector<LayerMask> bpCollisionMapping = std::vector<LayerMask>(bpLayerCount);
		for (size_t i = 0; i < bpLayerCount; i++)
		{
			std::string layerName;
			Enum<BPLayer>().ToString(BPLayer(i), layerName);
			const LayerMask mask = pPhysics->Settings().Value<LayerMask>(layerName + "CollisionMask");
			bpCollisionMapping[i] = mask;
		}
		pPhysics->BPCollisionFilter().SetBPCollisionMapping(std::move(bpCollisionMapping));

		pPhysics->SetupPhysics();
	}

	void JoltEditorExtension::HandleStart(Module* pModule)
	{
	}

	void JoltEditorExtension::HandleStop(Module* pModule)
	{
		JoltPhysicsModule* pPhysics = static_cast<JoltPhysicsModule*>(pModule);
		pPhysics->CleanupPhysics();
	}

	void JoltEditorExtension::HandleUpdate(Module* pModule)
	{
	}
}
