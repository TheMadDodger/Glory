#pragma once
#include "Object.h"
#include "ScriptBinding.h"
#include <typeinfo>

namespace Glory
{
	class Engine;

	class Module : public Object
	{
	public:
		Module();
		virtual ~Module();

		virtual const std::type_info& GetModuleType() = 0;

		Engine* GetEngine();

	protected:
		virtual void Initialize() = 0;
		virtual void PostInitialize() {};
		virtual void Cleanup() = 0;
		
		virtual void OnBeginFrame() {};
		virtual void OnEndFrame() {};

		virtual void OnGameThreadFrameStart() {};
		virtual void OnGameThreadFrameEnd() {};
		virtual void OnGraphicsThreadFrameStart() {};
		virtual void OnGraphicsThreadFrameEnd() {};

		virtual void Update() {};
		virtual void Draw() {};

		virtual bool HasPriority();

		virtual void GetScriptBindings(std::vector<ScriptBinding>& bindings) {};

	protected:
		Engine* m_pEngine;

	private:
		friend class Engine;
		friend class ScriptingBinder;
	};
}