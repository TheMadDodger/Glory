#include "BasisImporter.h"

#include <basisu/basisu_enc.h>
#include <basisu/basisu_comp.h>
#include <basisu/basisu_uastc_enc.h>
#include <basisu/basisu_transcoder.h>
#include <sstream>
#include <Debug.h>
#include <TextureData.h>

namespace Glory::Editor
{
	constexpr size_t NumSupportedExtensions = 9;
	constexpr std::string_view SupportedExtensions[NumSupportedExtensions] = {
		".jpg",
		".JPEG",
		".JPG",
		".png",
		".PNG",
		".tif",
		".TIF",
		".webp",
		".WEBP",
	};

	BasisImporter::BasisImporter() : m_InitializedFlags(0)
	{
	}

	BasisImporter::~BasisImporter()
	{
	}

	std::string_view BasisImporter::Name() const
	{
		return "Basis Importer";
	}

	bool BasisImporter::SupportsExtension(const std::filesystem::path& extension) const
	{
		for (size_t i = 0; i < NumSupportedExtensions; ++i)
		{
			if (extension.compare(SupportedExtensions[i]) != 0) continue;
			return true;
		}
		return false;
	}

	ImageData* BasisImporter::LoadResource(const std::filesystem::path& path) const
	{
		basisu::image img;
		if (!basisu::load_image(path.string().c_str(), img))
		{
			std::stringstream str;
			str << "BasisImporter::LoadResource > Failed to load image " << path;
			Debug::LogError(str.str());
			return nullptr;
		}

		basisu::basis_compressor_params basisCompressorParams;
		basisCompressorParams.m_tex_type = basist::basis_texture_type::cBASISTexType2D;
		basisCompressorParams.m_source_images.push_back(img);
		basisCompressorParams.m_perceptual = false;
		basisCompressorParams.m_mip_srgb = false;
		basisCompressorParams.m_quality_level = 128;

		basisCompressorParams.m_write_output_basis_files = false;

		basisCompressorParams.m_uastc = true;
		//basisCompressorParams.m_max_endpoint_clusters = 512;
		//basisCompressorParams.m_max_selector_clusters = 512;
		basisCompressorParams.m_status_output = false;
		basisCompressorParams.m_compute_stats = false;

		basisu::job_pool jpool(1);
		basisCompressorParams.m_pJob_pool = &jpool;

		basisu::basis_compressor basisCompressor;
		if (!basisCompressor.init(basisCompressorParams))
		{
			std::stringstream str;
			str << "BasisImporter::LoadResource > Failed to initialize basis compressor for image " << path;
			Debug::LogError(str.str());
			return nullptr;
		}

		basisu::basis_compressor::error_code result = basisCompressor.process();
		if (result != basisu::basis_compressor::cECSuccess)
		{
			std::stringstream str;
			str << "BasisImporter::LoadResource > Failed to compress image " << path;
			Debug::LogError(str.str());
			return nullptr;
		}

		std::stringstream str;
		str << "BasisImporter::LoadResource > Succesfully compressed image " << path;
		Debug::LogInfo(str.str());

		const size_t size = basisCompressor.get_output_basis_file().size_in_bytes();
		char* pixels = new char[size];
		std::memcpy(pixels, basisCompressor.get_output_basis_file().data(), size);

		const uint32_t width = static_cast<uint32_t>(img.get_width());
		const uint32_t height = static_cast<uint32_t>(img.get_height());

		const size_t numPixels = width * height;

		PixelFormat pixelFormat = Glory::PixelFormat::PF_Astc4x4SrgbBlock;
		PixelFormat internalFormat = Glory::PixelFormat::PF_RGBA;
		uint8_t bytesPerPixel = 4;

		ImageData* pData = new ImageData(width, height, internalFormat, pixelFormat, bytesPerPixel, std::move(pixels), size, true);
		TextureData* pDefualtTexture = new TextureData(pData);
		pData->AddSubresource(pDefualtTexture, "Default");

		return pData;

		//const basisu::uint8_vec& basisFileData = basisCompressor.get_output_basis_file();

		//basist::basisu_transcoder transcoder;
		//basist::basisu_file_info fileinfo;
		//if (!transcoder.get_file_info(&basisFileData[0], (uint32_t)basisFileData.size(), fileinfo))
		//{
		//	Debug::LogError("Failed retrieving Basis file information!");
		//	return nullptr;
		//}

		//basist::basisu_image_info info;
		//transcoder.get_image_info(basisFileData.data(), (uint32_t)basisFileData.size(), info, 0);

		//basist::basisu_image_level_info levelInfo;

		//if (!transcoder.get_image_level_info(&basisFileData[0], (uint32_t)basisFileData.size(), levelInfo, 0, 0))
		//{
		//	Debug::LogError("Failed retrieving image level information!");
		//	return false;
		//}

		//basisu::image transcodedImage(levelInfo.m_num_blocks_x, levelInfo.m_num_blocks_y);
		//fill_buffer_with_random_bytes(&transcodedImage(0, 0), transcodedImage.get_total_pixels() * sizeof(uint32_t));

		//transcoder.start_transcoding(&basisFileData[0], (uint32_t)basisFileData.size());
		////if (!transcoder.transcode_image_level(&basisFileData[0], (uint32_t)basisFileData.size(), 0, 0, dest.data() + offset, levelInfo.m_total_blocks, basist::transcoder_texture_format::cTFBC3_RGBA, 0, 0, nullptr, 0)) {

		//if (!transcoder.transcode_image_level(&basisFileData[0], (uint32_t)basisFileData.size(),
		//	0, 0, &transcodedImage(0, 0).r, transcodedImage.get_total_pixels(),
		//	basist::transcoder_texture_format::cTFBC3_RGBA, 0, transcodedImage.get_pitch(), nullptr, transcodedImage.get_height()))
		//{
		//	Debug::LogError("Failed to transcode image!");
		//	transcoder.stop_transcoding();
		//	return nullptr;
		//}
		//transcoder.stop_transcoding();

		//const size_t size = transcodedImage.get_pixels().size_in_bytes();
		//char* pixels = new char[size];
		//std::memcpy(pixels, transcodedImage.get_pixels().data(), size);

		//ImageData* pData = new ImageData(width, height, internalFormat, pixelFormat, bytesPerPixel, std::move(pixels), size, true);
		//TextureData* pDefualtTexture = new TextureData(pData);
		//pData->AddSubresource(pDefualtTexture, "Default");

		//return pData;
	}

	void BasisImporter::Initialize()
	{
		basisu::basisu_encoder_init();
		basist::basisu_transcoder_init();
	}

	void BasisImporter::Cleanup()
	{
		basisu::basisu_encoder_deinit();
	}
}
