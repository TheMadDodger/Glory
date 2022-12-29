#include "MathCSAPI.h"
#include "GloryMonoScipting.h"

namespace Glory
{
#pragma region Converters

	glm::vec2 ToGLMVec2(Vec2Wrapper v)
	{
		return glm::vec2(v.x, v.y);
	}

	Vec2Wrapper ToVec2Wrapper(const glm::vec2& v)
	{
		return { v.x, v.y };
	}

	glm::vec3 ToGLMVec3(Vec3Wrapper v)
	{
		return glm::vec3(v.x, v.y, v.z);
	}

	Vec3Wrapper ToVec3Wrapper(const glm::vec3& v)
	{
		return { v.x, v.y, v.z };
	}

	glm::vec4 ToGLMVec4(Vec4Wrapper v)
	{
		return glm::vec4(v.x, v.y, v.z, v.w);
	}

	Vec4Wrapper ToVec4Wrapper(const glm::vec4& v)
	{
		return { v.x, v.y, v.z, v.w };
	}

#pragma endregion

#pragma region Vector2

	Vec2Wrapper Vector2AddOverload(Vec2Wrapper a, Vec2Wrapper b)
	{
		return ToVec2Wrapper(ToGLMVec2(a) + ToGLMVec2(b));
	}

	Vec2Wrapper Vector2SubtractOverload(Vec2Wrapper a, Vec2Wrapper b)
	{
		return ToVec2Wrapper(ToGLMVec2(a) - ToGLMVec2(b));
	}

	Vec2Wrapper Vector2MultiplyOverload(Vec2Wrapper a, Vec2Wrapper b)
	{
		return ToVec2Wrapper(ToGLMVec2(a) * ToGLMVec2(b));
	}

	Vec2Wrapper Vector2DivideOverload(Vec2Wrapper a, Vec2Wrapper b)
	{
		return ToVec2Wrapper(ToGLMVec2(a) / ToGLMVec2(b));
	}

#pragma endregion

#pragma region Vector3

	Vec3Wrapper Vector3AddOverload(Vec3Wrapper a, Vec3Wrapper b)
	{
		return ToVec3Wrapper(ToGLMVec3(a) + ToGLMVec3(b));
	}

	Vec3Wrapper Vector3SubtractOverload(Vec3Wrapper a, Vec3Wrapper b)
	{
		return ToVec3Wrapper(ToGLMVec3(a) - ToGLMVec3(b));
	}

	Vec3Wrapper Vector3MultiplyOverload(Vec3Wrapper a, Vec3Wrapper b)
	{
		return ToVec3Wrapper(ToGLMVec3(a) * ToGLMVec3(b));
	}

	Vec3Wrapper Vector3DivideOverload(Vec3Wrapper a, Vec3Wrapper b)
	{
		return ToVec3Wrapper(ToGLMVec3(a) / ToGLMVec3(b));
	}

#pragma endregion

#pragma region Vector4

	Vec4Wrapper Vector4AddOverload(Vec4Wrapper a, Vec4Wrapper b)
	{
		return ToVec4Wrapper(ToGLMVec4(a) + ToGLMVec4(b));
	}

	Vec4Wrapper Vector4SubtractOverload(Vec4Wrapper a, Vec4Wrapper b)
	{
		return ToVec4Wrapper(ToGLMVec4(a) - ToGLMVec4(b));
	}

	Vec4Wrapper Vector4MultiplyOverload(Vec4Wrapper a, Vec4Wrapper b)
	{
		return ToVec4Wrapper(ToGLMVec4(a) * ToGLMVec4(b));
	}

	Vec4Wrapper Vector4DivideOverload(Vec4Wrapper a, Vec4Wrapper b)
	{
		return ToVec4Wrapper(ToGLMVec4(a) / ToGLMVec4(b));
	}

#pragma endregion

#pragma region Binding

	void MathCSAPI::AddInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		BIND("GloryEngine.Vector2::op_Addition(GloryEngine.Vector2,GloryEngine.Vector2)", Vector2AddOverload);
		BIND("GloryEngine.Vector2::op_Subtraction(GloryEngine.Vector2,GloryEngine.Vector2)", Vector2SubtractOverload);
		BIND("GloryEngine.Vector2::op_Multiply(GloryEngine.Vector2,GloryEngine.Vector2)", Vector2MultiplyOverload);
		BIND("GloryEngine.Vector2::op_Division(GloryEngine.Vector2,GloryEngine.Vector2)", Vector2DivideOverload);

		BIND("GloryEngine.Vector3::op_Addition(GloryEngine.Vector3,GloryEngine.Vector3)", Vector3AddOverload);
		BIND("GloryEngine.Vector3::op_Subtraction(GloryEngine.Vector3,GloryEngine.Vector3)", Vector3SubtractOverload);
		BIND("GloryEngine.Vector3::op_Multiply(GloryEngine.Vector3,GloryEngine.Vector3)", Vector3MultiplyOverload);
		BIND("GloryEngine.Vector3::op_Division(GloryEngine.Vector3,GloryEngine.Vector3)", Vector3DivideOverload);

		BIND("GloryEngine.Vector4::op_Addition(GloryEngine.Vector4,GloryEngine.Vector4)", Vector4AddOverload);
		BIND("GloryEngine.Vector4::op_Subtraction(GloryEngine.Vector4,GloryEngine.Vector4)", Vector4SubtractOverload);
		BIND("GloryEngine.Vector4::op_Multiply(GloryEngine.Vector4,GloryEngine.Vector4)", Vector4MultiplyOverload);
		BIND("GloryEngine.Vector4::op_Division(GloryEngine.Vector4,GloryEngine.Vector4)", Vector4DivideOverload);
	}

#pragma endregion
}
