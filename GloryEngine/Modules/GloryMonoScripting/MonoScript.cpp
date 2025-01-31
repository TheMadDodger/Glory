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

	MonoScript::MonoScript(FileData* pFileData, std::string_view ns, std::string_view className)
		: FileData(pFileData), m_NamespaceName(ns), m_ClassName(className)
	{
		APPEND_TYPE(MonoScript);
	}
	
	MonoScript::MonoScript(std::vector<char>&& data)
		: FileData(std::move(data)), m_NamespaceName(""), m_ClassName("")
	{
		APPEND_TYPE(MonoScript);
	}

	MonoScript::~MonoScript()
	{
	}

	std::string MonoScript::FullName() const
	{
		std::stringstream str;
		str << m_NamespaceName << "." << m_Name;
		return str.str();
	}
}
