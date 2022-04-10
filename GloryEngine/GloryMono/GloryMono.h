#pragma once
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <Glory.h>


namespace Glory
{
	class GloryMono
	{
	public:
		void Initialize();
		void Cleanup();

	private:
		MonoDomain* m_pMonoDomain;
		MonoAssembly* m_pMonoAssembly;
		MonoImage* m_pMonoImage;
		MonoObject* m_pMonoObject;
		uint32_t m_MonoObjectGCHandle = 0;
	};

	class GLORY_EXPORT RandomWord
	{
	public:
		static MonoString* GetRandomWord();
	};
}

