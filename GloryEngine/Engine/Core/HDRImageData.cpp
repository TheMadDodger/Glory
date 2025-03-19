#include "HDRImageData.h"

namespace Glory
{
	HDRImageData::HDRImageData() : ImageData()
	{
		APPEND_TYPE(HDRImageData);
	}

	HDRImageData::HDRImageData(uint32_t w, uint32_t h, char*&& pPixels, size_t dataSize) :
		ImageData(w, h, PixelFormat::PF_R16G16B16Sfloat, PixelFormat::PF_RGB, 12, std::move(pPixels), dataSize, false, DataType::DT_Float)
	{
		APPEND_TYPE(HDRImageData);
	}

	HDRImageData::~HDRImageData()
	{
	}
}
