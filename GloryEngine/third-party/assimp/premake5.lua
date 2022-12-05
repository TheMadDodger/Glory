ASSIMPDir = "%{SubmodoleDirs.assimp}"

os.copyfile("config/config.h", "%{rootDir}/submodules/assimp/include/assimp/config.h")
os.copyfile("config/zconf.h", "%{rootDir}/submodules/assimp/contrib/zlib/zconf.h")
os.copyfile("config/revision.h", "%{rootDir}/submodules/assimp/revision.h")

project "assimpstatic"
	location "%{ASSIMPDir}"
    kind "StaticLib"
	language "C++"
	targetname "assimp"

	targetdir ("%{ASSIMPDir}/lib/%{outputDir}")
	objdir ("%{ASSIMPDir}/%{outputDir}")

	files
    {
	    "%{ASSIMPDir}/include/**",
        "%{ASSIMPDir}/code/common/Assimp.cpp",
        "%{ASSIMPDir}/code/common/AssertHandler.h",
        "%{ASSIMPDir}/code/common/AssertHandler.cpp",
        "%{ASSIMPDir}/code/common/Exceptional.cpp",
        "%{ASSIMPDir}/code/common/BaseImporter.cpp",
        "%{ASSIMPDir}/code/common/BaseProcess.cpp",
        "%{ASSIMPDir}/code/common/Importer.cpp",
        "%{ASSIMPDir}/code/common/ImporterRegistry.cpp",
        "%{ASSIMPDir}/code/common/CreateAnimMesh.cpp",
        "%{ASSIMPDir}/code/common/scene.cpp",
        "%{ASSIMPDir}/code/common/DefaultIOStream.cpp",
        "%{ASSIMPDir}/code/common/DefaultIOSystem.cpp",
        "%{ASSIMPDir}/code/common/DefaultLogger.cpp",
        "%{ASSIMPDir}/code/common/PostStepRegistry.cpp",
        "%{ASSIMPDir}/code/common/ScenePreprocessor.cpp",
        "%{ASSIMPDir}/code/common/SGSpatialSort.cpp",
        "%{ASSIMPDir}/code/common/SpatialSort.cpp",
        "%{ASSIMPDir}/code/common/SkeletonMeshBuilder.cpp",
        "%{ASSIMPDir}/code/common/Version.cpp",
        "%{ASSIMPDir}/code/common/VertexTriangleAdjacency.cpp",
        "%{ASSIMPDir}/code/common/material.cpp",
        "%{ASSIMPDir}/code/common/IOSystem.cpp",
        "%{ASSIMPDir}/code/common/Compression.cpp",
        "%{ASSIMPDir}/code/CApi/CInterfaceIOWrapper.cpp",
        "%{ASSIMPDir}/code/AssetLib/Obj/ObjFileImporter.cpp",
        "%{ASSIMPDir}/code/AssetLib/Obj/ObjFileMtlImporter.cpp",
        "%{ASSIMPDir}/code/AssetLib/Obj/ObjFileParser.cpp",
        "%{ASSIMPDir}/code/AssetLib/FBX/FBXImporter.cpp",
        "%{ASSIMPDir}/code/AssetLib/FBX/FBXConverter.cpp",
        "%{ASSIMPDir}/code/AssetLib/FBX/FBXDocument.cpp",
        "%{ASSIMPDir}/code/AssetLib/FBX/FBXParser.cpp",
        "%{ASSIMPDir}/code/AssetLib/FBX/FBXTokenizer.cpp",
        "%{ASSIMPDir}/code/AssetLib/FBX/FBXUtil.cpp",
        "%{ASSIMPDir}/code/AssetLib/FBX/FBXProperties.cpp",
        "%{ASSIMPDir}/code/AssetLib/FBX/FBXMeshGeometry.cpp",
        "%{ASSIMPDir}/code/AssetLib/FBX/FBXDocumentUtil.cpp",
        "%{ASSIMPDir}/code/AssetLib/FBX/FBXAnimation.cpp",
        "%{ASSIMPDir}/code/AssetLib/FBX/FBXModel.cpp",
        "%{ASSIMPDir}/code/AssetLib/FBX/FBXMaterial.cpp",
        "%{ASSIMPDir}/code/AssetLib/FBX/FBXNodeAttribute.cpp",
        "%{ASSIMPDir}/code/AssetLib/FBX/FBXDeformer.cpp",
        "%{ASSIMPDir}/code/AssetLib/FBX/FBXBinaryTokenizer.cpp",
        "%{ASSIMPDir}/code/PostProcessing/ArmaturePopulate.cpp",
        "%{ASSIMPDir}/code/PostProcessing/GenBoundingBoxesProcess.cpp",
        "%{ASSIMPDir}/code/PostProcessing/EmbedTexturesProcess.cpp",
        "%{ASSIMPDir}/code/PostProcessing/ConvertToLHProcess.cpp",
        "%{ASSIMPDir}/code/PostProcessing/GenVertexNormalsProcess.cpp",
        "%{ASSIMPDir}/code/PostProcessing/ProcessHelper.cpp",
        "%{ASSIMPDir}/code/PostProcessing/ScaleProcess.cpp",
        "%{ASSIMPDir}/code/PostProcessing/TriangulateProcess.cpp",
        "%{ASSIMPDir}/code/PostProcessing/CalcTangentsProcess.cpp",
        "%{ASSIMPDir}/code/PostProcessing/ScaleProcess.cpp",
        "%{ASSIMPDir}/code/PostProcessing/EmbedTexturesProcess.cpp",
        "%{ASSIMPDir}/code/PostProcessing/ValidateDataStructure.cpp",
        "%{ASSIMPDir}/code/PostProcessing/MakeVerboseFormat.cpp",
        "%{ASSIMPDir}/code/Material/MaterialSystem.cpp",

        "%{ASSIMPDir}/contrib/zlib/*.h",
        "%{ASSIMPDir}/contrib/zlib/*.c",
    }

    sysincludedirs
    {
        "%{ASSIMPDir}",
        "%{ASSIMPDir}/include",
        "%{ASSIMPDir}/contrib",
        "%{ASSIMPDir}/contrib/irrXML",
        "%{ASSIMPDir}/contrib/zlib",
        "%{ASSIMPDir}/contrib/rapidjson/include",
    }
        
    includedirs
    {
        "%{ASSIMPDir}",
        "%{ASSIMPDir}/code",
        "%{ASSIMPDir}/include",
        "%{ASSIMPDir}/contrib",
        "%{ASSIMPDir}/contrib/irrXML",
        "%{ASSIMPDir}/contrib/zlib",
        "%{ASSIMPDir}/contrib/rapidjson/include",
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        -- "SWIG",
        --"ASSIMP_BUILD_NO_OWN_ZLIB",
        "ASSIMP_BUILD_NO_M3D_IMPORTER",
        "ASSIMP_BUILD_NO_X_IMPORTER",
        "ASSIMP_BUILD_NO_3DS_IMPORTER",
        "ASSIMP_BUILD_NO_MD3_IMPORTER",
        "ASSIMP_BUILD_NO_MDL_IMPORTER",
        "ASSIMP_BUILD_NO_MD2_IMPORTER",
        "ASSIMP_BUILD_NO_PLY_IMPORTER",
        "ASSIMP_BUILD_NO_ASE_IMPORTER",
        -- "ASSIMP_BUILD_NO_OBJ_IMPORTER",
        "ASSIMP_BUILD_NO_AMF_IMPORTER",
        "ASSIMP_BUILD_NO_HMP_IMPORTER",
        "ASSIMP_BUILD_NO_SMD_IMPORTER",
        "ASSIMP_BUILD_NO_MDC_IMPORTER",
        "ASSIMP_BUILD_NO_MD5_IMPORTER",
        "ASSIMP_BUILD_NO_STL_IMPORTER",
        "ASSIMP_BUILD_NO_LWO_IMPORTER",
        "ASSIMP_BUILD_NO_DXF_IMPORTER",
        "ASSIMP_BUILD_NO_NFF_IMPORTER",
        "ASSIMP_BUILD_NO_RAW_IMPORTER",
        "ASSIMP_BUILD_NO_OFF_IMPORTER",
        "ASSIMP_BUILD_NO_AC_IMPORTER",
        "ASSIMP_BUILD_NO_BVH_IMPORTER",
        "ASSIMP_BUILD_NO_IRRMESH_IMPORTER",
        "ASSIMP_BUILD_NO_IRR_IMPORTER",
        "ASSIMP_BUILD_NO_Q3D_IMPORTER",
        "ASSIMP_BUILD_NO_B3D_IMPORTER",
        "ASSIMP_BUILD_NO_COLLADA_IMPORTER",
        "ASSIMP_BUILD_NO_TERRAGEN_IMPORTER",
        "ASSIMP_BUILD_NO_CSM_IMPORTER",
        "ASSIMP_BUILD_NO_3D_IMPORTER",
        "ASSIMP_BUILD_NO_LWS_IMPORTER",
        "ASSIMP_BUILD_NO_OGRE_IMPORTER",
        "ASSIMP_BUILD_NO_OPENGEX_IMPORTER",
        "ASSIMP_BUILD_NO_MS3D_IMPORTER",
        "ASSIMP_BUILD_NO_COB_IMPORTER",
        "ASSIMP_BUILD_NO_BLEND_IMPORTER",
        "ASSIMP_BUILD_NO_Q3BSP_IMPORTER",
        "ASSIMP_BUILD_NO_NDO_IMPORTER",
        "ASSIMP_BUILD_NO_IFC_IMPORTER",
        "ASSIMP_BUILD_NO_XGL_IMPORTER",
        -- "ASSIMP_BUILD_NO_FBX_IMPORTER",
        "ASSIMP_BUILD_NO_ASSBIN_IMPORTER",
        "ASSIMP_BUILD_NO_GLTF_IMPORTER",
        "ASSIMP_BUILD_NO_GLTF1_IMPORTER",
        "ASSIMP_BUILD_NO_GLTF2_IMPORTER",
        "ASSIMP_BUILD_NO_C4D_IMPORTER",
        "ASSIMP_BUILD_NO_3MF_IMPORTER",
        "ASSIMP_BUILD_NO_X3D_IMPORTER",
        "ASSIMP_BUILD_NO_MMD_IMPORTER",
        
        "ASSIMP_BUILD_NO_STEP_EXPORTER",
        "ASSIMP_BUILD_NO_SIB_IMPORTER",
        "ASSIMP_BUILD_NO_IQM_IMPORTER",
        "ASSIMP_BUILD_NO_IQM_EXPORTER",

        -- "ASSIMP_BUILD_NO_MAKELEFTHANDED_PROCESS",
        -- "ASSIMP_BUILD_NO_FLIPUVS_PROCESS",
        -- "ASSIMP_BUILD_NO_FLIPWINDINGORDER_PROCESS",
        -- "ASSIMP_BUILD_NO_CALCTANGENTS_PROCESS",
        "ASSIMP_BUILD_NO_JOINVERTICES_PROCESS",
        -- "ASSIMP_BUILD_NO_TRIANGULATE_PROCESS",
        "ASSIMP_BUILD_NO_GENFACENORMALS_PROCESS",
        -- "ASSIMP_BUILD_NO_GENVERTEXNORMALS_PROCESS",
        "ASSIMP_BUILD_NO_REMOVEVC_PROCESS",
        "ASSIMP_BUILD_NO_SPLITLARGEMESHES_PROCESS",
        "ASSIMP_BUILD_NO_PRETRANSFORMVERTICES_PROCESS",
        "ASSIMP_BUILD_NO_LIMITBONEWEIGHTS_PROCESS",
        -- "ASSIMP_BUILD_NO_VALIDATEDS_PROCESS",
        "ASSIMP_BUILD_NO_IMPROVECACHELOCALITY_PROCESS",
        "ASSIMP_BUILD_NO_FIXINFACINGNORMALS_PROCESS",
        "ASSIMP_BUILD_NO_REMOVE_REDUNDANTMATERIALS_PROCESS",
        "ASSIMP_BUILD_NO_FINDINVALIDDATA_PROCESS",
        "ASSIMP_BUILD_NO_FINDDEGENERATES_PROCESS",
        "ASSIMP_BUILD_NO_SORTBYPTYPE_PROCESS",
        "ASSIMP_BUILD_NO_GENUVCOORDS_PROCESS",
        "ASSIMP_BUILD_NO_TRANSFORMTEXCOORDS_PROCESS",
        "ASSIMP_BUILD_NO_FINDINSTANCES_PROCESS",
        "ASSIMP_BUILD_NO_OPTIMIZEMESHES_PROCESS",
        "ASSIMP_BUILD_NO_OPTIMIZEGRAPH_PROCESS",
        "ASSIMP_BUILD_NO_SPLITBYBONECOUNT_PROCESS",
        "ASSIMP_BUILD_NO_DEBONE_PROCESS",
        --"ASSIMP_BUILD_NO_EMBEDTEXTURES_PROCESS",
        --"ASSIMP_BUILD_NO_GLOBALSCALE_PROCESS",
    }

    filter "system:windows"
	    systemversion "latest"
	    cppdialect "C++17"
	    staticruntime "off"

    filter "system:linux"
	    pic "On"
	    systemversion "latest"
	    cppdialect "C++17"
	    staticruntime "off"

    filter "configurations:Debug"
	    runtime "Debug"
	    defines "_DEBUG"
	    symbols "On"

    filter "configurations:Release"
	    runtime "Release"
	    defines "NDEBUG"
	    optimize "On"
