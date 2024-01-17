#pragma once
#include <ScriptingExtender.h>

namespace Glory
{
	class Engine;

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
		static void SetEngine(Engine* pEngine);
	};
}
