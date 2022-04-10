#pragma once
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

namespace Glory
{
	class GloryMono
	{
	public:
		static void Initialize();
		static void Cleanup();

	private:
		static MonoDomain* m_pMonoDomain;
		static MonoAssembly* m_pMonoAssembly;
		static MonoImage* m_pMonoImage;
		static MonoObject* m_pMonoObject;
		static uint32_t m_MonoObjectGCHandle;
	};

	class RandomWord
	{
	public:
		static MonoString* GetWord();
	};
}

