#pragma once
#include "ScriptExtensions.h"
#include "GloryMono.h"
#include "MonoManager.h"
#include <Debug.h>

namespace Glory
{
	struct Layer;

	struct LayerWrapper
	{
	public:
		GLORY_API LayerWrapper(const Layer* pLayer);

		LayerMask Mask;
		MonoString* Name;
	};

	class CoreCSAPI
	{
	public:
		static void AddInternalCalls(std::vector<InternalCall>& internalCalls);
		static void SetEngine(Engine* pEngine);

	private:
		CoreCSAPI();
		virtual ~CoreCSAPI();
	};
}
