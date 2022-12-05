#pragma once
#include <string>
#include <ScriptingExtender.h>
#include "GloryMono.h"

namespace Glory
{
	class MonoManager
	{
	public:
		static GLORY_API MonoDomain* GetDomain();
		static GLORY_API void LoadLib(const ScriptingLib& lib);

		static GLORY_API void Reload();

	private:
		static void Initialize(const std::string& assemblyDir = ".", const std::string& configDir = "");
		static void Cleanup();

	private:
		MonoManager();
		virtual ~MonoManager();

	private:
		friend class GloryMonoScipting;
		static MonoDomain* m_pMainDomain;
		static MonoDomain* m_pDomain;
	};
}
