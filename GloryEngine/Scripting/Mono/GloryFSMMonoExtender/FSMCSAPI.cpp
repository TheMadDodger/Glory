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
		const FSMNode* node = pFSMData->FindNode(nameStr);
		return node ? node->m_ID : 0;
	}

#pragma endregion

#pragma region FSM Manager

	void FSMManager_DestroyInstance(uint64_t instanceId)
	{
		FSM_MODULE->DestroyFSMState(instanceId);
	}

#pragma endregion

#pragma region FSM Node

	unsigned int FSMNode_GetTransitionCount(uint64_t fsmId, uint64_t nodeId)
	{
		Resource* pFSMResource = FSM_EngineInstance->GetAssetManager().FindResource(fsmId);
		if (!pFSMResource) return 0;
		FSMData* pFSMData = static_cast<FSMData*>(pFSMResource);
		const FSMNode* node = pFSMData->Node((UUID)nodeId);
		if (!node) return 0;
		return (unsigned int)node->m_Transitions.size();
	}

	uint64_t FSMNode_FindTransitionID(uint64_t fsmId, uint64_t nodeId, MonoString* name)
	{
		Resource* pFSMResource = FSM_EngineInstance->GetAssetManager().FindResource(fsmId);
		if (!pFSMResource) return 0;
		FSMData* pFSMData = static_cast<FSMData*>(pFSMResource);
		const FSMNode* node = pFSMData->Node((UUID)nodeId);
		if (!node) return 0;
		
		const std::string_view nameStr = mono_string_to_utf8(name);
		const FSMTransition* transition = pFSMData->FindTransition(nameStr);
		if (!transition) return 0;

		for (size_t i = 0; i < node->m_Transitions.size(); ++i)
		{
			if (node->m_Transitions[i] != transition->m_ID) continue;
			return transition->m_ID;
		}
		return 0;
	}

	uint64_t FSMNode_GetTransitionID(uint64_t fsmId, uint64_t nodeId, unsigned int index)
	{
		Resource* pFSMResource = FSM_EngineInstance->GetAssetManager().FindResource(fsmId);
		if (!pFSMResource) return 0;
		FSMData* pFSMData = static_cast<FSMData*>(pFSMResource);
		const FSMNode* node = pFSMData->Node((UUID)nodeId);
		if (!node) return 0;

		if (index >= node->m_Transitions.size()) return 0;
		return node->m_Transitions[index];
	}

#pragma endregion

#pragma region Binding

	void FSMCSAPI::AddInternalCalls(std::vector<InternalCall>& internalCalls)
	{
        /* FSM Data */
        BIND("GloryEngine.FSM.FSMTemplate::FSMTemplate_CreateInstance", FSMTemplate_CreateInstance);
        BIND("GloryEngine.FSM.FSMTemplate::FSMTemplate_FindNode", FSMTemplate_FindNode);

		/* FSM Node */
        BIND("GloryEngine.FSM.FSMNode::FSMNode_GetTransitionCount", FSMNode_GetTransitionCount);
        BIND("GloryEngine.FSM.FSMNode::FSMNode_FindTransitionID", FSMNode_FindTransitionID);
        BIND("GloryEngine.FSM.FSMNode::FSMNode_GetTransitionID", FSMNode_GetTransitionID);


		/* FSM Manager */
        BIND("FSMManager.FSM.FSMManager::FSMManager_DestroyInstance", FSMManager_DestroyInstance);
	}

	void FSMCSAPI::SetEngine(Engine* pEngine)
	{
		FSM_EngineInstance = pEngine;
	}

#pragma endregion

}