#pragma once
#include "ScriptProperty.h"
#include "MonoScript.h"
#include "AssetReference.h"

#include <Reflection.h>
#include <BinaryBuffer.h>
#include <yaml-cpp/node/node.h>

namespace Glory
{
	class MonoScript;

	struct ScriptTypeReference
	{
		ScriptTypeReference(): m_Hash(0) {}
		ScriptTypeReference(uint32_t hash): m_Hash(hash) {}

		bool operator==(const ScriptTypeReference& other) const { return m_Hash == other.m_Hash; }

		REFLECTABLE(ScriptTypeReference, (uint32_t)(m_Hash));
	};

	struct MonoScriptComponent
	{
		MonoScriptComponent() : m_ScriptType(0), m_ScriptData(), m_CachedComponentID(0), m_ScriptObjectHandle(0) {}
		MonoScriptComponent(uint32_t hash) : m_ScriptType(hash),
			m_ScriptData(), m_CachedComponentID(0), m_ScriptObjectHandle(0) {}

		REFLECTABLE(MonoScriptComponent,
			(BasicBuffer<char>) (m_ScriptData),
			(ScriptTypeReference) (m_ScriptType)
		);

		UUID m_CachedComponentID;
		uint32_t m_ScriptObjectHandle;
	};
}
