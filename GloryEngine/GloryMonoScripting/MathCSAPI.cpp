#include "MathCSAPI.h"
#include "GloryMonoScipting.h"

namespace Glory
{
#pragma region Vector2
	struct Vec2Wrapper
	{
		float x;
		float y;
	};

	glm::vec2 ToGLMVec2(Vec2Wrapper v)
	{
		return glm::vec2(v.x, v.y);
	}

	Vec2Wrapper ToVec2Wrapper(const glm::vec2& v)
	{
		return { v.x, v.y };
	}

	Vec2Wrapper VectorAddOverload(Vec2Wrapper a, Vec2Wrapper b)
	{
		return ToVec2Wrapper(ToGLMVec2(a) + ToGLMVec2(b));
	}

	Vec2Wrapper VectorSubtractOverload(Vec2Wrapper a, Vec2Wrapper b)
	{
		return ToVec2Wrapper(ToGLMVec2(a) - ToGLMVec2(b));
	}

	Vec2Wrapper VectorMultiplyOverload(Vec2Wrapper a, Vec2Wrapper b)
	{
		return ToVec2Wrapper(ToGLMVec2(a) * ToGLMVec2(b));
	}

	Vec2Wrapper VectorDivideOverload(Vec2Wrapper a, Vec2Wrapper b)
	{
		return ToVec2Wrapper(ToGLMVec2(a) / ToGLMVec2(b));
	}
#pragma endregion

	void MathCSAPI::AddInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		BIND("GloryEngine.Vector2::op_Addition(GloryEngine.Vector2,GloryEngine.Vector2)", VectorAddOverload);
		BIND("GloryEngine.Vector2::op_Subtraction(GloryEngine.Vector2,GloryEngine.Vector2)", VectorSubtractOverload);
		BIND("GloryEngine.Vector2::op_Multiply(GloryEngine.Vector2,GloryEngine.Vector2)", VectorMultiplyOverload);
		BIND("GloryEngine.Vector2::op_Division(GloryEngine.Vector2,GloryEngine.Vector2)", VectorDivideOverload);
	}
}
