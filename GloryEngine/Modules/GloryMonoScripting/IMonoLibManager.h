#pragma once

namespace Glory
{
	class Assembly;
	class Engine;

	class IMonoLibManager
	{
	public:
		virtual void Initialize(Engine* pEngine, Assembly* pAssembly) = 0;
		virtual void Cleanup(Engine* pEngine) = 0;
	};
}
