#pragma once
#include "Glory.h"
#include "IConsole.h"

namespace Glory
{
	class Console;

	class Logs : public IConsole
	{
	private:
		Logs(Console*) {};

		GLORY_API void Initialize() override;
		void OnConsoleClose() override {};
		void SetNextColor(const glm::vec4& color) override {};
		void ResetNextColor() override {};
		GLORY_API void Write(const std::string& line) override;

	private:
		friend class Console;
	};
}
