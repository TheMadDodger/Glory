#pragma once
#include "MonoEntityObjectManager.h"

#include <Glory.h>
#include <GloryMono.h>
#include <AssemblyBinding.h>

namespace Glory
{
	class MonoEntitySceneManager
	{
	public:
		static GLORY_API MonoObject* GetSceneObject(GScene* pScene);
		static GLORY_API MonoSceneObjectManager* GetSceneObjectManager(GScene* pScene);

	private:
		static void Initialize(AssemblyBinding* pAssembly);
		static void Cleanup();

	private:
		friend class EntityLibManager;
		/* No instances allowed */
		MonoEntitySceneManager() = delete;
	};
}
