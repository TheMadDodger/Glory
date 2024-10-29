#include "MonoComponentObjectManager.h"

#include <mono/jit/jit.h>
#include <mono/metadata/debug-helpers.h>

namespace Glory
{
	MonoClass* MonoComponentObjectManager::m_pComponentManagerClass = nullptr;
	MonoMethod* MonoComponentObjectManager::m_pClearMethod = nullptr;

	void MonoComponentObjectManager::Initialize(MonoImage* pImage)
	{
		m_pComponentManagerClass = mono_class_from_name(pImage, "GloryEngine.Entities", "EntityComponentManager");
		mono_class_init(m_pComponentManagerClass);

		MonoMethodDesc* pMethodDesc = mono_method_desc_new(".::Clear()", false);
		m_pClearMethod = mono_method_desc_search_in_class(pMethodDesc, m_pComponentManagerClass);
		mono_method_desc_free(pMethodDesc);
	}

	void MonoComponentObjectManager::Cleanup()
	{
		MonoObject* pExcept;
		mono_runtime_invoke(m_pClearMethod, nullptr, nullptr, &pExcept);
	}

	MonoComponentObjectManager::MonoComponentObjectManager()
	{
	}

	MonoComponentObjectManager::~MonoComponentObjectManager()
	{
	}
}
