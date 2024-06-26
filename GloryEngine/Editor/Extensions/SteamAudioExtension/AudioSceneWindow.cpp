#include "AudioSceneWindow.h"

#include <PhysicsComponents.h>

#include <Engine.h>
#include <SceneManager.h>
#include <Components.h>

#include <SteamAudioModule.h>

#include <EditorApplication.h>

#include <glm/gtx/matrix_decompose.hpp>

#include <phonon/phonon.h>

namespace Glory::Editor
{
	AudioSceneWindow::AudioSceneWindow(): EditorWindowTemplate("Steam Audio", 1600.0f, 600.0f)
	{
	}

	AudioSceneWindow::~AudioSceneWindow()
	{
	}

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

			const glm::vec4 topFrontLeft = transform*glm::vec4(-extends.x, extends.y, -extends.z, 1.0f);
			const glm::vec4 topFrontRight = transform*glm::vec4(extends.x, extends.y, -extends.z, 1.0f);
			const glm::vec4 topBackRight = transform*glm::vec4(extends.x, extends.y, extends.z, 1.0f);
			const glm::vec4 topBackLeft = transform*glm::vec4(-extends.x, extends.y, extends.z, 1.0f);

			const glm::vec4 bottomFrontLeft = transform*glm::vec4(-extends.x, -extends.y, -extends.z, 1.0f);
			const glm::vec4 bottomFrontRight = transform*glm::vec4(extends.x, -extends.y, -extends.z, 1.0f);
			const glm::vec4 bottomBackRight = transform*glm::vec4(extends.x, -extends.y, extends.z, 1.0f);
			const glm::vec4 bottomBackLeft = transform*glm::vec4(-extends.x, -extends.y, extends.z, 1.0f);

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

	void AudioSceneWindow::OnGUI()
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		SteamAudioModule* pSteamAudio = pEngine->GetOptionalModule<SteamAudioModule>();
		if (!pSteamAudio) return;
		IPLContext context = pSteamAudio->GetContext();

		if (ImGui::Button("Build occlusion scene"))
		{
			IPLSceneSettings sceneSettings{};
			sceneSettings.type = IPL_SCENETYPE_DEFAULT;

			IPLScene scene = nullptr;
			iplSceneCreate(context, &sceneSettings, &scene);

			SceneManager* sceneManager = pEngine->GetSceneManager();
			for (size_t i = 0; i < sceneManager->OpenScenesCount(); ++i)
			{
				GScene* pScene = sceneManager->GetOpenScene(i);
				Utils::ECS::EntityRegistry& registry = pScene->GetRegistry();
				Utils::ECS::TypeView<PhysicsBody>* pPhysicsBodies = registry.GetTypeView<PhysicsBody>();
				if (!pPhysicsBodies) continue;

				for (size_t i = 0; i < pPhysicsBodies->Size(); ++i)
				{
					const Utils::ECS::EntityID entity = pPhysicsBodies->EntityAt(i);
					const PhysicsBody& body = pPhysicsBodies->Get(entity);
					switch (body.m_BodyType)
					{
					case BodyType::Static:
						break;
					default:
						continue;
					}

					const Transform& transform = registry.GetComponent<Transform>(entity);

					if (body.m_Shape.m_ShapeType == ShapeType::None) continue;
					MeshData mesh = GenerateMesh(body.m_Shape, transform.MatTransform);

					/* @todo: Materials should come from a separate component, maybe even a resource? */
					IPLMaterial materials[1] = {
						{ {0.1f, 0.1f, 0.1f}, 0.5f, {0.2f, 0.2f, 0.2f} }
					};

					IPLint32 materialIndices[6] = { 0, 0, 0, 0, 0, 0 };

					IPLStaticMeshSettings staticMeshSettings{};
					staticMeshSettings.numVertices = mesh.VertexCount();
					staticMeshSettings.numTriangles = 2;
					staticMeshSettings.numMaterials = 1;
					staticMeshSettings.vertices = reinterpret_cast<IPLVector3*>(mesh.Vertices());
					staticMeshSettings.triangles = reinterpret_cast<IPLTriangle*>(mesh.Indices());
					staticMeshSettings.materialIndices = materialIndices;
					staticMeshSettings.materials = materials;

					IPLStaticMesh staticMesh = nullptr;
					iplStaticMeshCreate(scene, &staticMeshSettings, &staticMesh);
					iplStaticMeshAdd(staticMesh, scene);
					iplSceneCommit(scene);
				}
			}

			pSteamAudio->SetScene(scene);
		}
	}
}
