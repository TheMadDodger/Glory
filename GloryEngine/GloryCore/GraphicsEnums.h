#pragma once

namespace Glory
{
	enum class PixelFormat
	{
		PF_RED,
		PF_GREEN,
		PF_BLUE,
		PF_ALPHA,
		PF_RGB,
		PF_RGBA,
		PF_BGR,
		PF_BGRA,
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

	enum class Filter
	{
		F_Nearest,
		F_Linear,
		F_CubicIMG,
		F_CubicEXT,
	};

	enum class SamplerAddressMode
	{
		SAM_Repeat,
		SAM_MirroredRepeat,
		SAM_ClampToEdge,
		SAM_ClampToBorder,
		SAM_MirrorClampToEdge,
		SAM_MirrorClampToEdgeKHR,
	};

	enum class CompareOp
	{
		OP_Never,
		OP_Less,
		OP_Equal,
		OP_LessOrEqual,
		OP_Greater,
		OP_NotEqual,
		OP_GreaterOrEqual,
		OP_Always,
	};

	struct SamplerSettings
	{
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
}
