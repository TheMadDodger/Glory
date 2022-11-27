#pragma once
#include "ScriptExtensions.h"
#include "GloryMono.h"
#include "MonoManager.h"
#include <Debug.h>

namespace Glory
{
	struct LayerWrapper
	{
	public:
		LayerWrapper(const Layer* pLayer) : Mask(pLayer ? pLayer->m_Mask : 0),
			Name(mono_string_new(MonoManager::GetDomain(), pLayer ? pLayer->m_Name.c_str() : ""))
		{ }

		LayerMask Mask;
		MonoString* Name;
	};

	class CoreCSAPI
	{
	public:
		static void AddInternalCalls(std::vector<InternalCall>& internalCalls);

	private:
		CoreCSAPI();
		virtual ~CoreCSAPI();
	};
}
