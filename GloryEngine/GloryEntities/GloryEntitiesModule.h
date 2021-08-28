#pragma once
#include <Module.h>

namespace Glory
{
	class GloryEntitiesModule : Module
	{
	public:
		GloryEntitiesModule();
		virtual ~GloryEntitiesModule();

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;
		virtual void Update() override;
		virtual void Draw() override;

	private:
	};
}
