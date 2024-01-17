#pragma once
#include "UUID.h"

namespace Glory
{
	class GPUResourceManager;

	class GPUResource
	{
	public:
		GPUResource();
		virtual ~GPUResource();

		void SetDirty(bool dirty);
		bool IsDirty() const;

	protected:
		GPUResourceManager* m_pOwner = nullptr;

	private:
		friend class GPUResourceManager;
		UUID m_UUID;
		bool m_IsDirty;
	};
}