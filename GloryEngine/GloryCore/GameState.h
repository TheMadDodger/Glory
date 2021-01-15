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
		virtual void OnUserStart()
		{
			Debug::Log("This is a test info log!", Debug::LogLevel::Info);
			Debug::Log("This is a test notice log!", Debug::LogLevel::Notice);
			Debug::Log("This is a test warning log!", Debug::LogLevel::Warning);
			Debug::Log("This is a test error log!", Debug::LogLevel::Error);
		};
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
