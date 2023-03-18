#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <vector>
#include <Glory.h>
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
		Vec3Wrapper(const glm::vec3& v);
		Vec3Wrapper(float _x, float _y, float _z);

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

	struct QuatWrapper
	{
		QuatWrapper(const glm::quat& q);
		QuatWrapper(float _x, float _y, float _z, float _w);

		float x;
		float y;
		float z;
		float w;
	};

	struct Mat4Wrapper
	{
		Vec4Wrapper Vec1;
		Vec4Wrapper Vec2;
		Vec4Wrapper Vec3;
		Vec4Wrapper Vec4;

		Mat4Wrapper(const Vec4Wrapper& v1, const Vec4Wrapper& v2, const Vec4Wrapper& v3, const Vec4Wrapper& v4)
		{
			Vec1 = v1;
			Vec2 = v2;
			Vec3 = v3;
			Vec4 = v4;
		}
	};

	GLORY_API glm::vec2 ToGLMVec2(const Vec2Wrapper& v);
	GLORY_API Vec2Wrapper ToVec2Wrapper(const glm::vec2& v);
	GLORY_API glm::vec3 ToGLMVec3(const Vec3Wrapper& v);
	GLORY_API Vec3Wrapper ToVec3Wrapper(const glm::vec3& v);
	GLORY_API glm::vec4 ToGLMVec4(const Vec4Wrapper& v);
	GLORY_API Vec4Wrapper ToVec4Wrapper(const glm::vec4& v);
	GLORY_API glm::quat ToGLMQuat(const QuatWrapper& q);
	GLORY_API QuatWrapper ToQuatWrapper(const glm::quat& q);
	GLORY_API glm::mat4 ToGLMMat4(const Mat4Wrapper& m);
	GLORY_API Mat4Wrapper ToMat4Wrapper(const glm::mat4& m);

	class MathCSAPI
	{
	public:
		static void AddInternalCalls(std::vector<InternalCall>& internalCalls);
	};
}
