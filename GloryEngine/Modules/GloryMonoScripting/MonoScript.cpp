#include "MonoScript.h"
#include "ScriptingMethodsHelper.h"
#include "MonoManager.h"
#include "Assembly.h"
#include "AssemblyDomain.h"
#include "CoreLibManager.h"
#include "GloryMonoScipting.h"

#include <Engine.h>
#include <GScene.h>
#include <Serializers.h>
#include <SceneManager.h>
#include <BinaryStream.h>
#include <AssetDatabase.h>
#include <Reflection.h>
#include <PropertySerializer.h>
#include <AssetReference.h>

#include <mono/jit/jit.h>

namespace Glory
{
	MonoScript::MonoScript()
	{
		APPEND_TYPE(MonoScript);
	}

	MonoScript::MonoScript(FileData* pFileData, std::vector<std::string>&& classes)
		: FileData(pFileData), m_Classes(std::move(classes))
	{
		APPEND_TYPE(MonoScript);
	}
	
	MonoScript::MonoScript(std::vector<char>&& data)
		: FileData(std::move(data))
	{
		APPEND_TYPE(MonoScript);
	}

	MonoScript::~MonoScript()
	{
	}

	bool MonoScript::HasClass(std::string_view name) const
	{
		for (size_t i = 0; i < m_Classes.size(); ++i)
		{
			if (m_Classes[i] != name) continue;
			return true;
		}
		return false;
	}

	std::string_view MonoScript::ClassName(size_t index) const
	{
		if (index >= m_Classes.size()) return "";
		return m_Classes[index];
	}
}
