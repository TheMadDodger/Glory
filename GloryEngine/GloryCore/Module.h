#pragma once
#include "Object.h"

namespace Glory
{
	class Module : public Object
	{
	public:
		Module();
		virtual ~Module();

	protected:
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
		virtual void Update() = 0;
		virtual void Draw() = 0;

	private:
		friend class Engine;
	};
}