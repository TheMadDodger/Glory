ProjectDir = "../%{SubmodoleDirs.assimp}"

os.copyfile("config/config.h", "../../submodules/assimp/include/assimp/config.h")
--os.copyfile("config/zconf.h", "../../submodules/assimp/contrib/zlib/zconf.h")
os.copyfile("config/revision.h", "../../submodules/assimp/revision.h")

project "assimp"
	location "%{ProjectDir}"
	kind "SharedLib"
	language "C++"


	targetdir ("%{ProjectDir}/lib/%{outputdir}")
	objdir ("%{ProjectDir}/%{outputdir}")

	files
	{
		"%{ProjectDir}/include/**",
        "%{ProjectDir}/code/common/Assimp.cpp",
        "%{ProjectDir}/code/common/AssertHandler.h",
        "%{ProjectDir}/code/common/AssertHandler.cpp",
        "%{ProjectDir}/code/common/Exceptional.cpp",
        "%{ProjectDir}/code/common/BaseImporter.cpp",
        "%{ProjectDir}/code/common/BaseProcess.cpp",
        "%{ProjectDir}/code/common/Importer.cpp",
        "%{ProjectDir}/code/common/ImporterRegistry.cpp",
        "%{ProjectDir}/code/common/CreateAnimMesh.cpp",
        "%{ProjectDir}/code/common/scene.cpp",
        "%{ProjectDir}/code/common/DefaultIOStream.cpp",
        "%{ProjectDir}/code/common/DefaultIOSystem.cpp",
        "%{ProjectDir}/code/common/DefaultLogger.cpp",
        "%{ProjectDir}/code/common/PostStepRegistry.cpp",
        "%{ProjectDir}/code/common/ScenePreprocessor.cpp",
        "%{ProjectDir}/code/common/SGSpatialSort.cpp",
        "%{ProjectDir}/code/common/SpatialSort.cpp",
        "%{ProjectDir}/code/common/SkeletonMeshBuilder.cpp",
        "%{ProjectDir}/code/common/Version.cpp",
        "%{ProjectDir}/code/common/VertexTriangleAdjacency.cpp",
        "%{ProjectDir}/code/common/material.cpp",
        "%{ProjectDir}/code/CApi/CInterfaceIOWrapper.cpp",
        "%{ProjectDir}/code/AssetLib/Obj/ObjFileImporter.cpp",
        "%{ProjectDir}/code/AssetLib/Obj/ObjFileMtlImporter.cpp",
        "%{ProjectDir}/code/AssetLib/Obj/ObjFileParser.cpp",
        "%{ProjectDir}/code/AssetLib/FBX/FBXImporter.cpp",
        "%{ProjectDir}/code/AssetLib/FBX/FBXConverter.cpp",
        "%{ProjectDir}/code/AssetLib/FBX/FBXDocument.cpp",
        "%{ProjectDir}/code/AssetLib/FBX/FBXParser.cpp",
        "%{ProjectDir}/code/AssetLib/FBX/FBXTokenizer.cpp",
        "%{ProjectDir}/code/AssetLib/FBX/FBXUtil.cpp",
        "%{ProjectDir}/code/AssetLib/FBX/FBXProperties.cpp",
        "%{ProjectDir}/code/AssetLib/FBX/FBXMeshGeometry.cpp",
        "%{ProjectDir}/code/AssetLib/FBX/FBXDocumentUtil.cpp",
        "%{ProjectDir}/code/AssetLib/FBX/FBXAnimation.cpp",
        "%{ProjectDir}/code/AssetLib/FBX/FBXModel.cpp",
        "%{ProjectDir}/code/AssetLib/FBX/FBXMaterial.cpp",
        "%{ProjectDir}/code/AssetLib/FBX/FBXNodeAttribute.cpp",
        "%{ProjectDir}/code/AssetLib/FBX/FBXDeformer.cpp",
        "%{ProjectDir}/code/AssetLib/FBX/FBXBinaryTokenizer.cpp",
        "%{ProjectDir}/code/PostProcessing/ArmaturePopulate.cpp",
        "%{ProjectDir}/code/PostProcessing/GenBoundingBoxesProcess.cpp",
        "%{ProjectDir}/code/PostProcessing/EmbedTexturesProcess.cpp",
        "%{ProjectDir}/code/PostProcessing/ConvertToLHProcess.cpp",
        "%{ProjectDir}/code/PostProcessing/GenVertexNormalsProcess.cpp",
        "%{ProjectDir}/code/PostProcessing/ProcessHelper.cpp",
        "%{ProjectDir}/code/PostProcessing/ScaleProcess.cpp",
        "%{ProjectDir}/code/PostProcessing/TriangulateProcess.cpp",
        "%{ProjectDir}/code/PostProcessing/CalcTangentsProcess.cpp",
        "%{ProjectDir}/code/PostProcessing/ScaleProcess.cpp",
        "%{ProjectDir}/code/PostProcessing/EmbedTexturesProcess.cpp",
        "%{ProjectDir}/code/PostProcessing/ValidateDataStructure.cpp",
        "%{ProjectDir}/code/PostProcessing/MakeVerboseFormat.cpp",
        "%{ProjectDir}/code/Material/MaterialSystem.cpp",
        "%{ProjectDir}/contrib/irrXML/*",
	}

	sysincludedirs
    {
        "%{ProjectDir}",
        "%{ProjectDir}/include",
        "%{ProjectDir}/contrib",
        "%{ProjectDir}/contrib/irrXML",
        "%{ProjectDir}/contrib/zlib",
        "%{ProjectDir}/contrib/rapidjson/include",
    }
        
    includedirs
    {
        "%{ProjectDir}",
        "%{ProjectDir}/code",
        "%{ProjectDir}/include",
        "%{ProjectDir}/contrib",
        "%{ProjectDir}/contrib/irrXML",
        "%{ProjectDir}/contrib/zlib",
        "%{ProjectDir}/contrib/rapidjson/include",
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        -- "SWIG",
        "ASSIMP_BUILD_NO_OWN_ZLIB",

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
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"