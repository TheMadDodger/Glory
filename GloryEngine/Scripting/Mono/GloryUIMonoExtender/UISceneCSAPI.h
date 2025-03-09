#pragma once
#include <vector>

namespace Glory
{
	class InternalCall;
	class Engine;

	class UISceneCSAPI
	{
	public:
		static void AddInternalCalls(std::vector<InternalCall>& internalCalls);
		static void SetEngine(Engine* pEngine);

	public:
		UISceneCSAPI() = delete;
	};
}