#pragma once
#include <string_view>
#include <map>
#include <vector>

namespace Glory
{
	class InputModule;
	struct InputMap;

	struct PlayerInputData
	{
		InputMap* m_InputMap;
		std::map<std::string, char[4]> m_InputData;
	};

	class PlayerInput
	{
	public:
		PlayerInput(InputModule* pInputModule, size_t playerIndex);
		virtual ~PlayerInput();

		std::string_view InputMode();

	private:
		InputModule* m_pInputModule;
		size_t m_PlayerIndex;
		std::string_view m_InputMode;
		std::vector<PlayerInputData> m_InputData;
	};
}
