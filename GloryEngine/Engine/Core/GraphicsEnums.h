#pragma once
#include <Reflection.h>
#include <glm/ext/vector_uint2.hpp>

REFLECTABLE_ENUM_NS(Glory, Filter,
    F_None,
    F_Nearest,
    F_Linear,
    F_CubicIMG,
    F_CubicEXT);

REFLECTABLE_ENUM_NS(Glory, SamplerAddressMode,
    SAM_Repeat,
    SAM_MirroredRepeat,
    SAM_ClampToEdge,
    SAM_ClampToBorder,
    SAM_MirrorClampToEdge,
    SAM_MirrorClampToEdgeKHR);

REFLECTABLE_ENUM_NS(Glory, CompareOp,
    OP_Never,
    OP_Less,
    OP_Equal,
    OP_LessOrEqual,
    OP_Greater,
    OP_NotEqual,
    OP_GreaterOrEqual,
    OP_Always);

REFLECTABLE_ENUM_NS(Glory, BlurType, Box, Median);

REFLECTABLE_ENUM_NS(Glory, Func,
    OP_Keep,
    OP_Zero,
    OP_Replace,
    OP_Increment,
    OP_IncrementWrap,
    OP_Decrement,
    OP_DecrementWrap,
    OP_Invert);

REFLECTABLE_ENUM_NS(Glory, CullFace,
    None,
    Front,
    Back,
    FrontAndBack);

namespace Glory
{
    constexpr size_t AspectRatioCount = 5;
    constexpr char* AspectRatios[AspectRatioCount] = {
        "4:3",
        "16:9",
        "16:10",
        "21:9",
        "32:9",
    };

    const std::vector<glm::uvec2> Resolutions[AspectRatioCount] = {
        { { 1400, 1050 }, { 1440, 1080 }, { 1600, 1200 }, { 1920, 1440 }, { 2048, 1536 } },
        { { 1280, 720 }, { 1366, 768 }, { 1600, 900 }, { 1920, 1080 }, { 2560, 1440 }, { 3840, 2160 }, { 5120, 2880 }, { 7680, 4320 } },
        { { 1280, 800 }, { 1920, 1200 }, { 2560, 1600 } },
        { { 2560, 1080 }, { 3440, 1440 }, { 3840, 1600 },  { 5120, 2160 } },
        { { 3840, 1080 }, { 5120, 1440 }, { 7680, 2160 } }
    };

	enum class PixelFormat
	{
        PF_Undefined,

        // Basic format
        PF_R,
        PF_RG,
        PF_RGB,
        PF_BGR,
        PF_RGBA,
        PF_BGRA,
        PF_RI,
        PF_RGI,
        PF_RGBI,
        PF_BGRI,
        PF_RGBAI,
        PF_BGRAI,
        PF_Stencil,
        PF_Depth,
        PF_DepthStencil,

        // Internal format
        PF_R4G4UnormPack8,
        PF_R4G4B4A4UnormPack16,
        PF_B4G4R4A4UnormPack16,
        PF_R5G6B5UnormPack16,
        PF_B5G6R5UnormPack16,
        PF_R5G5B5A1UnormPack16,
        PF_B5G5R5A1UnormPack16,
        PF_A1R5G5B5UnormPack16,
        PF_R8Unorm,
        PF_R8Snorm,
        PF_R8Uscaled,
        PF_R8Sscaled,
        PF_R8Uint,
        PF_R8Sint,
        PF_R8Srgb,
        PF_R8G8Unorm,
        PF_R8G8Snorm,
        PF_R8G8Uscaled,
        PF_R8G8Sscaled,
        PF_R8G8Uint,
        PF_R8G8Sint,
        PF_R8G8Srgb,
        PF_R8G8B8Unorm,
        PF_R8G8B8Snorm,
        PF_R8G8B8Uscaled,
        PF_R8G8B8Sscaled,
        PF_R8G8B8Uint,
        PF_R8G8B8Sint,
        PF_R8G8B8Srgb,
        PF_B8G8R8Unorm,
        PF_B8G8R8Snorm,
        PF_B8G8R8Uscaled,
        PF_B8G8R8Sscaled,
        PF_B8G8R8Uint,
        PF_B8G8R8Sint,
        PF_B8G8R8Srgb,
        PF_R8G8B8A8Unorm,
        PF_R8G8B8A8Snorm,
        PF_R8G8B8A8Uscaled,
        PF_R8G8B8A8Sscaled,
        PF_R8G8B8A8Uint,
        PF_R8G8B8A8Sint,
        PF_R8G8B8A8Srgb,
        PF_B8G8R8A8Unorm,
        PF_B8G8R8A8Snorm,
        PF_B8G8R8A8Uscaled,
        PF_B8G8R8A8Sscaled,
        PF_B8G8R8A8Uint,
        PF_B8G8R8A8Sint,
        PF_B8G8R8A8Srgb,
        PF_A8B8G8R8UnormPack32,
        PF_A8B8G8R8SnormPack32,
        PF_A8B8G8R8UscaledPack32,
        PF_A8B8G8R8SscaledPack32,
        PF_A8B8G8R8UintPack32,
        PF_A8B8G8R8SintPack32,
        PF_A8B8G8R8SrgbPack32,
        PF_A2R10G10B10UnormPack32,
        PF_A2R10G10B10SnormPack32,
        PF_A2R10G10B10UscaledPack32,
        PF_A2R10G10B10SscaledPack32,
        PF_A2R10G10B10UintPack32,
        PF_A2R10G10B10SintPack32,
        PF_A2B10G10R10UnormPack32,
        PF_A2B10G10R10SnormPack32,
        PF_A2B10G10R10UscaledPack32,
        PF_A2B10G10R10SscaledPack32,
        PF_A2B10G10R10UintPack32,
        PF_A2B10G10R10SintPack32,
        PF_R16Unorm,
        PF_R16Snorm,
        PF_R16Uscaled,
        PF_R16Sscaled,
        PF_R16Uint,
        PF_R16Sint,
        PF_R16Sfloat,
        PF_R16G16Unorm,
        PF_R16G16Snorm,
        PF_R16G16Uscaled,
        PF_R16G16Sscaled,
        PF_R16G16Uint,
        PF_R16G16Sint,
        PF_R16G16Sfloat,
        PF_R16G16B16Unorm,
        PF_R16G16B16Snorm,
        PF_R16G16B16Uscaled,
        PF_R16G16B16Sscaled,
        PF_R16G16B16Uint,
        PF_R16G16B16Sint,
        PF_R16G16B16Sfloat,
        PF_R16G16B16A16Unorm,
        PF_R16G16B16A16Snorm ,
        PF_R16G16B16A16Uscaled,
        PF_R16G16B16A16Sscaled,
        PF_R16G16B16A16Uint,
        PF_R16G16B16A16Sint,
        PF_R16G16B16A16Sfloat,
        PF_R32Uint,
        PF_R32Sint,
        PF_R32Sfloat,
        PF_R32G32Uint,
        PF_R32G32Sint,
        PF_R32G32Sfloat,
        PF_R32G32B32Uint,
        PF_R32G32B32Sint,
        PF_R32G32B32Sfloat,
        PF_R32G32B32A32Uint,
        PF_R32G32B32A32Sint,
        PF_R32G32B32A32Sfloat,
        PF_R64Uint,
        PF_R64Sint,
        PF_R64Sfloat,
        PF_R64G64Uint,
        PF_R64G64Sint,
        PF_R64G64Sfloat,
        PF_R64G64B64Uint,
        PF_R64G64B64Sint,
        PF_R64G64B64Sfloat,
        PF_R64G64B64A64Uint,
        PF_R64G64B64A64Sint,
        PF_R64G64B64A64Sfloat,
        PF_B10G11R11UfloatPack32,
        PF_E5B9G9R9UfloatPack32,
        PF_D16Unorm,
        PF_X8D24UnormPack32,
        PF_D32Sfloat,
        PF_S8Uint,
        PF_D16UnormS8Uint,
        PF_D24UnormS8Uint,
        PF_D32SfloatS8Uint,
        PF_Bc1RgbUnormBlock,
        PF_Bc1RgbSrgbBlock,
        PF_Bc1RgbaUnormBlock,
        PF_Bc1RgbaSrgbBlock,
        PF_Bc2UnormBlock,
        PF_Bc2SrgbBlock,
        PF_Bc3UnormBlock,
        PF_Bc3SrgbBlock,
        PF_Bc4UnormBlock,
        PF_Bc4SnormBlock,
        PF_Bc5UnormBlock,
        PF_Bc5SnormBlock,
        PF_Bc6HUfloatBlock,
        PF_Bc6HSfloatBlock,
        PF_Bc7UnormBlock,
        PF_Bc7SrgbBlock,
        PF_Etc2R8G8B8UnormBlock,
        PF_Etc2R8G8B8SrgbBlock,
        PF_Etc2R8G8B8A1UnormBlock,
        PF_Etc2R8G8B8A1SrgbBlock,
        PF_Etc2R8G8B8A8UnormBlock,
        PF_Etc2R8G8B8A8SrgbBlock,
        PF_EacR11UnormBlock,
        PF_EacR11SnormBlock,
        PF_EacR11G11UnormBlock,
        PF_EacR11G11SnormBlock,
        PF_Astc4x4UnormBlock,
        PF_Astc4x4SrgbBlock,
        PF_Astc5x4UnormBlock,
        PF_Astc5x4SrgbBlock,
        PF_Astc5x5UnormBlock,
        PF_Astc5x5SrgbBlock,
        PF_Astc6x5UnormBlock,
        PF_Astc6x5SrgbBlock,
        PF_Astc6x6UnormBlock,
        PF_Astc6x6SrgbBlock,
        PF_Astc8x5UnormBlock,
        PF_Astc8x5SrgbBlock,
        PF_Astc8x6UnormBlock,
        PF_Astc8x6SrgbBlock,
        PF_Astc8x8UnormBlock,
        PF_Astc8x8SrgbBlock,
        PF_Astc10x5UnormBlock,
        PF_Astc10x5SrgbBlock,
        PF_Astc10x6UnormBlock,
        PF_Astc10x6SrgbBlock,
        PF_Astc10x8UnormBlock,
        PF_Astc10x8SrgbBlock,
        PF_Astc10x10UnormBlock,
        PF_Astc10x10SrgbBlock,
        PF_Astc12x10UnormBlock,
        PF_Astc12x10SrgbBlock,
        PF_Astc12x12UnormBlock,
        PF_Astc12x12SrgbBlock,

        PF_Depth16,
        PF_Depth24,
        PF_Depth32,


        // NOT SUPPORTED FOR NOW
        PF_G8B8G8R8422Unorm = 1000156000,
        PF_B8G8R8G8422Unorm,
        PF_G8B8R83Plane420Unorm,
        PF_G8B8R82Plane420Unorm,
        PF_G8B8R83Plane422Unorm,
        PF_G8B8R82Plane422Unorm,
        PF_G8B8R83Plane444Unorm,
        PF_R10X6UnormPack16,
        PF_R10X6G10X6Unorm2Pack16,
        PF_R10X6G10X6B10X6A10X6Unorm4Pack16,
        PF_G10X6B10X6G10X6R10X6422Unorm4Pack16,
        PF_B10X6G10X6R10X6G10X6422Unorm4Pack16,
        PF_G10X6B10X6R10X63Plane420Unorm3Pack16,
        PF_G10X6B10X6R10X62Plane420Unorm3Pack16,
        PF_G10X6B10X6R10X63Plane422Unorm3Pack16,
        PF_G10X6B10X6R10X62Plane422Unorm3Pack16,
        PF_G10X6B10X6R10X63Plane444Unorm3Pack16,
        PF_R12X4UnormPack16,
        PF_R12X4G12X4Unorm2Pack16,
        PF_R12X4G12X4B12X4A12X4Unorm4Pack16,
        PF_G12X4B12X4G12X4R12X4422Unorm4Pack16,
        PF_B12X4G12X4R12X4G12X4422Unorm4Pack16,
        PF_G12X4B12X4R12X43Plane420Unorm3Pack16,
        PF_G12X4B12X4R12X42Plane420Unorm3Pack16,
        PF_G12X4B12X4R12X43Plane422Unorm3Pack16,
        PF_G12X4B12X4R12X42Plane422Unorm3Pack16,
        PF_G12X4B12X4R12X43Plane444Unorm3Pack16,
        PF_G16B16G16R16422Unorm,
        PF_B16G16R16G16422Unorm,
        PF_G16B16R163Plane420Unorm,
        PF_G16B16R162Plane420Unorm,
        PF_G16B16R163Plane422Unorm,
        PF_G16B16R162Plane422Unorm,
        PF_G16B16R163Plane444Unorm,
        PF_Pvrtc12BppUnormBlockIMG,
        PF_Pvrtc14BppUnormBlockIMG,
        PF_Pvrtc22BppUnormBlockIMG,
        PF_Pvrtc24BppUnormBlockIMG,
        PF_Pvrtc12BppSrgbBlockIMG,
        PF_Pvrtc14BppSrgbBlockIMG,
        PF_Pvrtc22BppSrgbBlockIMG,
        PF_Pvrtc24BppSrgbBlockIMG,
        PF_Astc4x4SfloatBlockEXT,
        PF_Astc5x4SfloatBlockEXT,
        PF_Astc5x5SfloatBlockEXT,
        PF_Astc6x5SfloatBlockEXT,
        PF_Astc6x6SfloatBlockEXT,
        PF_Astc8x5SfloatBlockEXT,
        PF_Astc8x6SfloatBlockEXT,
        PF_Astc8x8SfloatBlockEXT,
        PF_Astc10x5SfloatBlockEXT,
        PF_Astc10x6SfloatBlockEXT,
        PF_Astc10x8SfloatBlockEXT,
        PF_Astc10x10SfloatBlockEXT,
        PF_Astc12x10SfloatBlockEXT,
        PF_Astc12x12SfloatBlockEXT,
        PF_B10X6G10X6R10X6G10X6422Unorm4Pack16KHR,
        PF_B12X4G12X4R12X4G12X4422Unorm4Pack16KHR,
        PF_B16G16R16G16422UnormKHR,
        PF_B8G8R8G8422UnormKHR,
        PF_G10X6B10X6G10X6R10X6422Unorm4Pack16KHR,
        PF_G10X6B10X6R10X62Plane420Unorm3Pack16KHR,
        PF_G10X6B10X6R10X62Plane422Unorm3Pack16KHR,
        PF_G10X6B10X6R10X63Plane420Unorm3Pack16KHR,
        PF_G10X6B10X6R10X63Plane422Unorm3Pack16KHR,
        PF_G10X6B10X6R10X63Plane444Unorm3Pack16KHR,
        PF_G12X4B12X4G12X4R12X4422Unorm4Pack16KHR,
        PF_G12X4B12X4R12X42Plane420Unorm3Pack16KHR,
        PF_G12X4B12X4R12X42Plane422Unorm3Pack16KHR,
        PF_G12X4B12X4R12X43Plane420Unorm3Pack16KHR,
        PF_G12X4B12X4R12X43Plane422Unorm3Pack16KHR,
        PF_G12X4B12X4R12X43Plane444Unorm3Pack16KHR,
        PF_G16B16G16R16422UnormKHR,
        PF_G16B16R162Plane420UnormKHR,
        PF_G16B16R162Plane422UnormKHR,
        PF_G16B16R163Plane420UnormKHR,
        PF_G16B16R163Plane422UnormKHR,
        PF_G16B16R163Plane444UnormKHR,
        PF_G8B8G8R8422UnormKHR,
        PF_G8B8R82Plane420UnormKHR,
        PF_G8B8R82Plane422UnormKHR,
        PF_G8B8R83Plane420UnormKHR,
        PF_G8B8R83Plane422UnormKHR,
        PF_G8B8R83Plane444UnormKHR,
        PF_R10X6G10X6B10X6A10X6Unorm4Pack16KHR,
        PF_R10X6G10X6Unorm2Pack16KHR,
        PF_R10X6UnormPack16KHR,
        PF_R12X4G12X4B12X4A12X4Unorm4Pack16KHR,
        PF_R12X4G12X4Unorm2Pack16KHR,
        PF_R12X4UnormPack16KHR,
	};

	enum class ImageType
	{
		IT_UNDEFINED,
		IT_1D,
		IT_2D,
		IT_3D,
		IT_Cube,
		IT_1DArray,
		IT_2DArray,
		IT_CubeArray,
        IT_Count
	};

	enum ImageAspect : int
	{
		IA_Color			= 0x00000001,
		IA_Depth			= 0x00000002,
		IA_Stencil			= 0x00000004,
		IA_Metadata			= 0x00000008,
		IA_Plane0			= 0x00000010,
		IA_Plane1			= 0x00000020,
		IA_Plane2			= 0x00000040,
		IA_MemoryPlane0EXT	= 0x00000080,
		IA_MemoryPlane1EXT	= 0x00000100,
		IA_MemoryPlane2EXT	= 0x00000200,
		IA_MemoryPlane3EXT	= 0x00000400,
		IA_Plane0KHR		= 0x00000800,
		IA_Plane1KHR		= 0x00001000,
		IA_Plane2KHR		= 0x00002000,
	};

	struct SamplerSettings
	{
        constexpr SamplerSettings() : MagFilter(Filter::F_Linear), MinFilter(Filter::F_Linear),
            AddressModeU(SamplerAddressMode::SAM_Repeat), AddressModeV(SamplerAddressMode::SAM_Repeat),
            AddressModeW(SamplerAddressMode::SAM_Repeat), AnisotropyEnable(false), MaxAnisotropy(1.f),
            UnnormalizedCoordinates(false), CompareEnable(false), CompareOp(CompareOp::OP_Never),
            MipmapMode(Filter::F_Linear), MipLODBias(0.0f), MinLOD(0.f), MaxLOD(0.f)
        {};

		Filter MagFilter;
		Filter MinFilter;
		SamplerAddressMode AddressModeU;
		SamplerAddressMode AddressModeV;
		SamplerAddressMode AddressModeW;
		bool AnisotropyEnable;
		float MaxAnisotropy;
		bool UnnormalizedCoordinates;
		bool CompareEnable;
		CompareOp CompareOp;
		Filter MipmapMode;
		float MipLODBias;
		float MinLOD;
		float MaxLOD;
	};

    static constexpr SamplerSettings DefaultSampler = SamplerSettings();

    enum class ShaderType
    {
        ST_Unknown,
        ST_Vertex,
        ST_Fragment,
        ST_Geomtery,
        ST_TessControl,
        ST_TessEval,
        ST_Compute,

        ST_Count,
    };

    enum ShaderTypeFlag : uint8_t
    {
        STF_Vertex = 1 << 0,
        STF_Fragment = 1 << 1,
        STF_Geomtery = 1 << 2,
        STF_TessControl = 1 << 3,
        STF_TessEval = 1 << 4,
        STF_Compute = 1 << 5,
    };

    constexpr size_t ShaderTypeToFlagCount = 7;
    constexpr ShaderTypeFlag ShaderTypeToFlag[ShaderTypeToFlagCount] = {
        ShaderTypeFlag(0),
        ShaderTypeFlag::STF_Vertex,
        ShaderTypeFlag::STF_Fragment,
        ShaderTypeFlag::STF_Geomtery,
        ShaderTypeFlag::STF_TessControl,
        ShaderTypeFlag::STF_TessEval,
        ShaderTypeFlag::STF_Compute,
    };

    static_assert(ShaderTypeToFlagCount == (size_t)ShaderType::ST_Count);

    enum class BufferBindingTarget
    {
        B_ARRAY,
        B_ATOMIC_COUNTER,
        B_COPY_READ,
        B_COPY_WRITE,
        B_DISPATCH_INDIRECT,
        B_DRAW_INDIRECT,
        B_ELEMENT_ARRAY,
        B_PIXEL_PACK,
        B_PIXEL_UNPACK,
        B_QUERY,
        B_SHADER_STORAGE,
        B_TEXTURE,
        B_TRANSFORM_FEEDBACK,
        B_UNIFORM,

        //B_TransferSrc = 1 << 0,
        //B_TransferDst = 1 << 1,
        //B_UniformTexelBuffer = 1 << 2,
        //B_StorageTexelBuffer = 1 << 3,
        //B_UniformBuffer = 1 << 4,
        //B_StorageBuffer = 1 << 5,
        //B_IndexBuffer = 1 << 7,
        //B_VertexBuffer = 1 << 8,
        //B_IndirectBuffer = 1 << 9,
        //B_TransformFeedbackBufferEXT = 1 << 15,
        //B_TransformFeedbackCounterBufferEXT = 1 << 16,
        //B_ConditionalRenderingEXT = 1 << 2,
        //B_AccelerationStructureBuildInputReadOnlyKHR = 1 << 17,
        //B_AccelerationStructureStorageKHR = 1 << 18,
        //B_ShaderBindingTableKHR = 1 << 19,
        //B_RayTracingNV = 1 << 20,
        //B_VideoEncodeDstKHR = 1 << 21,
        //B_VideoEncodeSrcKHR = 1 << 22,
        //B_SamplerDescriptorBufferEXT = 1 << 23,
        //B_ResourceDescriptorBufferEXT = 1 << 24,
        //B_PushDescriptorsDescriptorBufferEXT = 1 << 25,
        //B_MicromapBuildInputReadOnlyEXT = 1 << 26,
        //B_MicromapStorageEXT = 1 << 27,
        //B_TileMemoryQCOM = 1 << 28,
    };

    enum class MemoryUsage
    {
        MU_STREAM_DRAW,
        MU_STREAM_READ,
        MU_STREAM_COPY,
        MU_STATIC_DRAW,
        MU_STATIC_READ,
        MU_STATIC_COPY,
        MU_DYNAMIC_DRAW,
        MU_DYNAMIC_READ,
        MU_DYNAMIC_COPY,

        //MU_DeviceLocal = 1 << 0,
        //MU_HostVisible = 1 << 1,
        //MU_HostCoherent = 1 << 2,
        //MU_HostCached = 1 << 3,
        //MU_LazilyAllocated = 1 << 4,
        //MU_Protected = 1 << 5,
        //MU_DeviceCoherentAMD = 1 << 6,
        //MU_DeviceUncachedAMD = 1 << 7,
        //MU_RdmaCapableNV = 1 << 8,
    };

    enum class PrimitiveType
    {
        PT_Point,
        PT_LineStrip,
        PT_LineLoop,
        PT_Lines,
        PT_LineStripAdjacency,
        PT_LinesAdjacency,
        PT_TriangleStrip,
        PT_TriangleFan,
        PT_Triangles,
        PT_TriangleStripAdjacency,
        PT_TrianglesAdjacency,
        PT_Patches
    };

    enum class DataType
    {
        DT_Byte,
        DT_UByte,
        DT_Short,
        DT_UShort,
        DT_Int,
        DT_UInt,
        DT_Float,
        DT_2Bytes,
        DT_3Bytes,
        DT_4Bytes,
        DT_Double
    };

    /** @brief Pipeline type based on ASSIMP aiShadingMode */
    enum PipelineType
    {
        /** @brief Unknown */
        PT_Unknown = 0,

        /** @brief Flat shading. Shading is done on per-face base,
         *  diffuse only. Also known as 'faceted shading'.
         */
        PT_Flat = 1,

        /** @brief Simple Gouraud shading. */
        PT_Gouraud = 2,

        /** @brief Phong-Shading */
        PT_Phong = 3,

        /** @brief Phong-Blinn-Shading */
        PT_Blinn = 4,

        /** @brief Toon-Shading per pixel
         *
         *  Also known as 'comic' shader.
         */
        PT_Toon = 5,

        /** @brief OrenNayar-Shading per pixel
         *
         *  Extension to standard Lambertian shading, taking the
         *  roughness of the material into account
         */
        PT_OrenNayar = 6,

        /** @brief Minnaert-Shading per pixel
         *
         *  Extension to standard Lambertian shading, taking the
         *  "darkness" of the material into account
         */
        PT_Minnaert = 7,

        /** @brief CookTorrance-Shading per pixel
         *
         *  Special shader for metallic surfaces.
         */
        PT_CookTorrance = 8,

        /** @brief No shading at all. Constant light influence of 1.0. */
        PT_Unlit = 9,

        /** @brief Fresnel shading */
        PT_Fresnel = 10,

        /** @brief Physically-Based Rendering (PBR) shading using
        * Bidirectional scattering/reflectance distribution function (BSDF/BRDF)
        * There are multiple methods under this banner, and model files may provide
        * data for more than one PBR-BRDF method.
        * Applications should use the set of provided properties to determine which
        * of their preferred PBR rendering methods are likely to be available
        * eg:
        * - If AI_MATKEY_METALLIC_FACTOR is set, then a Metallic/Roughness is available
        * - If AI_MATKEY_GLOSSINESS_FACTOR is set, then a Specular/Glossiness is available
        * Note that some PBR methods allow layering of techniques
        */
        PT_PBR_BRDF = 11,

        /** @brief Text rendering */
        PT_Text = 12,

        /** @brief Screen effects */
        PT_Screen = 13,

        /** @brief UI rendering */
        PT_UI = 14,

        /** @brief For any other pipelines */
        PT_Other = 15,

        /** @brief For skybox rendering */
        PT_Skybox = 16,

        /** @brief For rendering shadow maps */
        PT_Shadow = 17,

        /** @brief For compute pipelines */
        PT_Compute = 18,

        /** @brief Number of pipeline types */
        PT_Count,
    };

    const std::string Enum<PipelineType>::m_EnumStringValues[] = {
        "Unknown",
        "Flat",
        "Gouraud",
        "Phong",
        "Blinn",
        "Toon",
        "OrenNayar",
        "Minnaert",
        "CookTorrance",
        "Unlit",
        "Fresnel",
        "PBR_BRDF",
        "Text",
        "Screen",
        "Other",
        "Skybox",
        "Shadow",
        "Compute",
    };

    const size_t Enum<PipelineType>::m_NumValues = PT_Count;
    bool Enum<PipelineType>::Valid() { return true; }

    /** @brief Texture type based on ASSIMP aiTextureType */
    enum TextureType : uint32_t
    {
        /** Dummy value.
         *
         *  No texture, but the value to be used as 'texture semantic'
         *  (#aiMaterialProperty::mSemantic) for all material properties
         *  *not* related to textures.
         */
        TT_None = 0,

        /** LEGACY API MATERIALS
         * Legacy refers to materials which
         * Were originally implemented in the specifications around 2000.
         * These must never be removed, as most engines support them.
         */

         /** The texture is combined with the result of the diffuse
          *  lighting equation.
          *  OR
          *  PBR Specular/Glossiness
          */
        TT_Diffuse = 1,

        /** The texture is combined with the result of the specular
         *  lighting equation.
         *  OR
         *  PBR Specular/Glossiness
         */
        TT_Specular = 2,

        /** The texture is combined with the result of the ambient
         *  lighting equation.
         */
        TT_Ambient = 3,

        /** The texture is added to the result of the lighting
         *  calculation. It isn't influenced by incoming light.
         */
        TT_Emissive = 4,

        /** The texture is a height map.
         *
         *  By convention, higher gray-scale values stand for
         *  higher elevations from the base height.
         */
        TT_Height = 5,

        /** The texture is a (tangent space) normal-map.
         *
         *  Again, there are several conventions for tangent-space
         *  normal maps. Assimp does (intentionally) not
         *  distinguish here.
         */
        TT_Normals = 6,

        /** The texture defines the glossiness of the material.
         *
         *  The glossiness is in fact the exponent of the specular
         *  (phong) lighting equation. Usually there is a conversion
         *  function defined to map the linear color values in the
         *  texture to a suitable exponent. Have fun.
        */
        TT_Shininess = 7,

        /** The texture defines per-pixel opacity.
         *
         *  Usually 'white' means opaque and 'black' means
         *  'transparency'. Or quite the opposite. Have fun.
        */
        TT_Opacity = 8,

        /** Displacement texture
         *
         *  The exact purpose and format is application-dependent.
         *  Higher color values stand for higher vertex displacements.
        */
        TT_Displacement = 9,

        /** Lightmap texture (aka Ambient Occlusion)
         *
         *  Both 'Lightmaps' and dedicated 'ambient occlusion maps' are
         *  covered by this material property. The texture contains a
         *  scaling value for the final color value of a pixel. Its
         *  intensity is not affected by incoming light.
        */
        TT_Lightmap = 10,

        /** Reflection texture
         *
         * Contains the color of a perfect mirror reflection.
         * Rarely used, almost never for real-time applications.
        */
        TT_Relection = 11,

        /** PBR Materials
         * PBR definitions from maya and other modelling packages now use this standard.
         * This was originally introduced around 2012.
         * Support for this is in game engines like Godot, Unreal or Unity3D.
         * Modelling packages which use this are very common now.
         */

        TT_BaseColor = 12,
        TT_NormalCamera = 13,
        TT_EmissionColor = 14,
        TT_Metalness = 15,
        TT_DiffuseRoughness = 16,
        TT_AmbientOcclusion = 17,

        /** Unknown texture
         *
         *  A texture reference that does not match any of the definitions
         *  above is considered to be 'unknown'. It is still imported,
         *  but is excluded from any further post-processing.
        */
        TT_Unknown = 18,

        /** PBR Material Modifiers
        * Some modern renderers have further PBR modifiers that may be overlaid
        * on top of the 'base' PBR materials for additional realism.
        * These use multiple texture maps, so only the base type is directly defined
        */

        /** Sheen
        * Generally used to simulate textiles that are covered in a layer of microfibers
        * eg velvet
        * https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_materials_sheen
        */
        TT_Seen = 19,

        /** Clearcoat
        * Simulates a layer of 'polish' or 'laquer' layered on top of a PBR substrate
        * https://autodesk.github.io/standard-surface/#closures/coating
        * https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_materials_clearcoat
        */
        TT_Clearcoat = 20,

        /** Transmission
        * Simulates transmission through the surface
        * May include further information such as wall thickness
        */
        TT_Transmission = 21,

        TT_Count
    };

    const std::string Enum<TextureType>::m_EnumStringValues[] = {
        "None",
        "Diffuse",
        "Specular",
        "Ambient",
        "Emissive",
        "Height",
        "Normal",
        "Shininess",
        "Opacity",
        "Displacement",
        "Lightmap",
        "Relection",
        "BaseColor",
        "NormalCamera",
        "EmissionColor",
        "Metalness",
        "DiffuseRoughness",
        "AmbientOcclusion",
        "Unknown",
        "Seen",
        "Clearcoat",
        "Transmission",
    };

    const size_t Enum<TextureType>::m_NumValues = TT_Count;
    bool Enum<TextureType>::Valid() { return true; }

    struct LightingSettings
    {
        uint64_t m_Skybox;
        uint64_t m_IrradianceMap;
    };

    struct SSAOSettings
    {
        int m_Enabled = 1;
        int m_Dirty = 1;
        float m_SampleRadius = 2.5f;
        float m_SampleBias = 0.025f;
        int m_KernelSize = 12;
        BlurType m_BlurType = BlurType::Box;
        int m_BlurSize = 2;
        float m_Separation = 0.5f;
        int m_BinsSize = 10;
        float m_Magnitude = 1.0f;
        float m_Contrast = 1.0f;
    };

    constexpr SSAOSettings DefaultSSAO;

    struct PipelineShaderMetaData
    {
        uint64_t PipelineID;
        ShaderType Type;
    };

    struct TextureCreateInfo
    {
        uint32_t m_Width;
        uint32_t m_Height;
        PixelFormat m_PixelFormat;
        PixelFormat m_InternalFormat;
        ImageType m_ImageType;
        DataType m_Type;
        uint32_t m_UsageFlags;
        uint32_t m_SharingMode;
        ImageAspect m_ImageAspectFlags;
        SamplerSettings m_SamplerSettings = SamplerSettings();
    };

    struct Attachment
    {
        Attachment(const std::string& name, const PixelFormat& pixelFormat, const PixelFormat& internalFormat,
            const ImageType& imageType, const ImageAspect& imageAspect, DataType type = DataType::DT_UByte, bool autoBind = true);
        Attachment(const std::string& name, const TextureCreateInfo& textureInfo, bool autoBind = true);

        //PixelFormat::PF_R8G8B8A8Srgb
        std::string Name;
        PixelFormat InternalFormat;
        PixelFormat Format;
        ImageType ImageType;
        ImageAspect ImageAspect;
        DataType m_Type;
        bool m_AutoBind;
    };

    struct RenderTextureCreateInfo
    {
    public:
        RenderTextureCreateInfo();
        RenderTextureCreateInfo(uint32_t width, uint32_t height, bool hasDepth, bool hasStencil = false);

        uint32_t Width;
        uint32_t Height;
        bool HasDepth;
        bool HasStencil;
        std::vector<Attachment> Attachments;
    };

    struct ShaderBufferInfo
    {
        std::string Name;
        ShaderTypeFlag ShaderFlags;
    };
}
