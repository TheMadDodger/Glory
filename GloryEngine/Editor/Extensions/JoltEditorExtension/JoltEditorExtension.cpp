#include "JoltEditorExtension.h"

#include <EditorPlayer.h>
#include <LayerRef.h>
#include <JoltPhysicsModule.h>
#include <GLORY_YAML.h>

Glory::Editor::BaseEditorExtension* LoadExtension()
{
	return new Glory::Editor::JoltEditorExtension();
}

void SetContext(Glory::GloryContext* pContext, ImGuiContext* pImGUIContext)
{
	Glory::GloryContext::SetContext(pContext);
	ImGui::SetCurrentContext(pImGUIContext);
}

namespace Glory::Editor
{
	JoltEditorExtension::JoltEditorExtension()
	{
	}

	JoltEditorExtension::~JoltEditorExtension()
	{
	}

	void JoltEditorExtension::RegisterEditors()
	{
		EditorPlayer::RegisterLoopHandler(this);
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
			GloryReflect::Enum<BPLayer>().FromString(valueStr, bpLayer);
			bpLayersMapping.emplace(i + 1, JPH::BroadPhaseLayer(JPH::uint8(bpLayer)));
		}
		pPhysics->BPLayerImpl().SetObjectToBroadphase(std::move(bpLayersMapping));

		const size_t bpLayerCount = GloryReflect::Enum<BPLayer>().NumValues();
		std::vector<LayerMask> bpCollisionMapping = std::vector<LayerMask>(bpLayerCount);
		for (size_t i = 0; i < bpLayerCount; i++)
		{
			std::string layerName;
			GloryReflect::Enum<BPLayer>().ToString(BPLayer(i), layerName);
			const LayerMask mask = pPhysics->Settings().Value<LayerMask>(layerName + "CollisionMask");
			bpCollisionMapping[i] = mask;
		}
		pPhysics->BPCollisionFilter().SetBPCollisionMapping(std::move(bpCollisionMapping));
	}

	void JoltEditorExtension::HandleStart(Module* pModule)
	{
	}

	void JoltEditorExtension::HandleStop(Module* pModule)
	{
	}

	void JoltEditorExtension::HandleUpdate(Module* pModule)
	{
	}
}
