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

	struct MonoScriptComponent
	{
		MonoScriptComponent() : m_Script(0), m_ScriptData(), m_CachedComponentID(0), m_pScriptObject(nullptr) {}
		MonoScriptComponent(MonoScript* pScript) : m_Script(pScript != nullptr ? pScript->GetUUID() : 0),
			m_ScriptData(), m_CachedComponentID(0), m_pScriptObject(nullptr) {}

		REFLECTABLE(MonoScriptComponent,
			(BasicBuffer<char>) (m_ScriptData),
			(AssetReference<MonoScript>) (m_Script)
		);

		std::vector<ScriptProperty> m_ScriptProperties;
		UUID m_CachedComponentID;
		MonoObject* m_pScriptObject;
	};
}
