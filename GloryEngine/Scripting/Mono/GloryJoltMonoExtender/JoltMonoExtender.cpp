#include "JoltMonoExtender.h"
#include "PhysicsCSAPI.h"
#include "PhysicsComponentsCSAPI.h"

#include <ScriptingExtender.h>
#include <Engine.h>

namespace Glory
{
	JoltMonoExtender::JoltMonoExtender() : m_pLibManager(new JoltLibManager())
	{
	}

	JoltMonoExtender::~JoltMonoExtender()
	{
		delete m_pLibManager;
		m_pLibManager = nullptr;
	}

	std::string JoltMonoExtender::Language()
	{
		return "csharp";
	}

	void JoltMonoExtender::GetInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		PhysicsCSAPI::AddInternalCalls(internalCalls);
		PhysicsComponentsCSAPI::AddInternalCalls(internalCalls);
	}

	void JoltMonoExtender::GetLibs(ScriptingExtender* pScriptingExtender)
	{
		pScriptingExtender->AddInternalLib("GloryEngine.Jolt.dll", m_pLibManager);
	}

	IScriptExtender* OnLoadExtension()
	{
		return new JoltMonoExtender();
	}

	void JoltLibManager::Initialize(Engine* pEngine, Assembly*)
	{
		PhysicsCSAPI::SetEngine(pEngine);
		PhysicsComponentsCSAPI::SetEngine(pEngine);
	}

	void JoltLibManager::Cleanup()
	{
		
	}
}
