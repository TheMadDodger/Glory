#pragma once
#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace YAML
{
	Emitter& operator<<(Emitter& out, const glm::vec2& v);
	Emitter& operator<<(Emitter& out, const glm::vec3& v);
	Emitter& operator<<(Emitter& out, const glm::vec4& v);
	Emitter& operator<<(Emitter& out, const glm::quat& q);

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& v)
		{
			Node node;
			node.push_back(v.x);
			node.push_back(v.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& v)
		{
			if (!node.IsSequence() || node.size() < 2)
				return false;

			v.x = node[0].as<float>();
			v.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& v)
		{
			Node node;
			node.push_back(v.x);
			node.push_back(v.y);
			node.push_back(v.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& v)
		{
			if (!node.IsSequence() || node.size() < 3)
				return false;

			v.x = node[0].as<float>();
			v.y = node[1].as<float>();
			v.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& v)
		{
			Node node;
			node.push_back(v.x);
			node.push_back(v.y);
			node.push_back(v.z);
			node.push_back(v.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& v)
		{
			if (!node.IsSequence() || node.size() < 4)
				return false;

			v.x = node[0].as<float>();
			v.y = node[1].as<float>();
			v.z = node[2].as<float>();
			v.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::quat>
	{
		static Node encode(const glm::quat& q)
		{
			Node node;
			node.push_back(q.x);
			node.push_back(q.y);
			node.push_back(q.z);
			node.push_back(q.w);
			return node;
		}

		static bool decode(const Node& node, glm::quat& q)
		{
			if (!node.IsSequence() || node.size() < 4)
				return false;

			q.x = node[0].as<float>();
			q.y = node[1].as<float>();
			q.z = node[2].as<float>();
			q.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::ivec2>
	{
		static Node encode(const glm::ivec2& v)
		{
			Node node;
			node.push_back(v.x);
			node.push_back(v.y);
			return node;
		}

		static bool decode(const Node& node, glm::ivec2& v)
		{
			if (!node.IsSequence() || node.size() < 2)
				return false;

			v.x = node[0].as<int>();
			v.y = node[1].as<int>();
			return true;
		}
	};

	template<>
	struct convert<glm::ivec3>
	{
		static Node encode(const glm::ivec3& v)
		{
			Node node;
			node.push_back(v.x);
			node.push_back(v.y);
			node.push_back(v.z);
			return node;
		}

		static bool decode(const Node& node, glm::ivec3& v)
		{
			if (!node.IsSequence() || node.size() < 3)
				return false;

			v.x = node[0].as<int>();
			v.y = node[1].as<int>();
			v.z = node[2].as<int>();
			return true;
		}
	};

	template<>
	struct convert<glm::ivec4>
	{
		static Node encode(const glm::ivec4& v)
		{
			Node node;
			node.push_back(v.x);
			node.push_back(v.y);
			node.push_back(v.z);
			node.push_back(v.w);
			return node;
		}

		static bool decode(const Node& node, glm::ivec4& v)
		{
			if (!node.IsSequence() || node.size() < 4)
				return false;

			v.x = node[0].as<int>();
			v.y = node[1].as<int>();
			v.z = node[2].as<int>();
			v.w = node[3].as<int>();
			return true;
		}
	};
}