#pragma once
#include <ScriptingExtender.h>
#include <mono/jit/jit.h>

namespace Glory
{
	struct MonoEntityHandle
	{
		MonoEntityHandle();
		MonoEntityHandle(uint32_t entityID, uint64_t sceneID);

		uint32_t m_EntityID;
		uint64_t m_SceneID;
	};

	class EntityBindings
	{
	public:
		static void GetInternallCalls(std::vector<InternalCall>& internalCalls);

	private:
		static bool IsValid(MonoEntityHandle* pObject);
		static MonoEntityHandle GetEntityHandle(MonoObject* pObject);
	};
}
