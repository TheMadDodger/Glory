#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "Entity.h"

#include "ModelData.h"
#include "MaterialData.h"
#include "FontData.h"
#include "Camera.h"
#include "CameraRef.h"
#include "LayerManager.h"
#include "LightData.h"
#include "AssetReference.h"
#include "LayerRef.h"
#include "ShapeProperty.h"
#include "RenderData.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
//#include <glm/gtx/quaternion.hpp>

#include <Reflection.h>
#include <map>


REFLECTABLE_ENUM_NS(Glory, CameraPerspective, Orthographic, Perspective)
REFLECTABLE_ENUM_NS(Glory, PhysicsShapeType, Sphere, Box)

#define PROPERTY_BUFFER_SIZE 2048;

namespace Glory
{
	struct Transform
	{
		GLORY_API Transform();
		GLORY_API Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);
		GLORY_API Transform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);

		REFLECTABLE(Transform,
			(glm::vec3)	(Position),
			(glm::quat)	(Rotation),
			(glm::vec3) (Scale)
		)

		glm::mat4 MatTransform;
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
		MeshRenderer(MeshData* pMesh, MaterialData* pMaterial)
			: m_Mesh(pMesh != nullptr ? pMesh->GetUUID() : 0), m_Material(pMaterial != nullptr ? pMaterial->GetUUID() : 0) {}
		MeshRenderer()
			: m_Mesh(0), m_Material(0) {}

		REFLECTABLE(MeshRenderer,
			(AssetReference<MeshData>) (m_Mesh),
			(AssetReference<MaterialData>)	(m_Material)
		)
	};

	struct ModelRenderer
	{
		ModelRenderer(ModelData* pModel, MaterialData* pMaterial) : m_Model(pModel != nullptr ? pModel->GetUUID() : 0), m_Materials({ pMaterial != nullptr ? pMaterial->GetUUID() : 0 }) {}
		ModelRenderer() : m_Model(0), m_Materials(std::vector<MeshMaterial>()) {}

		REFLECTABLE(ModelRenderer,
			(AssetReference<ModelData>)	(m_Model),
			(std::vector<MeshMaterial>)	(m_Materials)
		)
	};

	struct CameraComponent
	{
		GLORY_API CameraComponent() : m_HalfFOV(45.0f), m_Near(0.1f), m_Far(3000.0f), m_DisplayIndex(0), m_Priority(0), m_ClearColor(glm::vec4(0.0f)), m_LayerMask(0), m_LastHash(0) {}
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

			uint32_t m_LastHash;

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
		LayerComponent() : m_Layer(0) {}

		REFLECTABLE(LayerComponent,
			(LayerRef)	(m_Layer)
		)
	};

	struct LightComponent
	{
		LightComponent() :
			m_Type(LightType::Point), m_Color(1.0f), m_Inner(45.0f),
			m_Outer(45.0f), m_Range(100.0f), m_Intensity(1.0f) {}
		LightComponent(const glm::vec4& color, float intensity, float range) :
			m_Type(LightType::Point), m_Color(color), m_Inner(45.0f),
			m_Outer(45.0f), m_Range(range), m_Intensity(intensity) {}

		REFLECTABLE(LightComponent,
			(LightType)	(m_Type),
			(glm::vec4)	(m_Color),
			(float)	(m_Inner),
			(float)	(m_Outer),
			(float)	(m_Range),
			(float)	(m_Intensity)
		)
	};

	struct TextComponent
	{
		TextComponent() : m_Font(0), m_Text("Hello World!"), m_Scale(0.01f),
			m_Color(1.0f, 1.0f, 1.0f, 1.0f), m_Alignment(Alignment::Left), m_WrapWidth(0.0f), m_Dirty(true) {}

		REFLECTABLE(TextComponent,
			(AssetReference<FontData>)(m_Font),
			(std::string)(m_Text),
			(float)(m_Scale),
			(glm::vec4)(m_Color),
			(Alignment)(m_Alignment),
			(float)(m_WrapWidth)
		);
		bool m_Dirty;
	};

	//settings->mSupportingVolume = Plane(Vec3::sAxisY(), -cCharacterRadiusStanding); // Accept contacts that touch the lower sphere of the capsule
	//mCharacter = new Character(settings, RVec3::sZero(), Quat::sIdentity(), 0, mPhysicsSystem);

	//ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//
	//int width, height;
	//pWindow->GetDrawableSize(&width, &height);

	//ubo.proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 10.0f);
	//ubo.proj[1][1] *= -1; // In OpenGL the Y coordinate of the clip coordinates is inverted, so we must flip it for use in Vulkan
}
