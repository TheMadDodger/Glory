#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ModelData.h>
#include <MaterialData.h>
#include <Camera.h>
//#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <CameraRef.h>
#include <Layer.h>
#include <LightData.h>
#include <Script.h>
#include "Entity.h"

#include <AssetReference.h>
#include <LayerRef.h>
#include <Reflection.h>
#include <map>

REFLECTABLE_ENUM(CameraPerspective, Orthographic, Perspective)

#define PROPERTY_BUFFER_SIZE 2048;

namespace Glory
{
	struct Transform
	{
		GLORY_API Transform();
		GLORY_API Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);

		REFLECTABLE(Transform,
			(glm::vec3)	(Position),
			(glm::quat)	(Rotation),
			(glm::vec3) (Scale)
		)

		glm::mat4 MatTransform;

		Entity Parent;
	};

	struct MeshFilter
	{
		MeshFilter() : m_pModelData(0) {}
		MeshFilter(ModelData* pModelData) : m_pModelData(pModelData != nullptr ? pModelData->GetUUID() : 0) {}
		
		REFLECTABLE(MeshFilter,
			(AssetReference<ModelData>)	(m_pModelData)
		)
	};
	
	struct MeshMaterial
	{
		MeshMaterial() : m_MaterialReference(0) {}
		MeshMaterial(UUID uuid) : m_MaterialReference(uuid) {}

		bool operator==(const MeshMaterial&& other)
		{
			return other.m_MaterialReference.AssetUUID() == other.m_MaterialReference.AssetUUID();
		}

		bool operator==(MeshMaterial& other)
		{
			return other.m_MaterialReference.AssetUUID() == other.m_MaterialReference.AssetUUID();
		}

		REFLECTABLE(MeshMaterial,
			(AssetReference<MaterialData>) (m_MaterialReference)
		)
	};

	struct MeshRenderer
	{
		MeshRenderer(MaterialData* pMaterial) : m_pMaterials({ pMaterial != nullptr ? pMaterial->GetUUID() : 0 }) {}
		MeshRenderer() : m_pMaterials(std::vector<MeshMaterial>()) {}

		REFLECTABLE(MeshRenderer,
			(std::vector<MeshMaterial>)	(m_pMaterials)
		)
	};
	
	struct CameraComponent
	{
		GLORY_API CameraComponent() : m_HalfFOV(45.0f), m_Near(0.1f), m_Far(10.0f), m_DisplayIndex(0), m_Priority(0), m_ClearColor(glm::vec4(0.0f)), m_LayerMask(0), m_LastHash(0) {}
		GLORY_API CameraComponent(float halfFOV, float near, float far, int displayIndex = 0, int priority = 0, const glm::vec4& clearColor = glm::vec4(0.0f))
			: m_HalfFOV(halfFOV), m_Near(near), m_Far(far), m_DisplayIndex(displayIndex), m_Priority(priority), m_ClearColor(clearColor), m_LayerMask(0), m_LastHash(0) {}
		
		REFLECTABLE(CameraComponent,
			(float)	(m_HalfFOV),
			(float)	(m_Near),
			(float)	(m_Far),
			(int)	(m_DisplayIndex),
			(int)	(m_Priority),
			(LayerMask)	(m_LayerMask),
			(glm::vec4)	(m_ClearColor)
		)
	
		size_t m_LastHash;
	
		CameraRef m_Camera;
	};
	
	struct LookAt
	{
		LookAt() : m_Eye(0.0f), m_Center(0.0f), m_Up({0.0f, 1.0f, 0.0f}) {}
		LookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up) : m_Eye(eye), m_Center(center), m_Up(up) {}
	
		REFLECTABLE(LookAt,
			(glm::vec3)	(m_Eye),
			(glm::vec3)	(m_Center),
			(glm::vec3)	(m_Up)
		)
	};
	
	struct Spin
	{
		Spin() : m_Speed(10.0f), m_Time(0.0f) {}
		Spin(float speed) : m_Speed(speed), m_Time(0.0f) {}

		REFLECTABLE(Spin,
			(float)	(m_Speed),
			(float)	(m_Time)
		)
	};
	
	struct LayerComponent
	{
		LayerComponent() : m_Layer("") {}
		LayerComponent(const Layer* pLayer) : m_Layer(pLayer->m_Name) {}

		REFLECTABLE(LayerComponent,
			(LayerRef)	(m_Layer)
		)
	};
	
	struct LightComponent
	{
		LightComponent() : m_Color(1.0f), m_Intensity(10.0f), m_Range(100.0f) {}
		LightComponent(const glm::vec4& color, float intensity, float range) : m_Color(color), m_Intensity(intensity), m_Range(range) {}

		REFLECTABLE(LightComponent,
			(glm::vec4)	(m_Color),
			(float)	(m_Intensity),
			(float)	(m_Range)
		)
	};

	struct ScriptedComponent
	{
		ScriptedComponent() : m_Script(0), m_ScriptData() {}
		ScriptedComponent(Script* pScript) : m_Script(pScript != nullptr ? pScript->GetUUID() : 0), m_ScriptData() {}

		// Script data MUST be at offset 0 otherwise undo/redo actions will read the wrong address
		YAML::Node m_ScriptData;

		REFLECTABLE(ScriptedComponent,
			(AssetReference<Script>) (m_Script)
		);

		std::vector<ScriptProperty> m_ScriptProperties;
		std::vector<ScriptProperty> m_ScriptChildProperties;
	};

	//ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//
	//int width, height;
	//pWindow->GetDrawableSize(&width, &height);

	//ubo.proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 10.0f);
	//ubo.proj[1][1] *= -1; // In OpenGL the Y coordinate of the clip coordinates is inverted, so we must flip it for use in Vulkan
}
