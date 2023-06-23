#pragma once

namespace Glory
{
	class Assembly;

	class IMonoLibManager
	{
	public:
		virtual void Initialize(Assembly* pAssembly) = 0;
		virtual void Cleanup() = 0;
	};
}
