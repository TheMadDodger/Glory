#pragma once
#pragma comment (lib,"mono-2.0-sgen.lib")
#pragma comment (lib,"MonoPosixHelper.lib")

#include <Glory.h>
#include <Module.h>
#include <GloryContext.h>
#include <ScriptingModule.h>
#include <ScriptExtensions.h>
#include <IScriptExtender.h>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/debug-helpers.h>

extern "C" GLORY_API Glory::Module * OnLoadModule(Glory::GloryContext * pContext);

//namespace Glory
//{
//	class GloryMono
//	{
//	public:
//		static void Initialize();
//		static void Cleanup();
//
//	private:
//		static MonoDomain* m_pMonoDomain;
//		static MonoAssembly* m_pMonoAssembly;
//		static MonoImage* m_pMonoImage;
//		static MonoObject* m_pMonoObject;
//		static uint32_t m_MonoObjectGCHandle;
//	};
//
//	class RandomWord
//	{
//	public:
//		static MonoString* GetWord();
//	};
//}

