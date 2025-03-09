#include "MathCSAPI.h"
#include "GloryMonoScipting.h"

namespace Glory
{
#pragma region Converters

	glm::vec2 ToGLMVec2(const Vec2Wrapper& v)
	{
		return glm::vec2(v.x, v.y);
	}

	Vec2Wrapper ToVec2Wrapper(const glm::vec2& v)
	{
		return { v.x, v.y };
	}

	glm::vec3 ToGLMVec3(const Vec3Wrapper& v)
	{
		return glm::vec3(v.x, v.y, v.z);
	}

	Vec3Wrapper ToVec3Wrapper(const glm::vec3& v)
	{
		return { v.x, v.y, v.z };
	}

	glm::vec4 ToGLMVec4(const Vec4Wrapper& v)
	{
		return glm::vec4(v.x, v.y, v.z, v.w);
	}

	Vec4Wrapper ToVec4Wrapper(const glm::vec4& v)
	{
		return { v.x, v.y, v.z, v.w };
	}

	glm::quat ToGLMQuat(const QuatWrapper& q)
	{
		return glm::quat(q.w, q.x, q.y, q.z);
	}

	QuatWrapper ToQuatWrapper(const glm::quat& q)
	{
		return { q.x, q.y, q.z, q.w };
	}

	glm::mat4 ToGLMMat4(const Mat4Wrapper& m)
	{
		return glm::mat4(ToGLMVec4(m.Vec1), ToGLMVec4(m.Vec2), ToGLMVec4(m.Vec3), ToGLMVec4(m.Vec4));
	}

	Mat4Wrapper ToMat4Wrapper(const glm::mat4& m)
	{
		const Vec4Wrapper v1 = ToVec4Wrapper(m[0]);
		const Vec4Wrapper v2 = ToVec4Wrapper(m[1]);
		const Vec4Wrapper v3 = ToVec4Wrapper(m[2]);
		const Vec4Wrapper v4 = ToVec4Wrapper(m[3]);
		return Mat4Wrapper(v1, v2, v3, v4);
	}

	Vec2Wrapper::Vec2Wrapper() : x(0.0f), y(0.0f) {}
	Vec2Wrapper::Vec2Wrapper(const glm::vec2& v) : Vec2Wrapper(v.x, v.y) {}
	Vec2Wrapper::Vec2Wrapper(float _x, float _y) : x(_x), y(_y) {}

	Vec3Wrapper::Vec3Wrapper(const glm::vec3& v) : Vec3Wrapper(v.x, v.y, v.z) {}

	Vec3Wrapper::Vec3Wrapper(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

	QuatWrapper::QuatWrapper(const glm::quat& q) : QuatWrapper(q.x, q.y, q.z, q.w) {}

	QuatWrapper::QuatWrapper(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}

#pragma endregion

#pragma region Vector2

	Vec2Wrapper Vector2_GetNormalized(Vec2Wrapper a)
	{
		return ToVec2Wrapper(glm::normalize(ToGLMVec2(a)));
	}

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

	/* FIXME: It's probably way faster to calculate these on the CS rather than having the overhead of calling the c++ */

	Vec3Wrapper Vector3_GetNormalized(Vec3Wrapper a)
	{
		return glm::normalize(ToGLMVec3(a));
	}

	Vec3Wrapper Vector3AddOverload(Vec3Wrapper a, Vec3Wrapper b)
	{
		return ToGLMVec3(a) + ToGLMVec3(b);
	}

	Vec3Wrapper Vector3SubtractOverload(Vec3Wrapper a, Vec3Wrapper b)
	{
		return ToGLMVec3(a) - ToGLMVec3(b);
	}

	Vec3Wrapper Vector3MultiplyOverload(Vec3Wrapper a, Vec3Wrapper b)
	{
		return ToGLMVec3(a) * ToGLMVec3(b);
	}

	Vec3Wrapper Vector3DivideOverload(Vec3Wrapper a, Vec3Wrapper b)
	{
		return ToGLMVec3(a) / ToGLMVec3(b);
	}

#pragma endregion

#pragma region Vector4

	Vec4Wrapper Vector4_GetNormalized(Vec4Wrapper a)
	{
		return ToVec4Wrapper(glm::normalize(ToGLMVec4(a)));
	}

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

#pragma region Quaternion

	QuatWrapper Quaternion_Euler(float x, float y, float z)
	{
		return glm::quat(glm::vec3(x, y, z));
	}

	QuatWrapper Quaternion_AddOverload(QuatWrapper a, QuatWrapper b)
	{
		return ToGLMQuat(a) + ToGLMQuat(b);
	}

	QuatWrapper Quaternion_SubtractOverload(QuatWrapper a, QuatWrapper b)
	{
		return ToGLMQuat(a) - ToGLMQuat(b);
	}

	QuatWrapper Quaternion_MultiplyOverload(QuatWrapper a, QuatWrapper b)
	{
		return ToGLMQuat(a) * ToGLMQuat(b);
	}

	QuatWrapper Quaternion_MultiplyFloatOverload(QuatWrapper a, float b)
	{
		return ToGLMQuat(a) * b;
	}

#pragma endregion

#pragma region Mat4

	Mat4Wrapper Mat4_GetIdentity()
	{
		Mat4Wrapper m = ToMat4Wrapper(glm::identity<glm::mat4>());
		return m;
	}

	Mat4Wrapper Mat4_GetInverse(Mat4Wrapper m)
	{
		return ToMat4Wrapper(glm::inverse(ToGLMMat4(m)));
	}

	Mat4Wrapper Mat4_Translate(Vec3Wrapper v)
	{
		return ToMat4Wrapper(glm::translate(glm::identity<glm::mat4>(), ToGLMVec3(v)));
	}

	Mat4Wrapper Mat4_TranslateMat4(Mat4Wrapper m, Vec3Wrapper v)
	{
		return ToMat4Wrapper(glm::translate(ToGLMMat4(m), ToGLMVec3(v)));
	}

	Mat4Wrapper Mat4_Rotate(float angle, Vec3Wrapper axis)
	{
		return ToMat4Wrapper(glm::rotate(glm::identity<glm::mat4>(), angle, ToGLMVec3(axis)));
	}

	Mat4Wrapper Mat4_RotateMat4(Mat4Wrapper m, float angle, Vec3Wrapper axis)
	{
		return ToMat4Wrapper(glm::rotate(ToGLMMat4(m), angle, ToGLMVec3(axis)));
	}

	Mat4Wrapper Mat4_Scale(Vec3Wrapper v)
	{
		return ToMat4Wrapper(glm::scale(glm::identity<glm::mat4>(), ToGLMVec3(v)));
	}

	Mat4Wrapper Mat4_ScaleMat4(Mat4Wrapper m, Vec3Wrapper v)
	{
		return ToMat4Wrapper(glm::scale(ToGLMMat4(m), ToGLMVec3(v)));
	}

	Mat4Wrapper Mat4_AddOverload(Mat4Wrapper a, Mat4Wrapper b)
	{
		return ToMat4Wrapper(ToGLMMat4(a) + ToGLMMat4(b));
	}

	Mat4Wrapper Mat4_SubtractOverload(Mat4Wrapper a, Mat4Wrapper b)
	{
		return ToMat4Wrapper(ToGLMMat4(a) - ToGLMMat4(b));
	}

	Mat4Wrapper Mat4_MultiplyOverload(Mat4Wrapper a, Mat4Wrapper b)
	{
		return ToMat4Wrapper(ToGLMMat4(a) * ToGLMMat4(b));
	}

	Mat4Wrapper Mat4_DivideOverload(Mat4Wrapper a, Mat4Wrapper b)
	{
		return ToMat4Wrapper(ToGLMMat4(a) / ToGLMMat4(b));
	}

	Vec4Wrapper Mat4_MultiplyVector4Overload(Mat4Wrapper a, Vec4Wrapper b)
	{
		return ToVec4Wrapper(ToGLMMat4(a) * ToGLMVec4(b));
	}

#pragma endregion

#pragma region Math

	Mat4Wrapper Math_LookAt(Vec3Wrapper eye, Vec3Wrapper center, Vec3Wrapper up)
	{
		return ToMat4Wrapper(glm::lookAt(ToGLMVec3(eye), ToGLMVec3(center), ToGLMVec3(up)));
	}

#pragma endregion

#pragma region Binding

	void MathCSAPI::AddInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		/* Vector2 */
		BIND("GloryEngine.Vector2::Vector2_GetNormalized", Vector2_GetNormalized);
		BIND("GloryEngine.Vector2::op_Addition(GloryEngine.Vector2,GloryEngine.Vector2)", Vector2AddOverload);
		BIND("GloryEngine.Vector2::op_Subtraction(GloryEngine.Vector2,GloryEngine.Vector2)", Vector2SubtractOverload);
		BIND("GloryEngine.Vector2::op_Multiply(GloryEngine.Vector2,GloryEngine.Vector2)", Vector2MultiplyOverload);
		BIND("GloryEngine.Vector2::op_Division(GloryEngine.Vector2,GloryEngine.Vector2)", Vector2DivideOverload);

		/* Vector3 */
		BIND("GloryEngine.Vector3::Vector3_GetNormalized", Vector3_GetNormalized);
		BIND("GloryEngine.Vector3::op_Addition(GloryEngine.Vector3,GloryEngine.Vector3)", Vector3AddOverload);
		BIND("GloryEngine.Vector3::op_Subtraction(GloryEngine.Vector3,GloryEngine.Vector3)", Vector3SubtractOverload);
		BIND("GloryEngine.Vector3::op_Multiply(GloryEngine.Vector3,GloryEngine.Vector3)", Vector3MultiplyOverload);
		BIND("GloryEngine.Vector3::op_Division(GloryEngine.Vector3,GloryEngine.Vector3)", Vector3DivideOverload);

		/* Vector4 */
		BIND("GloryEngine.Vector4::Vector4_GetNormalized", Vector4_GetNormalized);
		BIND("GloryEngine.Vector4::op_Addition(GloryEngine.Vector4,GloryEngine.Vector4)", Vector4AddOverload);
		BIND("GloryEngine.Vector4::op_Subtraction(GloryEngine.Vector4,GloryEngine.Vector4)", Vector4SubtractOverload);
		BIND("GloryEngine.Vector4::op_Multiply(GloryEngine.Vector4,GloryEngine.Vector4)", Vector4MultiplyOverload);
		BIND("GloryEngine.Vector4::op_Division(GloryEngine.Vector4,GloryEngine.Vector4)", Vector4DivideOverload);

		/* Quaternion */
		BIND("GloryEngine.Quaternion::Quaternion_Euler", Quaternion_Euler);
		BIND("GloryEngine.Quaternion::op_Addition(GloryEngine.Quaternion,GloryEngine.Quaternion)", Quaternion_AddOverload);
		BIND("GloryEngine.Quaternion::op_Subtraction(GloryEngine.Quaternion,GloryEngine.Quaternion)", Quaternion_SubtractOverload);
		BIND("GloryEngine.Quaternion::op_Multiply(GloryEngine.Quaternion,GloryEngine.Quaternion)", Quaternion_MultiplyOverload);
		BIND("GloryEngine.Quaternion::op_Multiply(GloryEngine.Quaternion,float)", Quaternion_MultiplyFloatOverload);

		/* Mat4 */
		BIND("GloryEngine.Mat4::Mat4_GetIdentity", Mat4_GetIdentity);
		BIND("GloryEngine.Mat4::Mat4_GetInverse", Mat4_GetInverse);
		BIND("GloryEngine.Mat4::Mat4_Translate", Mat4_Translate);
		BIND("GloryEngine.Mat4::Mat4_TranslateMat4", Mat4_TranslateMat4);
		BIND("GloryEngine.Mat4::Mat4_Rotate", Mat4_Rotate);
		BIND("GloryEngine.Mat4::Mat4_RotateMat4", Mat4_RotateMat4);
		BIND("GloryEngine.Mat4::Mat4_Translate", Mat4_Scale);
		BIND("GloryEngine.Mat4::Mat4_Translate", Mat4_ScaleMat4);
		BIND("GloryEngine.Mat4::op_Addition(GloryEngine.Mat4,GloryEngine.Mat4)", Mat4_AddOverload);
		BIND("GloryEngine.Mat4::op_Subtraction(GloryEngine.Mat4,GloryEngine.Mat4)", Mat4_SubtractOverload);
		BIND("GloryEngine.Mat4::op_Multiply(GloryEngine.Mat4,GloryEngine.Mat4)", Mat4_MultiplyOverload);
		BIND("GloryEngine.Mat4::op_Division(GloryEngine.Mat4,GloryEngine.Mat4)", Mat4_DivideOverload);
		BIND("GloryEngine.Mat4::op_Multiply(GloryEngine.Mat4,GloryEngine.Vector4)", Mat4_MultiplyVector4Overload);

		/* Math */
		BIND("GloryEngine.Math::Math_LookAt", Math_LookAt);
	}

#pragma endregion
}
