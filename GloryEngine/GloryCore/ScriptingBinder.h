#pragma once
#include "Engine.h"

namespace Glory
{
	class ScriptingBinder
	{
	public:
		static void Initialize(Engine* pEngine);

	private:
		ScriptingBinder();
		virtual ~ScriptingBinder();
	};
}
