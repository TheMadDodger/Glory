#pragma once

namespace Glory
{
	class Assembly;
	class Engine;

	class IMonoLibManager
	{
	public:
		virtual void CollectTypes(Engine* pEngine, Assembly* pAssembly) = 0;
		virtual void Initialize(Engine* pEngine, Assembly* pAssembly) = 0;
		virtual void Cleanup(Engine* pEngine) = 0;
		virtual void Reset(Engine* pEngine) = 0;
	};
}
