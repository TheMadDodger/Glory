#pragma once

namespace Glory
{
	class GloryCore
	{
	public:
		static void Initialize();
		static void Destroy();

	private:
		GloryCore();
		~GloryCore();
	};
}