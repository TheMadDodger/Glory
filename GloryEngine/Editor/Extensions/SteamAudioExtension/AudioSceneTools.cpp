#include "AudioSceneTools.h"

#include <GScene.h>
#include <PhysicsComponents.h>
#include <Components.h>
#include <SoundComponents.h>
#include <Engine.h>

namespace Glory::Editor
{
	MeshData GenerateMesh(const ShapeProperty& shape, const glm::mat4& transform)
	{
		MeshData mesh{ 100, 3 * sizeof(float), {AttributeType::Float3} };

		switch (shape.m_ShapeType)
		{
		case ShapeType::Sphere:
			break;
		case ShapeType::Box: {
			const Box& box = *shape.ShapePointer<Box>();
			const glm::vec3 extends = box.m_Extends;

			const glm::vec4 topFrontLeft = transform * glm::vec4(-extends.x, extends.y, -extends.z, 1.0f);
			const glm::vec4 topFrontRight = transform * glm::vec4(extends.x, extends.y, -extends.z, 1.0f);
			const glm::vec4 topBackRight = transform * glm::vec4(extends.x, extends.y, extends.z, 1.0f);
			const glm::vec4 topBackLeft = transform * glm::vec4(-extends.x, extends.y, extends.z, 1.0f);

			const glm::vec4 bottomFrontLeft = transform * glm::vec4(-extends.x, -extends.y, -extends.z, 1.0f);
			const glm::vec4 bottomFrontRight = transform * glm::vec4(extends.x, -extends.y, -extends.z, 1.0f);
			const glm::vec4 bottomBackRight = transform * glm::vec4(extends.x, -extends.y, extends.z, 1.0f);
			const glm::vec4 bottomBackLeft = transform * glm::vec4(-extends.x, -extends.y, extends.z, 1.0f);

			const uint32_t vBottomFrontLeft = mesh.AddVertex(reinterpret_cast<const float*>(&bottomFrontLeft));
			const uint32_t vBottomFrontRight = mesh.AddVertex(reinterpret_cast<const float*>(&bottomFrontRight));
			const uint32_t vTopFrontRight = mesh.AddVertex(reinterpret_cast<const float*>(&topFrontRight));
			const uint32_t vTopFrontLeft = mesh.AddVertex(reinterpret_cast<const float*>(&topFrontLeft));

			const uint32_t vTopBackLeft = mesh.AddVertex(reinterpret_cast<const float*>(&topBackLeft));
			const uint32_t vTopBackRight = mesh.AddVertex(reinterpret_cast<const float*>(&topBackRight));
			const uint32_t vBottomBackRight = mesh.AddVertex(reinterpret_cast<const float*>(&bottomBackRight));
			const uint32_t vBottomBackLeft = mesh.AddVertex(reinterpret_cast<const float*>(&bottomBackLeft));

			/* Front face */
			mesh.AddFace(vBottomFrontLeft, vBottomFrontRight, vTopFrontRight, vTopFrontLeft);

			/* Left face */
			mesh.AddFace(vBottomFrontLeft, vTopFrontLeft, vTopBackLeft, vBottomBackLeft);

			/* Right face */
			mesh.AddFace(vBottomFrontRight, vBottomBackRight, vTopBackRight, vTopFrontRight);

			/* Top face */
			mesh.AddFace(vTopFrontLeft, vTopFrontRight, vTopBackRight, vTopBackLeft);

			/* Bottom face */
			mesh.AddFace(vBottomFrontLeft, vBottomFrontRight, vBottomBackRight, vBottomBackLeft);

			/* Back face */
			mesh.AddFace(vTopBackLeft, vTopBackRight, vBottomBackRight, vBottomBackLeft);

			break;
		}
		case ShapeType::Cylinder:
			break;
		case ShapeType::Capsule:
			break;
		case ShapeType::TaperedCapsule:
			break;
		default:
			return mesh;
		}

		return mesh;
	}

	AudioScene GenerateAudioScene(Engine* pEngine, GScene* pScene, const SoundMaterial* defaultMaterial)
	{
		AudioScene audioScene{ pScene->GetUUID() };
		Utils::ECS::EntityRegistry& registry = pScene->GetRegistry();
		Utils::ECS::TypeView<PhysicsBody>* pPhysicsBodies = registry.GetTypeView<PhysicsBody>();
		if (!pPhysicsBodies) return audioScene;

		for (size_t i = 0; i < pPhysicsBodies->Size(); ++i)
		{
			const Utils::ECS::EntityID entity = pPhysicsBodies->EntityAt(i);
			const PhysicsBody& body = pPhysicsBodies->Get(entity);

			/* Only static bodies will be considered */
			switch (body.m_BodyType)
			{
			case BodyType::Static:
				break;
			default:
				continue;
			}

			const Transform& transform = registry.GetComponent<Transform>(entity);

			if (body.m_Shape.m_ShapeType == ShapeType::None) continue;
			SoundMaterial materialData;
			const SoundMaterial* material = defaultMaterial;

			/* Get material from sound occluder */
			if (registry.HasComponent<SoundOccluder>(entity))
			{
				const SoundOccluder& occluder = registry.GetComponent<SoundOccluder>(entity);
				Utils::ECS::EntityView* pEntityView = registry.GetEntityView(entity);
				const bool active = pEntityView->IsActive() && registry.GetTypeView<SoundOccluder>()->IsActive(entity);
				if (!active) continue;
				material = &occluder.m_Material;
				if (occluder.m_MaterialAsset)
				{
					SoundMaterialData* pMaterial = occluder.m_MaterialAsset.GetImmediate(&pEngine->GetAssetManager());
					if (pMaterial)
						material = &pMaterial->Material();
				}
			}

			materialData.m_Absorption = material->m_Absorption;
			materialData.m_Scattering = material->m_Scattering;
			materialData.m_Transmission = material->m_Transmission;

			MeshData mesh = GenerateMesh(body.m_Shape, transform.MatTransform);
			audioScene.AddMesh(std::move(mesh), std::move(materialData));
		}

		return audioScene;
	}
}
