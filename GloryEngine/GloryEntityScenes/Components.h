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
#include "Entity.h"

namespace Glory
{
	struct Transform
	{
		Transform();
		Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);

		glm::vec3 Position;
		glm::quat Rotation;
		glm::vec3 Scale;

		glm::mat4 MatTransform;

		Entity Parent;
	};

	struct MeshFilter
	{
		MeshFilter() : m_pModelData(0) {}
		MeshFilter(ModelData* pModelData) : m_pModelData(pModelData != nullptr ? pModelData->GetUUID() : 0) {}
		UUID m_pModelData;
	};

	struct MeshRenderer
	{
		MeshRenderer(MaterialData* pMaterial) : m_pMaterials({ pMaterial != nullptr ? pMaterial->GetUUID() : 0 }) {}
		MeshRenderer() : m_pMaterials(std::vector<UUID>()) {}
		std::vector<UUID> m_pMaterials;
	};

	//enum CameraPerspective
	//{
	//	Orthographic,
	//	Perspective,
	//};

	struct CameraComponent
	{
		CameraComponent() : m_HalfFOV(45.0f), m_Near(0.1f), m_Far(10.0f), m_DisplayIndex(0), m_Priority(0), m_ClearColor(glm::vec4(0.0f)), m_LayerMask(0), m_LastHash(0) {}
		CameraComponent(float halfFOV, float near, float far, int displayIndex = 0, int priority = 0, const glm::vec4& clearColor = glm::vec4(0.0f))
			: m_HalfFOV(halfFOV), m_Near(near), m_Far(far), m_DisplayIndex(displayIndex), m_Priority(priority), m_ClearColor(clearColor), m_LayerMask(0), m_LastHash(0) {}
		
		float m_HalfFOV;
		float m_Near;
		float m_Far;
		
		int m_DisplayIndex;
		int m_Priority;
		LayerMask m_LayerMask;
		glm::vec4 m_ClearColor;

		size_t m_LastHash;

		CameraRef m_Camera;
	};

	struct LookAt
	{
		LookAt() : m_Eye(0.0f), m_Center(0.0f), m_Up({0.0f, 1.0f, 0.0f}) {}
		LookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up) : m_Eye(eye), m_Center(center), m_Up(up) {}

		glm::vec3 m_Eye;
		glm::vec3 m_Center;
		glm::vec3 m_Up;
	};

	struct Spin
	{
		Spin() : m_Speed(10.0f) {}
		Spin(float speed) : m_Speed(speed) {}

		float m_Speed;
	};

	struct LayerComponent
	{
		LayerComponent() : m_pLayer(nullptr) {}
		LayerComponent(const Layer* pLayer) : m_pLayer(pLayer) {}

		const Layer* m_pLayer;
	};

	struct LightComponent
	{
		LightComponent() : m_Color(1.0f), m_Intensity(10.0f), m_Range(100.0f) {}
		LightComponent(const glm::vec4& color, float intensity, float range) : m_Color(color), m_Intensity(intensity), m_Range(range) {}

		glm::vec4 m_Color;
		float m_Intensity;
		float m_Range;
	};

	//ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//
	//int width, height;
	//pWindow->GetDrawableSize(&width, &height);

	//ubo.proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 10.0f);
	//ubo.proj[1][1] *= -1; // In OpenGL the Y coordinate of the clip coordinates is inverted, so we must flip it for use in Vulkan
}
