#pragma once
#include "ResourceLoaderModule.h"
#include "ImageData.h"

namespace Glory
{
	class ImageLoaderModule : public ResourceLoaderModule<ImageData, ImportSettings>
	{
	public:
		ImageLoaderModule();
		virtual ~ImageLoaderModule();

		virtual const std::type_info& GetModuleType() override;

	protected:
		virtual ImageData* LoadTexture(const std::string& path) = 0;

	private:
		virtual ImageData* LoadResource(const std::string& path, const ImportSettings&) override;

	protected:
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
		virtual void Update() override {};
		virtual void Draw() override {};
	};
}
