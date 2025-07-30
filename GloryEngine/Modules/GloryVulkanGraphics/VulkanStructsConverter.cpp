#include "VulkanStructsConverter.h"

namespace Glory
{
    const std::map<PixelFormat, vk::Format> TO_VKFORMAT = {
        { PixelFormat::PF_Undefined,                 vk::Format::eUndefined },

        // Basic format
        { PixelFormat::PF_R,                         vk::Format::eR8Srgb },
        { PixelFormat::PF_RG,                        vk::Format::eR8G8Srgb },
        { PixelFormat::PF_RGB,                       vk::Format::eR8G8B8Srgb },
        { PixelFormat::PF_BGR,                       vk::Format::eB8G8R8Srgb },
        { PixelFormat::PF_RGBA,                      vk::Format::eR8G8B8A8Srgb },
        { PixelFormat::PF_BGRA,                      vk::Format::eR8G8B8A8Srgb },
        { PixelFormat::PF_RI,                        vk::Format::eR8Sint },
        { PixelFormat::PF_RGI,                       vk::Format::eR8G8Sint },
        { PixelFormat::PF_RGBI,                      vk::Format::eR8G8B8Sint },
        { PixelFormat::PF_BGRI,                      vk::Format::eB8G8R8Sint },
        { PixelFormat::PF_RGBAI,                     vk::Format::eR8G8B8A8Sint },
        { PixelFormat::PF_BGRAI,                     vk::Format::eB8G8R8A8Sint },
        //{ PixelFormat::PF_Stencil,                   GL_STENCIL_INDEX },
        //{ PixelFormat::PF_Depth,                     GL_DEPTH_COMPONENT },
        //{ PixelFormat::PF_DepthStencil,              GL_DEPTH_STENCIL },

        // Internal format
        { PixelFormat::PF_R4G4UnormPack8,            vk::Format::eR4G4UnormPack8,          },
        { PixelFormat::PF_R4G4B4A4UnormPack16,       vk::Format::eR4G4B4A4UnormPack16,     },
        { PixelFormat::PF_B4G4R4A4UnormPack16,       vk::Format::eB4G4R4A4UnormPack16,     },
        { PixelFormat::PF_R5G6B5UnormPack16,         vk::Format::eR5G6B5UnormPack16,       },
        { PixelFormat::PF_B5G6R5UnormPack16,         vk::Format::eB5G6R5UnormPack16,       },
        { PixelFormat::PF_R5G5B5A1UnormPack16,       vk::Format::eR5G5B5A1UnormPack16,     },
        { PixelFormat::PF_B5G5R5A1UnormPack16,       vk::Format::eB5G5R5A1UnormPack16,     },
        { PixelFormat::PF_A1R5G5B5UnormPack16,       vk::Format::eA1R5G5B5UnormPack16,     },
        { PixelFormat::PF_R8Unorm,                   vk::Format::eR8Unorm,                 },
        { PixelFormat::PF_R8Snorm,                   vk::Format::eR8Snorm,                 },
        { PixelFormat::PF_R8Uscaled,                 vk::Format::eR8Uscaled,               },
        { PixelFormat::PF_R8Sscaled,                 vk::Format::eR8Sscaled,               },
        { PixelFormat::PF_R8Uint,                    vk::Format::eR8Uint,                  },
        { PixelFormat::PF_R8Sint,                    vk::Format::eR8Sint,                  },
        { PixelFormat::PF_R8Srgb,                    vk::Format::eR8Srgb,                  },
        { PixelFormat::PF_R8G8Unorm,                 vk::Format::eR8G8Unorm,               },
        { PixelFormat::PF_R8G8Snorm,                 vk::Format::eR8G8Snorm,               },
        { PixelFormat::PF_R8G8Uscaled,               vk::Format::eR8G8Uscaled,             },
        { PixelFormat::PF_R8G8Sscaled,               vk::Format::eR8G8Sscaled,             },
        { PixelFormat::PF_R8G8Uint,                  vk::Format::eR8G8Uint,                },
        { PixelFormat::PF_R8G8Sint,                  vk::Format::eR8G8Sint,                },
        { PixelFormat::PF_R8G8Srgb,                  vk::Format::eR8G8Srgb,                },
        { PixelFormat::PF_R8G8B8Unorm,               vk::Format::eR8G8B8Unorm,             },
        { PixelFormat::PF_R8G8B8Snorm,               vk::Format::eR8G8B8Snorm,             },
        { PixelFormat::PF_R8G8B8Uscaled,             vk::Format::eR8G8B8Uscaled,           },
        { PixelFormat::PF_R8G8B8Sscaled,             vk::Format::eR8G8B8Sscaled,           },
        { PixelFormat::PF_R8G8B8Uint,                vk::Format::eR8G8B8Uint,              },
        { PixelFormat::PF_R8G8B8Sint,                vk::Format::eR8G8B8Sint,              },
        { PixelFormat::PF_R8G8B8Srgb,                vk::Format::eR8G8B8Srgb,              },
        { PixelFormat::PF_B8G8R8Unorm,               vk::Format::eB8G8R8Unorm,             },
        { PixelFormat::PF_B8G8R8Snorm,               vk::Format::eB8G8R8Snorm,             },
        { PixelFormat::PF_B8G8R8Uscaled,             vk::Format::eB8G8R8Uscaled,           },
        { PixelFormat::PF_B8G8R8Sscaled,             vk::Format::eB8G8R8Sscaled,           },
        { PixelFormat::PF_B8G8R8Uint,                vk::Format::eB8G8R8Uint,              },
        { PixelFormat::PF_B8G8R8Sint,                vk::Format::eB8G8R8Sint,              },
        { PixelFormat::PF_B8G8R8Srgb,                vk::Format::eB8G8R8Srgb,              },
        { PixelFormat::PF_R8G8B8A8Unorm,             vk::Format::eR8G8B8A8Unorm,           },
        { PixelFormat::PF_R8G8B8A8Snorm,             vk::Format::eR8G8B8A8Snorm,           },
        { PixelFormat::PF_R8G8B8A8Uscaled,           vk::Format::eR8G8B8A8Uscaled,         },
        { PixelFormat::PF_R8G8B8A8Sscaled,           vk::Format::eR8G8B8A8Sscaled,         },
        { PixelFormat::PF_R8G8B8A8Uint,              vk::Format::eR8G8B8A8Uint,            },
        { PixelFormat::PF_R8G8B8A8Sint,              vk::Format::eR8G8B8A8Sint,            },
        { PixelFormat::PF_R8G8B8A8Srgb,              vk::Format::eR8G8B8A8Srgb,            },
        { PixelFormat::PF_B8G8R8A8Unorm,             vk::Format::eB8G8R8A8Unorm,           },
        { PixelFormat::PF_B8G8R8A8Snorm,             vk::Format::eB8G8R8A8Snorm,           },
        { PixelFormat::PF_B8G8R8A8Uscaled,           vk::Format::eB8G8R8A8Uscaled,         },
        { PixelFormat::PF_B8G8R8A8Sscaled,           vk::Format::eB8G8R8A8Sscaled,         },
        { PixelFormat::PF_B8G8R8A8Uint,              vk::Format::eB8G8R8A8Uint,            },
        { PixelFormat::PF_B8G8R8A8Sint,              vk::Format::eB8G8R8A8Sint,            },
        { PixelFormat::PF_B8G8R8A8Srgb,              vk::Format::eB8G8R8A8Srgb,            },
        { PixelFormat::PF_A8B8G8R8UnormPack32,       vk::Format::eA8B8G8R8UnormPack32,     },
        { PixelFormat::PF_A8B8G8R8SnormPack32,       vk::Format::eA8B8G8R8SnormPack32,     },
        { PixelFormat::PF_A8B8G8R8UscaledPack32,     vk::Format::eA8B8G8R8UscaledPack32,   },
        { PixelFormat::PF_A8B8G8R8SscaledPack32,     vk::Format::eA8B8G8R8SscaledPack32,   },
        { PixelFormat::PF_A8B8G8R8UintPack32,        vk::Format::eA8B8G8R8UintPack32,      },
        { PixelFormat::PF_A8B8G8R8SintPack32,        vk::Format::eA8B8G8R8SintPack32,      },
        { PixelFormat::PF_A8B8G8R8SrgbPack32,        vk::Format::eA8B8G8R8SrgbPack32,      },
        { PixelFormat::PF_A2R10G10B10UnormPack32,    vk::Format::eA2R10G10B10UnormPack32,  },
        { PixelFormat::PF_A2R10G10B10SnormPack32,    vk::Format::eA2R10G10B10SnormPack32,  },
        { PixelFormat::PF_A2R10G10B10UscaledPack32,  vk::Format::eA2R10G10B10UscaledPack32,},
        { PixelFormat::PF_A2R10G10B10SscaledPack32,  vk::Format::eA2R10G10B10SscaledPack32,},
        { PixelFormat::PF_A2R10G10B10UintPack32,     vk::Format::eA2R10G10B10UintPack32,   },
        { PixelFormat::PF_A2R10G10B10SintPack32,     vk::Format::eA2R10G10B10SintPack32,   },
        { PixelFormat::PF_A2B10G10R10UnormPack32,    vk::Format::eA2B10G10R10UnormPack32,  },
        { PixelFormat::PF_A2B10G10R10SnormPack32,    vk::Format::eA2B10G10R10SnormPack32,  },
        { PixelFormat::PF_A2B10G10R10UscaledPack32,  vk::Format::eA2B10G10R10UscaledPack32,},
        { PixelFormat::PF_A2B10G10R10SscaledPack32,  vk::Format::eA2B10G10R10SscaledPack32,},
        { PixelFormat::PF_A2B10G10R10UintPack32,     vk::Format::eA2B10G10R10UintPack32,   },
        { PixelFormat::PF_A2B10G10R10SintPack32,     vk::Format::eA2B10G10R10SintPack32,   },
        { PixelFormat::PF_R16Unorm,                  vk::Format::eR16Unorm,                },
        { PixelFormat::PF_R16Snorm,                  vk::Format::eR16Snorm,                },
        { PixelFormat::PF_R16Uscaled,                vk::Format::eR16Uscaled,              },
        { PixelFormat::PF_R16Sscaled,                vk::Format::eR16Sscaled,              },
        { PixelFormat::PF_R16Uint,                   vk::Format::eR16Uint,                 },
        { PixelFormat::PF_R16Sint,                   vk::Format::eR16Sint,                 },
        { PixelFormat::PF_R16Sfloat,                 vk::Format::eR16Sfloat,               },
        { PixelFormat::PF_R16G16Unorm,               vk::Format::eR16G16Unorm,             },
        { PixelFormat::PF_R16G16Snorm,               vk::Format::eR16G16Snorm,             },
        { PixelFormat::PF_R16G16Uscaled,             vk::Format::eR16G16Uscaled,           },
        { PixelFormat::PF_R16G16Sscaled,             vk::Format::eR16G16Sscaled,           },
        { PixelFormat::PF_R16G16Uint,                vk::Format::eR16G16Uint,              },
        { PixelFormat::PF_R16G16Sint,                vk::Format::eR16G16Sint,              },
        { PixelFormat::PF_R16G16Sfloat,              vk::Format::eR16G16Sfloat,            },
        { PixelFormat::PF_R16G16B16Unorm,            vk::Format::eR16G16B16Unorm,          },
        { PixelFormat::PF_R16G16B16Snorm,            vk::Format::eR16G16B16Snorm,          },
        { PixelFormat::PF_R16G16B16Uscaled,          vk::Format::eR16G16B16Uscaled,        },
        { PixelFormat::PF_R16G16B16Sscaled,          vk::Format::eR16G16B16Sscaled,        },
        { PixelFormat::PF_R16G16B16Uint,             vk::Format::eR16G16B16Uint,           },
        { PixelFormat::PF_R16G16B16Sint,             vk::Format::eR16G16B16Sint,           },
        { PixelFormat::PF_R16G16B16Sfloat,           vk::Format::eR16G16B16Sfloat,         },
        { PixelFormat::PF_R16G16B16A16Unorm,         vk::Format::eR16G16B16A16Unorm,       },
        { PixelFormat::PF_R16G16B16A16Snorm ,        vk::Format::eR16G16B16A16Snorm ,      },
        { PixelFormat::PF_R16G16B16A16Uscaled,       vk::Format::eR16G16B16A16Uscaled,     },
        { PixelFormat::PF_R16G16B16A16Sscaled,       vk::Format::eR16G16B16A16Sscaled,     },
        { PixelFormat::PF_R16G16B16A16Uint,          vk::Format::eR16G16B16A16Uint,        },
        { PixelFormat::PF_R16G16B16A16Sint,          vk::Format::eR16G16B16A16Sint,        },
        { PixelFormat::PF_R16G16B16A16Sfloat,        vk::Format::eR16G16B16A16Sfloat,      },
        { PixelFormat::PF_R32Uint,                   vk::Format::eR32Uint,                 },
        { PixelFormat::PF_R32Sint,                   vk::Format::eR32Sint,                 },
        { PixelFormat::PF_R32Sfloat,                 vk::Format::eR32Sfloat,               },
        { PixelFormat::PF_R32G32Uint,                vk::Format::eR32G32Uint,              },
        { PixelFormat::PF_R32G32Sint,                vk::Format::eR32G32Sint,              },
        { PixelFormat::PF_R32G32Sfloat,              vk::Format::eR32G32Sfloat,            },
        { PixelFormat::PF_R32G32B32Uint,             vk::Format::eR32G32B32Uint,           },
        { PixelFormat::PF_R32G32B32Sint,             vk::Format::eR32G32B32Sint,           },
        { PixelFormat::PF_R32G32B32Sfloat,           vk::Format::eR32G32B32Sfloat,         },
        { PixelFormat::PF_R32G32B32A32Uint,          vk::Format::eR32G32B32A32Uint,        },
        { PixelFormat::PF_R32G32B32A32Sint,          vk::Format::eR32G32B32A32Sint,        },
        { PixelFormat::PF_R32G32B32A32Sfloat,        vk::Format::eR32G32B32A32Sfloat,      },
        { PixelFormat::PF_R64Uint,                   vk::Format::eR64Uint,                 },
        { PixelFormat::PF_R64Sint,                   vk::Format::eR64Sint,                 },
        { PixelFormat::PF_R64Sfloat,                 vk::Format::eR64Sfloat,               },
        { PixelFormat::PF_R64G64Uint,                vk::Format::eR64G64Uint,              },
        { PixelFormat::PF_R64G64Sint,                vk::Format::eR64G64Sint,              },
        { PixelFormat::PF_R64G64Sfloat,              vk::Format::eR64G64Sfloat,            },
        { PixelFormat::PF_R64G64B64Uint,             vk::Format::eR64G64B64Uint,           },
        { PixelFormat::PF_R64G64B64Sint,             vk::Format::eR64G64B64Sint,           },
        { PixelFormat::PF_R64G64B64Sfloat,           vk::Format::eR64G64B64Sfloat,         },
        { PixelFormat::PF_R64G64B64A64Uint,          vk::Format::eR64G64B64A64Uint,        },
        { PixelFormat::PF_R64G64B64A64Sint,          vk::Format::eR64G64B64A64Sint,        },
        { PixelFormat::PF_R64G64B64A64Sfloat,        vk::Format::eR64G64B64A64Sfloat,      },
        { PixelFormat::PF_B10G11R11UfloatPack32,     vk::Format::eB10G11R11UfloatPack32,   },
        { PixelFormat::PF_E5B9G9R9UfloatPack32,      vk::Format::eE5B9G9R9UfloatPack32,    },
        { PixelFormat::PF_D16Unorm,                  vk::Format::eD16Unorm,                },
        { PixelFormat::PF_X8D24UnormPack32,          vk::Format::eX8D24UnormPack32,        },
        { PixelFormat::PF_D32Sfloat,                 vk::Format::eD32Sfloat,               },
        { PixelFormat::PF_S8Uint,                    vk::Format::eS8Uint,                  },
        { PixelFormat::PF_D16UnormS8Uint,            vk::Format::eD16UnormS8Uint,          },
        { PixelFormat::PF_D24UnormS8Uint,            vk::Format::eD24UnormS8Uint,          },
        { PixelFormat::PF_D32SfloatS8Uint,           vk::Format::eD32SfloatS8Uint,         },
        { PixelFormat::PF_Bc1RgbUnormBlock,          vk::Format::eBc1RgbUnormBlock,        },
        { PixelFormat::PF_Bc1RgbSrgbBlock,           vk::Format::eBc1RgbSrgbBlock,         },
        { PixelFormat::PF_Bc1RgbaUnormBlock,         vk::Format::eBc1RgbaUnormBlock,       },
        { PixelFormat::PF_Bc1RgbaSrgbBlock,          vk::Format::eBc1RgbaSrgbBlock,        },
        { PixelFormat::PF_Bc2UnormBlock,             vk::Format::eBc2UnormBlock,           },
        { PixelFormat::PF_Bc2SrgbBlock,              vk::Format::eBc2SrgbBlock,            },
        { PixelFormat::PF_Bc3UnormBlock,             vk::Format::eBc3UnormBlock,           },
        { PixelFormat::PF_Bc3SrgbBlock,              vk::Format::eBc3SrgbBlock,            },
        { PixelFormat::PF_Bc4UnormBlock,             vk::Format::eBc4UnormBlock,           },
        { PixelFormat::PF_Bc4SnormBlock,             vk::Format::eBc4SnormBlock,           },
        { PixelFormat::PF_Bc5UnormBlock,             vk::Format::eBc5UnormBlock,           },
        { PixelFormat::PF_Bc5SnormBlock,             vk::Format::eBc5SnormBlock,           },
        { PixelFormat::PF_Bc6HUfloatBlock,           vk::Format::eBc6HUfloatBlock,         },
        { PixelFormat::PF_Bc6HSfloatBlock,           vk::Format::eBc6HSfloatBlock,         },
        { PixelFormat::PF_Bc7UnormBlock,             vk::Format::eBc7UnormBlock,           },
        { PixelFormat::PF_Bc7SrgbBlock,              vk::Format::eBc7SrgbBlock,            },
        { PixelFormat::PF_Etc2R8G8B8UnormBlock,      vk::Format::eEtc2R8G8B8UnormBlock,    },
        { PixelFormat::PF_Etc2R8G8B8SrgbBlock,       vk::Format::eEtc2R8G8B8SrgbBlock,     },
        { PixelFormat::PF_Etc2R8G8B8A1UnormBlock,    vk::Format::eEtc2R8G8B8A1UnormBlock,  },
        { PixelFormat::PF_Etc2R8G8B8A1SrgbBlock,     vk::Format::eEtc2R8G8B8A1SrgbBlock,   },
        { PixelFormat::PF_Etc2R8G8B8A8UnormBlock,    vk::Format::eEtc2R8G8B8A8UnormBlock,  },
        { PixelFormat::PF_Etc2R8G8B8A8SrgbBlock,     vk::Format::eEtc2R8G8B8A8SrgbBlock,   },
        { PixelFormat::PF_EacR11UnormBlock,          vk::Format::eEacR11UnormBlock,        },
        { PixelFormat::PF_EacR11SnormBlock,          vk::Format::eEacR11SnormBlock,        },
        { PixelFormat::PF_EacR11G11UnormBlock,       vk::Format::eEacR11G11UnormBlock,     },
        { PixelFormat::PF_EacR11G11SnormBlock,       vk::Format::eEacR11G11SnormBlock,     },
        { PixelFormat::PF_Astc4x4UnormBlock,         vk::Format::eAstc4x4UnormBlock,       },
        { PixelFormat::PF_Astc4x4SrgbBlock,          vk::Format::eAstc4x4SrgbBlock,        },
        { PixelFormat::PF_Astc5x4UnormBlock,         vk::Format::eAstc5x4UnormBlock,       },
        { PixelFormat::PF_Astc5x4SrgbBlock,          vk::Format::eAstc5x4SrgbBlock,        },
        { PixelFormat::PF_Astc5x5UnormBlock,         vk::Format::eAstc5x5UnormBlock,       },
        { PixelFormat::PF_Astc5x5SrgbBlock,          vk::Format::eAstc5x5SrgbBlock,        },
        { PixelFormat::PF_Astc6x5UnormBlock,         vk::Format::eAstc6x5UnormBlock,       },
        { PixelFormat::PF_Astc6x5SrgbBlock,          vk::Format::eAstc6x5SrgbBlock,        },
        { PixelFormat::PF_Astc6x6UnormBlock,         vk::Format::eAstc6x6UnormBlock,       },
        { PixelFormat::PF_Astc6x6SrgbBlock,          vk::Format::eAstc6x6SrgbBlock,        },
        { PixelFormat::PF_Astc8x5UnormBlock,         vk::Format::eAstc8x5UnormBlock,       },
        { PixelFormat::PF_Astc8x5SrgbBlock,          vk::Format::eAstc8x5SrgbBlock,        },
        { PixelFormat::PF_Astc8x6UnormBlock,         vk::Format::eAstc8x6UnormBlock,       },
        { PixelFormat::PF_Astc8x6SrgbBlock,          vk::Format::eAstc8x6SrgbBlock,        },
        { PixelFormat::PF_Astc8x8UnormBlock,         vk::Format::eAstc8x8UnormBlock,       },
        { PixelFormat::PF_Astc8x8SrgbBlock,          vk::Format::eAstc8x8SrgbBlock,        },
        { PixelFormat::PF_Astc10x5UnormBlock,        vk::Format::eAstc10x5UnormBlock,      },
        { PixelFormat::PF_Astc10x5SrgbBlock,         vk::Format::eAstc10x5SrgbBlock,       },
        { PixelFormat::PF_Astc10x6UnormBlock,        vk::Format::eAstc10x6UnormBlock,      },
        { PixelFormat::PF_Astc10x6SrgbBlock,         vk::Format::eAstc10x6SrgbBlock,       },
        { PixelFormat::PF_Astc10x8UnormBlock,        vk::Format::eAstc10x8UnormBlock,      },
        { PixelFormat::PF_Astc10x8SrgbBlock,         vk::Format::eAstc10x8SrgbBlock,       },
        { PixelFormat::PF_Astc10x10UnormBlock,       vk::Format::eAstc10x10UnormBlock,     },
        { PixelFormat::PF_Astc10x10SrgbBlock,        vk::Format::eAstc10x10SrgbBlock,      },
        { PixelFormat::PF_Astc12x10UnormBlock,       vk::Format::eAstc12x10UnormBlock,     },
        { PixelFormat::PF_Astc12x10SrgbBlock,        vk::Format::eAstc12x10SrgbBlock,      },
        { PixelFormat::PF_Astc12x12UnormBlock,       vk::Format::eAstc12x12UnormBlock,     },
        { PixelFormat::PF_Astc12x12SrgbBlock,        vk::Format::eAstc12x12SrgbBlock,      },
        //{ PixelFormat::PF_Depth16,                   vk::Format::eDepth16,                 },
        //{ PixelFormat::PF_Depth24,                   vk::Format::eDepth24,                 },
        //{ PixelFormat::PF_Depth32,                   vk::Format::eDepth32,                 },
    };

    vk::ImageViewType VKConverter::GetVulkanImageViewType(const ImageType& imageType)
    {
        switch (imageType)
        {
        case Glory::ImageType::IT_1D:
            return vk::ImageViewType::e1D;
        case Glory::ImageType::IT_2D:
            return vk::ImageViewType::e2D;
        case Glory::ImageType::IT_3D:
            return vk::ImageViewType::e3D;
        case Glory::ImageType::IT_Cube:
            return vk::ImageViewType::eCube;
        case Glory::ImageType::IT_1DArray:
            return vk::ImageViewType::e1DArray;
        case Glory::ImageType::IT_2DArray:
            return vk::ImageViewType::e2DArray;
        case Glory::ImageType::IT_CubeArray:
            return vk::ImageViewType::eCubeArray;
        }

        return (vk::ImageViewType)0;
    }

    vk::ImageType VKConverter::GetVulkanImageType(const ImageType& imageType)
    {
        switch (imageType)
        {
        case Glory::ImageType::IT_1D:
            return vk::ImageType::e1D;
        case Glory::ImageType::IT_2D:
            return vk::ImageType::e2D;
        case Glory::ImageType::IT_3D:
            return vk::ImageType::e3D;
        }

        return (vk::ImageType)0;
    }

    vk::ImageUsageFlags VKConverter::GetVulkanImageUsageFlags(const ImageAspect& aspectFlags)
    {
        vk::ImageUsageFlags flags = vk::ImageUsageFlags();
        if ((aspectFlags & ImageAspect::IA_Color) > 0) flags |= vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
        if ((aspectFlags & ImageAspect::IA_Depth) > 0) flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
        if ((aspectFlags & ImageAspect::IA_Stencil) > 0) flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
        return flags;
    }

    vk::ImageAspectFlags VKConverter::GetVulkanImageAspectFlags(const ImageAspect& aspectFlags)
    {
        vk::ImageAspectFlags flags = vk::ImageAspectFlags();

        if ((aspectFlags & ImageAspect::IA_Color) > 0) flags |= vk::ImageAspectFlagBits::eColor;
        if ((aspectFlags & ImageAspect::IA_Depth) > 0) flags |= vk::ImageAspectFlagBits::eDepth;
        if ((aspectFlags & ImageAspect::IA_Metadata) > 0) flags |= vk::ImageAspectFlagBits::eMetadata;
        if ((aspectFlags & ImageAspect::IA_Plane0) > 0) flags |= vk::ImageAspectFlagBits::ePlane0;
        if ((aspectFlags & ImageAspect::IA_Plane1) > 0) flags |= vk::ImageAspectFlagBits::ePlane1;
        if ((aspectFlags & ImageAspect::IA_Plane2) > 0) flags |= vk::ImageAspectFlagBits::ePlane2;
        if ((aspectFlags & ImageAspect::IA_Plane0KHR) > 0) flags |= vk::ImageAspectFlagBits::ePlane0KHR;
        if ((aspectFlags & ImageAspect::IA_Plane1KHR) > 0) flags |= vk::ImageAspectFlagBits::ePlane1KHR;
        if ((aspectFlags & ImageAspect::IA_Plane2KHR) > 0) flags |= vk::ImageAspectFlagBits::ePlane2KHR;
        if ((aspectFlags & ImageAspect::IA_MemoryPlane0EXT) > 0) flags |= vk::ImageAspectFlagBits::eMemoryPlane0EXT;
        if ((aspectFlags & ImageAspect::IA_MemoryPlane1EXT) > 0) flags |= vk::ImageAspectFlagBits::eMemoryPlane1EXT;
        if ((aspectFlags & ImageAspect::IA_MemoryPlane2EXT) > 0) flags |= vk::ImageAspectFlagBits::eMemoryPlane2EXT;
        if ((aspectFlags & ImageAspect::IA_MemoryPlane3EXT) > 0) flags |= vk::ImageAspectFlagBits::eMemoryPlane3EXT;
        if ((aspectFlags & ImageAspect::IA_Stencil) > 0) flags |= vk::ImageAspectFlagBits::eStencil;

        return flags;
    }

    vk::Filter VKConverter::GetVulkanFilter(const Filter& filter)
    {
        switch (filter)
        {
        case Glory::Filter::F_Nearest:
            return vk::Filter::eNearest;
        case Glory::Filter::F_Linear:
            return vk::Filter::eLinear;
        case Glory::Filter::F_CubicIMG:
            return vk::Filter::eCubicIMG;
        case Glory::Filter::F_CubicEXT:
            return vk::Filter::eCubicEXT;
        }

        return (vk::Filter)0;
    }

    vk::CompareOp VKConverter::GetVulkanCompareOp(const CompareOp& op)
    {
        switch (op)
        {
        case Glory::CompareOp::OP_Never:
            return vk::CompareOp::eNever;
        case Glory::CompareOp::OP_Less:
            return vk::CompareOp::eLess;
        case Glory::CompareOp::OP_Equal:
            return vk::CompareOp::eEqual;
        case Glory::CompareOp::OP_LessOrEqual:
            return vk::CompareOp::eLessOrEqual;
        case Glory::CompareOp::OP_Greater:
            return vk::CompareOp::eGreater;
        case Glory::CompareOp::OP_NotEqual:
            return vk::CompareOp::eNotEqual;
        case Glory::CompareOp::OP_GreaterOrEqual:
            return vk::CompareOp::eGreaterOrEqual;
        case Glory::CompareOp::OP_Always:
            return vk::CompareOp::eAlways;
        }
        return (vk::CompareOp)0;
    }

    vk::SamplerMipmapMode VKConverter::GetVulkanSamplerMipmapMode(const Filter& filter)
    {
        switch (filter)
        {
        case Glory::Filter::F_Nearest:
            return vk::SamplerMipmapMode::eNearest;
        case Glory::Filter::F_Linear:
            vk::SamplerMipmapMode::eLinear;
        }

        return (vk::SamplerMipmapMode)0;
    }

    vk::SamplerAddressMode VKConverter::GetSamplerAddressMode(const SamplerAddressMode& mode)
    {
        switch (mode)
        {
        case SamplerAddressMode::SAM_Repeat:
            return vk::SamplerAddressMode::eRepeat;
        case SamplerAddressMode::SAM_MirroredRepeat:
            return vk::SamplerAddressMode::eMirroredRepeat;
        case SamplerAddressMode::SAM_ClampToEdge:
            return vk::SamplerAddressMode::eClampToEdge;
        case SamplerAddressMode::SAM_ClampToBorder:
            return vk::SamplerAddressMode::eClampToBorder;
        case SamplerAddressMode::SAM_MirrorClampToEdge:
            return vk::SamplerAddressMode::eMirrorClampToEdge;
        case SamplerAddressMode::SAM_MirrorClampToEdgeKHR:
            return vk::SamplerAddressMode::eMirrorClampToEdgeKHR;
        }

        return (vk::SamplerAddressMode)0;
    }

    vk::SamplerCreateInfo VKConverter::GetVulkanSamplerInfo(const SamplerSettings& settings)
    {
        vk::SamplerCreateInfo samplerInfo = vk::SamplerCreateInfo();
        samplerInfo.magFilter = GetVulkanFilter(settings.MagFilter);
        samplerInfo.minFilter = GetVulkanFilter(settings.MinFilter);
        samplerInfo.addressModeU = GetSamplerAddressMode(settings.AddressModeU);
        samplerInfo.addressModeV = GetSamplerAddressMode(settings.AddressModeV);
        samplerInfo.addressModeW = GetSamplerAddressMode(settings.AddressModeW);
        samplerInfo.anisotropyEnable = settings.AnisotropyEnable ? VK_TRUE : VK_FALSE;
        samplerInfo.maxAnisotropy = settings.MaxAnisotropy;
        samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
        samplerInfo.unnormalizedCoordinates = settings.UnnormalizedCoordinates ? VK_TRUE : VK_FALSE;
        samplerInfo.compareEnable = settings.CompareEnable ? VK_TRUE : VK_FALSE;
        samplerInfo.compareOp = GetVulkanCompareOp(settings.CompareOp);
        samplerInfo.mipmapMode = GetVulkanSamplerMipmapMode(settings.MipmapMode);
        samplerInfo.mipLodBias = settings.MipLODBias;
        samplerInfo.minLod = settings.MinLOD;
        samplerInfo.maxLod = settings.MaxLOD;
        return samplerInfo;
    }

    vk::Format VKConverter::GetVulkanFormat(const PixelFormat& format)
    {
        auto iter = TO_VKFORMAT.find(format);
        return iter == TO_VKFORMAT.end() ? (vk::Format)format : iter->second;
    }

    vk::ShaderStageFlagBits VKConverter::GetShaderStageFlag(const ShaderType& shaderType)
    {
        switch (shaderType)
        {
        case ShaderType::ST_Vertex:
            return vk::ShaderStageFlagBits::eVertex;
        case ShaderType::ST_Fragment:
            return vk::ShaderStageFlagBits::eFragment;
        case ShaderType::ST_Geomtery:
            return vk::ShaderStageFlagBits::eGeometry;
        case ShaderType::ST_TessControl:
            return vk::ShaderStageFlagBits::eTessellationControl;
        case ShaderType::ST_TessEval:
            return vk::ShaderStageFlagBits::eTessellationEvaluation;
        case ShaderType::ST_Compute:
            return vk::ShaderStageFlagBits::eCompute;
        }

        return vk::ShaderStageFlagBits();
    }

    vk::BufferUsageFlags VKConverter::ToBufferUsageFlags(const BufferBindingTarget& bindingTarget)
    {
        switch (bindingTarget)
        {
            case BufferBindingTarget::B_ARRAY:
                return vk::BufferUsageFlagBits::eVertexBuffer;
            case BufferBindingTarget::B_ATOMIC_COUNTER:
                return vk::BufferUsageFlags(0);
            case BufferBindingTarget::B_COPY_READ:
                return vk::BufferUsageFlagBits::eTransferSrc;
            case BufferBindingTarget::B_COPY_WRITE:
                return vk::BufferUsageFlagBits::eTransferDst;
            case BufferBindingTarget::B_DISPATCH_INDIRECT:
                return vk::BufferUsageFlagBits::eIndexBuffer;
            case BufferBindingTarget::B_DRAW_INDIRECT:
                return vk::BufferUsageFlagBits::eIndexBuffer;
            case BufferBindingTarget::B_ELEMENT_ARRAY:
                return vk::BufferUsageFlagBits::eIndexBuffer;
            case BufferBindingTarget::B_PIXEL_PACK:
                return vk::BufferUsageFlagBits::eStorageTexelBuffer;
            case BufferBindingTarget::B_PIXEL_UNPACK:
                return vk::BufferUsageFlagBits::eUniformTexelBuffer;
            case BufferBindingTarget::B_QUERY:
                return vk::BufferUsageFlags(0);
            case BufferBindingTarget::B_SHADER_STORAGE:
                return vk::BufferUsageFlagBits::eStorageBuffer;
            case BufferBindingTarget::B_TEXTURE:
                return vk::BufferUsageFlags(0);
            case BufferBindingTarget::B_TRANSFORM_FEEDBACK:
                return vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT;
            case BufferBindingTarget::B_UNIFORM:
                return vk::BufferUsageFlagBits::eUniformBuffer;
        default:
            break;
        }
    }
}
