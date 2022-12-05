#pragma once
#include <ScriptingExtender.h>
#include <mono/jit/jit.h>
#include <EntityScene.h>

namespace Glory
{
	struct MonoEntityHandle
	{
		MonoEntityHandle();
		MonoEntityHandle(uint64_t entityID, uint64_t sceneID);

		uint64_t m_EntityID;
		uint64_t m_SceneID;
	};

	class EntityCSAPI
	{
	public:
		static void GetInternallCalls(std::vector<InternalCall>& internalCalls);
	};
}
