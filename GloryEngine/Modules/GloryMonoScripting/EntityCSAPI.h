#pragma once
#include <cstdint>
#include <vector>

namespace Glory
{
	class InternalCall;
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
