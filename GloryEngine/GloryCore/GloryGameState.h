#pragma once
#include "Object.h"

namespace Glory
{
	class GloryGameState : public Object
	{
	public:
		GloryGameState();
		virtual ~GloryGameState();

	protected:
		virtual void OnUserCreate() {};
		virtual void OnUserStart() {};
		virtual void OnUserBeginStep() {};
		virtual void OnUserEndStep() {};
		virtual void OnUserBeginDraw() {};
		virtual void OnUserEndDraw() {};
		virtual void OnUserEnd() {};
		virtual void OnUserDestroy() {};

	private:
		friend class GloryGame;

	private:
		/*GloryGameState(const GloryGameState& y) = delete;
		GloryGameState operator=(const GloryGameState& y) = delete;*/
	};
}
