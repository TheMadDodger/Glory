#pragma once
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

#include <engine_visibility.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
//#include <glm/gtx/quaternion.hpp>

#include <Reflection.h>
#include <map>

REFLECTABLE_ENUM_NS(Glory, CameraPerspective, Orthographic, Perspective)
REFLECTABLE_ENUM_NS(Glory, PhysicsShapeType, Sphere, Box)
REFLECTABLE_ENUM_NS(Glory, CameraOutputMode, None, ScaledResolution, FixedResolution)

#define PROPERTY_BUFFER_SIZE 2048;

namespace Glory
{
	struct Transform
	{
		GLORY_ENGINE_API Transform();
		GLORY_ENGINE_API Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);
		GLORY_ENGINE_API Transform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);

		REFLECTABLE(Transform,
			(glm::vec3)	(Position),
			(glm::quat)	(Rotation),
			(glm::vec3) (Scale)
		)

		glm::mat4 MatTransform;
	};

	struct MeshRenderer
	{
		GLORY_ENGINE_API MeshRenderer(MeshData* pMesh, MaterialData* pMaterial)
			: m_Mesh(pMesh != nullptr ? pMesh->GetUUID() : UUID(0ull)), m_Material(pMaterial != nullptr ? pMaterial->GetUUID() : UUID(0ull)),
			m_RenderStatic(false), m_WasSubmittedForStatic(false) {}
		GLORY_ENGINE_API MeshRenderer()
			: m_Mesh(0ull), m_Material(0ull), m_RenderStatic(false), m_WasSubmittedForStatic(false) {}

		REFLECTABLE_DESCRIPTIVE(MeshRenderer,
			PROP_TOOLTIP(ResourceReference<MeshData>, Mesh, "Mesh to render"),
			PROP_TOOLTIP(ResourceReference<MaterialData>, Material, "Material to render the mesh with"),
			(bool, m_RenderStatic, "Render Static", "Render this mesh as a static object")
		);

		bool m_WasSubmittedForStatic;
	};

	struct CameraComponent
	{
		GLORY_ENGINE_API CameraComponent() : m_HalfFOV(45.0f), m_Near(0.1f), m_Far(3000.0f), m_Priority(0),
			m_ClearColor(glm::vec4(0.0f)), m_Offset(0.0f, 0.0f), m_Resolution(1.0f, 1.0f),
			m_OutputMode(CameraOutputMode::ScaledResolution), m_LayerMask(0) {}
		GLORY_ENGINE_API CameraComponent(float halfFOV, float near, float far, int priority = 0, const glm::vec4& clearColor = glm::vec4(0.0f))
			: m_HalfFOV(halfFOV), m_Near(near), m_Far(far), m_Priority(priority), m_ClearColor(clearColor), m_Offset(0.0f, 0.0f),
			m_Resolution(1.0f, 1.0f), m_OutputMode(CameraOutputMode::ScaledResolution), m_LayerMask(0) {}

		REFLECTABLE_DESCRIPTIVE(CameraComponent,
			(float, m_HalfFOV, "Half FOV", "Half angle of horizontal Field of View"),
			(float, m_Near, "Near", "Near clipping plane"),
			(float, m_Far, "Far", "Far clipping plane"),
			(int, m_Priority, "Priority", "Render priority, lower value means it gets rendered first"),
			(LayerMask, m_LayerMask, "Layer Mask", "Mask to filter objects from being rendered with this camera"),
			(glm::vec4, m_ClearColor, "Clear Color", "The color to clean the cameras output with before rendering a new frame"),
			(glm::vec2, m_Offset, "Offset", "Set an offset on the render viewport\nWARNING! this feature is currently experimental!"),
			(glm::vec2, m_Resolution, "Resolution", "Output resolution of the camera"),
			(CameraOutputMode, m_OutputMode, "Output Mode",
				"Scales the resolution based on an output mode and sends the cameras render result to the screen\n"
				"	None - Do nothing\n"
				"	ScaledResolution - The cameras resolution is Resolution*ScreenResolution\n"
				"	FixedResolution - The resolution is fixed to the set resolution\n")
		)

		CameraRef m_Camera;
	};

	struct LayerComponent
	{
		GLORY_ENGINE_API LayerComponent() : m_Layer(0) {}

		REFLECTABLE_DESCRIPTIVE(LayerComponent,
			(LayerRef, m_Layer, "Layer", "")
		)
	};

	struct ShadowSettings
	{
		GLORY_ENGINE_API ShadowSettings() : m_Enable(false), m_Bias(0.0000005f)
		{}

		REFLECTABLE_DESCRIPTIVE(ShadowSettings,
			(bool, m_Enable, "", ""),
			(float, m_Bias, "Bias", "")
		);
	};

	struct LightComponent
	{
		GLORY_ENGINE_API LightComponent() :
			m_Type(LightType::Point), m_Color(1.0f), m_Intensity(1.0f), m_Inner(45.0f),
			m_Outer(60.0f), m_Range(100.0f), m_FalloffExponent(1.0f) {}
		GLORY_ENGINE_API LightComponent(const glm::vec4& color, float intensity, float exponent, float range) :
			m_Type(LightType::Point), m_Color(color), m_Intensity(intensity), m_Inner(45.0f),
			m_Outer(60.0f), m_Range(range), m_FalloffExponent(exponent) {}

		REFLECTABLE_DESCRIPTIVE(LightComponent,
			(LightType, m_Type, "Type", ""),
			(glm::vec3, m_Color, "Color", ""),
			(float, m_Intensity, "Intensity", ""),
			(float, m_Inner, "Inner Range/Radius", ""),
			(float, m_Outer, "Outer Range/Radius", ""),
			(float, m_Range, "Range", ""),
			(float, m_FalloffExponent, "Falloff Exponent", ""),
			(ShadowSettings, m_Shadows, "Shadow Settings", "")
		)
	};

	struct TextComponent
	{
		GLORY_ENGINE_API TextComponent() : m_Font(0ull), m_Text("Hello World!"), m_Scale(0.01f),
			m_Color(1.0f, 1.0f, 1.0f, 1.0f), m_Alignment(Alignment::Left), m_WrapWidth(0.0f), m_Dirty(true) {}

		REFLECTABLE_DESCRIPTIVE(TextComponent,
			PROP(ResourceReference<FontData>, Font),
			PROP(std::string, Text),
			PROP(float, Scale),
			PROP(glm::vec4, Color),
			PROP(Alignment, Alignment),
			PROP(float, WrapWidth)
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
