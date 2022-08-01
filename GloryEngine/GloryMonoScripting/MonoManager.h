#pragma once
#include <mono/jit/jit.h>
#include <string>
#include <ScriptingExtender.h>

namespace Glory
{
	class MonoManager
	{
	public:
		static MonoDomain* GetDomain();
		static void LoadLib(const ScriptingLib& lib);

	private:
		static void Initialize(const std::string& assemblyDir = ".", const std::string& configDir = "");
		static void Cleanup();

	private:
		MonoManager();
		virtual ~MonoManager();

	private:
		friend class GloryMonoScipting;
		static MonoDomain* m_pDomain;
	};
}