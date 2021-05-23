#pragma once
#include "Object.h"
#include <typeinfo>

namespace Glory
{
	class Module : public Object
	{
	public:
		Module();
		virtual ~Module();

		virtual const std::type_info& GetModuleType() = 0;

	protected:
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
		virtual void Update() = 0;
		virtual void Draw() = 0;

		virtual bool HasPriority();

	private:
		friend class Engine;
	};
}