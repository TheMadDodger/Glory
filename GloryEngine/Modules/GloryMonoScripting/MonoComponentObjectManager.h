#pragma once
#include <mono/metadata/object-forward.h>

namespace Glory
{
	class MonoComponentObjectManager
	{
	public:
		static void Initialize(MonoImage* pImage);
		static void Cleanup();

	private:
		MonoComponentObjectManager();
		virtual ~MonoComponentObjectManager();

	private:
		static MonoClass* m_pComponentManagerClass;
		static MonoMethod* m_pClearMethod;
	};
}
