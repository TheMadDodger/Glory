#pragma once
#include "Object.h"
#include "Debug.h"

namespace Glory
{
	class GameState : public Object
	{
	public:
		GameState();
		virtual ~GameState();

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
		friend class Game;

	private:
		/*GloryGameState(const GloryGameState& y) = delete;
		GloryGameState operator=(const GloryGameState& y) = delete;*/
	};
}
