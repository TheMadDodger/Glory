#pragma once
#include <cstdint>
#include <vector>
#include <Glory.h>

namespace Glory
{
	class InternalCall;
	class Engine;
	class GScene;

	GLORY_API GScene* GetEntityScene(uint64_t sceneID);

	class EntityCSAPI
	{
	public:
		static void GetInternallCalls(std::vector<InternalCall>& internalCalls);
		static void SetEngine(Engine* pEngine);
	};
}
