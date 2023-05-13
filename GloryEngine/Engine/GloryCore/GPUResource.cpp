#include "GPUResource.h"

namespace Glory
{
	GPUResource::GPUResource() : m_UUID(UUID()), m_IsDirty(false)
	{
	}

	GPUResource::~GPUResource()
	{
	}

	void GPUResource::SetDirty(bool dirty)
	{
		m_IsDirty = dirty;
	}

	bool GPUResource::IsDirty() const
	{
		return m_IsDirty;
	}
}