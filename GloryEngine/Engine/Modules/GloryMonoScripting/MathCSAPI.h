#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "ScriptExtensions.h"

namespace Glory
{
	struct Vec2Wrapper
	{
		float x;
		float y;
	};

	struct Vec3Wrapper
	{
		float x;
		float y;
		float z;
	};

	struct Vec4Wrapper
	{
		float x;
		float y;
		float z;
		float w;
	};

	class MathCSAPI
	{
	public:
		static void AddInternalCalls(std::vector<InternalCall>& internalCalls);
	};
}
