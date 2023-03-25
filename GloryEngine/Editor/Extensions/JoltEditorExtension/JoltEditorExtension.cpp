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

	void JoltEditorExtension::HandleStart(Module* pModule)
	{
		JoltPhysicsModule* pPhysics = static_cast<JoltPhysicsModule*>(pModule);

		std::vector<JPH::BroadPhaseLayer> bpLayersMapping{ pPhysics->Settings().ArraySize("Broadphase Layer Mapping") + 1 };
		for (size_t i = 0; i < bpLayersMapping.size(); ++i)
		{
			if (!i)
			{
				bpLayersMapping[i] = JPH::BroadPhaseLayer(0);
				continue;
			}
			bpLayersMapping[i] = JPH::BroadPhaseLayer(pPhysics->Settings().ArrayValue<LayerRef>("Broadphase Layer Mapping", i - 1).m_LayerIndex);
		}
		pPhysics->BPLayerImpl().SetObjectToBroadphase(std::move(bpLayersMapping));
	}

	void JoltEditorExtension::HandleStop(Module* pModule)
	{

	}

	void JoltEditorExtension::HandleUpdate(Module* pModule)
	{

	}
}
