#pragma once

namespace Glory
{
	class Assembly;
	class IEngine;

	class IMonoLibManager
	{
	public:
		virtual void CollectTypes(IEngine* pEngine, Assembly* pAssembly) = 0;
		virtual void Initialize(IEngine* pEngine, Assembly* pAssembly) = 0;
		virtual void Cleanup(IEngine* pEngine) = 0;
		virtual void Reset(IEngine* pEngine) = 0;
	};
}
