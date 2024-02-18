#include "GloryRuntime.h"
#include "RuntimeMaterialManager.h"
#include "RuntimeShaderManager.h"

#include <Engine.h>

namespace Glory
{
	GloryRuntime::GloryRuntime(Engine* pEngine): m_pEngine(pEngine),
		m_MaterialManager(new RuntimeMaterialManager(pEngine)), m_ShaderManager(new RuntimeShaderManager(pEngine))
	{
	}

	GloryRuntime::~GloryRuntime() = default;

	void GloryRuntime::Run()
	{
		m_pEngine->SetMaterialManager(m_MaterialManager.get());
		m_pEngine->SetShaderManager(m_ShaderManager.get());

		m_pEngine->Initialize();
		m_pEngine->StartThreads();

		while (!m_pEngine->WantsToQuit())
		{
			m_pEngine->Update();
		}
	}
}
