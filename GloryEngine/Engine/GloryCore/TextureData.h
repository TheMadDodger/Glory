#pragma once
#include "ImageData.h"

namespace Glory
{
	class TextureData : public Resource
	{
    public:
        TextureData();
        virtual ~TextureData();

    private:
        friend class TextureDataLoaderModule;
        friend class ImageLoaderModule;
        ImageData* m_pImage;
	};
}
