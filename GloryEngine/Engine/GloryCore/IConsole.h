#pragma once
#include <string>
#include <glm/glm.hpp>

namespace Glory
{
	class IConsole
	{
	public:
		virtual void Initialize() = 0;
		virtual void OnConsoleClose() = 0;
		virtual void SetNextColor(const glm::vec4& color) = 0;
		virtual void ResetNextColor() = 0;
		virtual void Write(const std::string& line) = 0;
	};
}
