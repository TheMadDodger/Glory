#include "FSMCSAPI.h"

#include <cstdint>
#include <GloryMonoScipting.h>
#include <Debug.h>
#include <Engine.h>
#include <ComponentTypes.h>
#include <AssetManager.h>
#include <FSM.h>
#include <FSMModule.h>

namespace Glory
{
	Engine* FSM_EngineInstance;
#define FSM_MODULE FSM_EngineInstance->GetOptionalModule<FSMModule>()

#pragma region FSM Template

	uint64_t FSMTemplate_CreateInstance(uint64_t fsmId)
	{
		Resource* pFSMResource = FSM_EngineInstance->GetAssetManager().FindResource(fsmId);
		if (!pFSMResource) return 0;
		FSMData* pFSMData = static_cast<FSMData*>(pFSMResource);
		const UUID stateID = FSM_MODULE->CreateFSMState(pFSMData);
		return stateID;
	}

	uint64_t FSMTemplate_FindNode(uint64_t fsmId, MonoString* name)
	{
		Resource* pFSMResource = FSM_EngineInstance->GetAssetManager().FindResource(fsmId);
		if (!pFSMResource) return 0;
		FSMData* pFSMData = static_cast<FSMData*>(pFSMResource);
		const std::string_view nameStr = mono_string_to_utf8(name);
		FSMNode* node = pFSMData->FindNode(nameStr);
		return node ? node->m_ID : 0;
	}

#pragma endregion


#pragma region Binding

	void FSMCSAPI::AddInternalCalls(std::vector<InternalCall>& internalCalls)
	{
        /* FSM Data */
        BIND("GloryEngine.FSM.FSMTemplate::FSMTemplate_CreateInstance", FSMTemplate_CreateInstance);
        BIND("GloryEngine.FSM.FSMTemplate::FSMTemplate_FindNode", FSMTemplate_FindNode);
	}

	void FSMCSAPI::SetEngine(Engine* pEngine)
	{
		FSM_EngineInstance = pEngine;
	}

#pragma endregion

}