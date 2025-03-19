#pragma once
#include "ImageData.h"

namespace Glory
{
	class HDRImageData : public ImageData
	{
	public:
		HDRImageData();
		HDRImageData(uint32_t w, uint32_t h, char*&& pPixels, size_t dataSize);
		virtual ~HDRImageData();
	};
}
