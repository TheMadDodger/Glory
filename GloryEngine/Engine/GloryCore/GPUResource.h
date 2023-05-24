#pragma once
#include "UUID.h"

namespace Glory
{
	class GPUResource
	{
	public:
		GPUResource();
		virtual ~GPUResource();

		void SetDirty(bool dirty);
		bool IsDirty() const;

	private:
		friend class GPUResourceManager;
		UUID m_UUID;
		bool m_IsDirty;
	};
}