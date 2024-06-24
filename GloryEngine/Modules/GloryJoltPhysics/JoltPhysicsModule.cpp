#include "JoltPhysicsModule.h"
#include "Helpers.h"

#include "PhysicsSystem.h"
#include "CharacterControllerSystem.h"

#include "LayerCollisionFilter.h"
#include "JoltCharacterManager.h"
#include "JoltShapeManager.h"
#include "BroadPhaseImpl.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseQuery.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseQuadTree.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>

#include <Debug.h>
#include <cstdarg>

#include <LayerManager.h>
#include <LayerRef.h>
#include <GLORY_YAML.h>

#include <JoltDebugRenderer.h>
#include <GameTime.h>
#include <Engine.h>
#include <SceneManager.h>
#include <BinaryStream.h>
#include <PhysicsComponents.h>


using namespace JPH;
using namespace JPH::literals;

namespace Glory
{
	JPH::Vec3 m_Gravity;

	LayerCollisionFilter m_CollisionFilter;
	BPLayerInterfaceImpl m_BPLayerImpl;
	ObjectVsBroadPhaseLayerFilterImpl m_ObjectVSBroadPhase;

	MyBodyActivationListener m_BodyActivationListener;
	MyContactListener m_ContactListener;
	JoltCharacterManager m_CharacterManager;
	JoltShapeManager m_ShapeManager;

	GLORY_MODULE_VERSION_CPP(JoltPhysicsModule);

	// Callback for traces, connect this to your own trace function if you have one
	static void TraceImpl(const char* inFMT, ...)
	{
		// Format the message
		//va_list list;
		//va_start(list, inFMT);
		//char buffer[1024];
		//vsnprintf(buffer, sizeof(buffer), inFMT, list);
		//va_end(list);
		//
		//// Print to the TTY
		//m_pEngine->GetDebug().LogInfo(buffer);
	}

	JoltPhysicsModule::JoltPhysicsModule()
		: m_pJPHTempAllocator(nullptr), m_pJPHJobSystem(nullptr), m_pJPHPhysicsSystem(nullptr),
		m_CollisionMatrix(std::vector<std::vector<bool>>())
	{
		m_CollisionFilter.m_pPhysicsModule = this;
		m_BodyActivationListener.m_pPhysics = this;
		m_ContactListener.m_pPhysics = this;
	}

	JoltPhysicsModule::~JoltPhysicsModule()
	{
		m_CharacterManager.DestroyAll();
		m_ContactCallbacks.clear();
		m_ActivationCallbacks.clear();
	}

	uint32_t JoltPhysicsModule::CreatePhysicsBody(const Shape& shape, const glm::vec3& inPosition, const glm::quat& inRotation, const glm::vec3& inScale, const BodyType bodyType, const uint16_t layerIndex)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::Shape* pShape = GetJPHShape(shape);
		JPH::Shape::ShapeResult scaledShape = pShape->ScaleShape(ToJPHVec3(inScale));
		BodyCreationSettings bodySettings(scaledShape.Get(), ToJPHVec3(inPosition), ToJPHQuat(inRotation), (EMotionType)bodyType, layerIndex);
		JPH::BodyID bodyID = bodyInterface.CreateAndAddBody(bodySettings, EActivation::Activate);
		return bodyID.GetIndexAndSequenceNumber();
	}

	void JoltPhysicsModule::DestroyPhysicsBody(uint32_t& bodyID)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		if (jphBodyID.IsInvalid()) return;
		bodyInterface.RemoveBody(jphBodyID);
		bodyInterface.DestroyBody(jphBodyID);
		bodyID = JPH::BodyID::cInvalidBodyID;
	}

	void JoltPhysicsModule::PollPhysicsState(uint32_t bodyID, glm::vec3* outPosition, glm::quat* outRotation)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };

		/* Output current position and velocity of the sphere */
		*outPosition = ToVec3(bodyInterface.GetCenterOfMassPosition(jphBodyID));
		*outRotation = ToQuat(bodyInterface.GetRotation(jphBodyID));
	}

	void JoltPhysicsModule::SetBodyPosition(uint32_t bodyID, const glm::vec3& position, const ActivationType activationType)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };

		bodyInterface.SetPosition(jphBodyID, ToJPHVec3(position), EActivation(activationType));
	}

	void JoltPhysicsModule::SetBodyRotation(uint32_t bodyID, const glm::quat& rotation, const ActivationType activationType)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };

		bodyInterface.SetRotation(jphBodyID, ToJPHQuat(rotation), EActivation(activationType));
	}

	void JoltPhysicsModule::SetBodyScale(uint32_t bodyID, const glm::vec3& inScale, const ActivationType activationType)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };

		JPH::ShapeRefC shape = bodyInterface.GetShape(jphBodyID);
		if (!shape) return;
		shape->ScaleShape(ToJPHVec3(inScale));

		if (activationType == ActivationType::Activate)
			bodyInterface.ActivateBody(jphBodyID);
	}

	void JoltPhysicsModule::ActivateBody(uint32_t bodyID)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		bodyInterface.ActivateBody(jphBodyID);
	}

	void JoltPhysicsModule::DeactivateBody(uint32_t bodyID)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		bodyInterface.DeactivateBody(jphBodyID);
	}

	bool JoltPhysicsModule::IsBodyActive(uint32_t bodyID) const
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		return bodyInterface.IsActive(jphBodyID);
	}

	bool JoltPhysicsModule::IsValidBody(uint32_t bodyID) const
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		if (jphBodyID.IsInvalid()) return false;
		return bodyInterface.IsAdded(jphBodyID);
	}

	glm::vec3 JoltPhysicsModule::GetBodyPosition(uint32_t bodyID) const
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		return ToVec3(bodyInterface.GetPosition(jphBodyID));
	}

	glm::vec3 JoltPhysicsModule::GetBodyCenterOfMassPosition(uint32_t bodyID) const
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		return ToVec3(bodyInterface.GetCenterOfMassPosition(jphBodyID));
	}

	glm::quat JoltPhysicsModule::GetBodyRotation(uint32_t bodyID) const
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		return ToQuat(bodyInterface.GetRotation(jphBodyID));
	}

	void JoltPhysicsModule::MoveBodyKinematic(uint32_t bodyID, const glm::vec3& targetPosition, const glm::quat& targetRotation, float deltaTime)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		bodyInterface.MoveKinematic(jphBodyID, ToJPHVec3(targetPosition), ToJPHQuat(targetRotation), deltaTime);
	}

	void JoltPhysicsModule::SetBodyLinearAndAngularVelocity(uint32_t bodyID, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		bodyInterface.SetLinearAndAngularVelocity(jphBodyID, ToJPHVec3(linearVelocity), ToJPHVec3(angularVelocity));
	}

	void JoltPhysicsModule::GetBodyLinearAndAngularVelocity(uint32_t bodyID, glm::vec3& linearVelocity, glm::vec3& angularVelocity) const
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		JPH::Vec3 lv, av;
		bodyInterface.GetLinearAndAngularVelocity(jphBodyID, lv, av);
		linearVelocity = ToVec3(lv);
		angularVelocity = ToVec3(av);
	}

	void JoltPhysicsModule::SetBodyLinearVelocity(uint32_t bodyID, const glm::vec3& linearVelocity)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		bodyInterface.SetLinearVelocity(jphBodyID, ToJPHVec3(linearVelocity));
	}

	glm::vec3 JoltPhysicsModule::GetBodyLinearVelocity(uint32_t bodyID) const
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		return ToVec3(bodyInterface.GetLinearVelocity(jphBodyID));
	}

	void JoltPhysicsModule::AddBodyLinearVelocity(uint32_t bodyID, const glm::vec3& linearVelocity)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		bodyInterface.AddLinearVelocity(jphBodyID, ToJPHVec3(linearVelocity));
	}

	void JoltPhysicsModule::AddBodyLinearAndAngularVelocity(uint32_t bodyID, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		bodyInterface.AddLinearAndAngularVelocity(jphBodyID, ToJPHVec3(linearVelocity), ToJPHVec3(angularVelocity));
	}

	void JoltPhysicsModule::SetBodyAngularVelocity(uint32_t bodyID, const glm::vec3& angularVelocity)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		bodyInterface.SetAngularVelocity(jphBodyID, ToJPHVec3(angularVelocity));
	}

	glm::vec3 JoltPhysicsModule::GetBodyAngularVelocity(uint32_t bodyID) const
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		return ToVec3(bodyInterface.GetAngularVelocity(jphBodyID));
	}

	glm::vec3 JoltPhysicsModule::GetBodyPointVelocity(uint32_t bodyID, const glm::vec3& point) const
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		return ToVec3(bodyInterface.GetPointVelocity(jphBodyID, ToJPHVec3(point)));
	}

	void JoltPhysicsModule::SetBodyPositionRotationAndVelocity(uint32_t bodyID, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		bodyInterface.SetPositionRotationAndVelocity(jphBodyID, ToJPHVec3(position), ToJPHQuat(rotation), ToJPHVec3(linearVelocity), ToJPHVec3(angularVelocity));
	}

	void JoltPhysicsModule::AddBodyForce(uint32_t bodyID, const glm::vec3& force)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		bodyInterface.AddForce(jphBodyID, ToJPHVec3(force));
	}

	void JoltPhysicsModule::AddBodyForce(uint32_t bodyID, const glm::vec3& force, const glm::vec3& point)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		bodyInterface.AddForce(jphBodyID, ToJPHVec3(force), ToJPHVec3(point));
	}

	void JoltPhysicsModule::AddBodyTorque(uint32_t bodyID, const glm::vec3& torque)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		bodyInterface.AddTorque(jphBodyID, ToJPHVec3(torque));
	}

	void JoltPhysicsModule::AddBodyForceAndTorque(uint32_t bodyID, const glm::vec3& force, const glm::vec3& torque)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		bodyInterface.AddForceAndTorque(jphBodyID, ToJPHVec3(force), ToJPHVec3(torque));
	}

	void JoltPhysicsModule::AddBodyImpulse(uint32_t bodyID, const glm::vec3& impulse)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		bodyInterface.AddImpulse(jphBodyID, ToJPHVec3(impulse));
	}

	void JoltPhysicsModule::AddBodyImpulse(uint32_t bodyID, const glm::vec3& impulse, const glm::vec3& point)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		bodyInterface.AddImpulse(jphBodyID, ToJPHVec3(impulse), ToJPHVec3(point));
	}

	void JoltPhysicsModule::AddBodyAngularImpulse(uint32_t bodyID, const glm::vec3& angularImpulse)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		bodyInterface.AddAngularImpulse(jphBodyID, ToJPHVec3(angularImpulse));
	}

	void JoltPhysicsModule::SetBodyObjectLayer(uint32_t bodyID, const uint16_t layerIndex)
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		bodyInterface.SetObjectLayer(jphBodyID, layerIndex);
	}

	const uint16_t JoltPhysicsModule::GetBodyObjectLayer(uint32_t bodyID) const
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		JPH::BodyID jphBodyID{ bodyID };
		return bodyInterface.GetObjectLayer(jphBodyID);
	}

	class RayCastLayerFilter : public JPH::ObjectLayerFilter
	{
	public:
		RayCastLayerFilter(const LayerMask& layerMask) : m_Mask(layerMask) {}

		virtual bool ShouldCollide(ObjectLayer inLayer) const
		{
			return m_Mask == 0 || inLayer == 0 || m_Mask & LayerMask{ uint64_t(1) << (inLayer - 1) };
		}

	private:
		const LayerMask m_Mask;
	};

	bool JoltPhysicsModule::CastRay(const Ray& ray, RayCastResult& result, float maxDistance, const LayerMask& layerMask, const std::vector<uint32_t>& ignoreBodyIDs) const
	{
		const JPH::RRayCast jphRay{ ToJPHVec3(ray.m_Origin), ToJPHVec3(ray.m_Direction) * maxDistance };
		AllHitCollisionCollector<CastRayCollector> collector;
		IgnoreMultipleBodiesFilter bodyFilter;
		bodyFilter.Reserve(ignoreBodyIDs.size());
		for (size_t i = 0; i < ignoreBodyIDs.size(); ++i)
		{
			bodyFilter.IgnoreBody(JPH::BodyID(ignoreBodyIDs[i]));
		}

		RayCastSettings ray_settings;
		m_pJPHPhysicsSystem->GetNarrowPhaseQuery().CastRay(jphRay, ray_settings, collector, {}, RayCastLayerFilter{ layerMask }, bodyFilter);
		if (!collector.HadHit()) return false;
		result = RayCastResult();
		for (size_t i = 0; i < collector.mHits.size(); ++i)
		{
			const JPH::RayCastResult& rayHit = collector.mHits[i];
			const JPH::RVec3 hit = jphRay.GetPointOnRay(rayHit.mFraction);
			const float distance = rayHit.mFraction * maxDistance;

			result.m_Hits.push_back(
				RayCastHit{ distance, rayHit.mBodyID.GetIndexAndSequenceNumber(),
				rayHit.mSubShapeID2.GetValue(), ToVec3(hit) }
			);
		}
		return true;
	}

	BPLayerInterfaceImpl& JoltPhysicsModule::BPLayerImpl()
	{
		return m_BPLayerImpl;
	}

	ObjectVsBroadPhaseLayerFilterImpl& JoltPhysicsModule::BPCollisionFilter()
	{
		return m_ObjectVSBroadPhase;
	}

	void JoltPhysicsModule::TriggerLateActivationCallback(ActivationCallback callbackType, uint32_t bodyID)
	{
		m_LateActivationCallbacks[callbackType].push_back(bodyID);
	}

	void JoltPhysicsModule::TriggerLateContactCallback(ContactCallback callbackType, uint32_t body1ID, uint32_t body2ID)
	{
		m_LateContactCallbacks[callbackType].push_back({ body1ID, body2ID });
	}

	JoltCharacterManager* JoltPhysicsModule::GetCharacterManager()
	{
		return &m_CharacterManager;
	}

	JoltShapeManager* JoltPhysicsModule::GetShapeManager()
	{
		return &m_ShapeManager;
	}

	void JoltPhysicsModule::SetGravity(const glm::vec3& gravity)
	{
		m_Gravity = ToJPHVec3(gravity);
		if (m_pJPHPhysicsSystem) m_pJPHPhysicsSystem->SetGravity(m_Gravity);
	}

	const glm::vec3 JoltPhysicsModule::GetGravity() const
	{
		return ToVec3(m_pJPHPhysicsSystem->GetGravity());
	}

	void JoltPhysicsModule::CleanupPhysics()
	{
		m_CharacterManager.DestroyAll();

		delete m_pJPHTempAllocator;
		m_pJPHTempAllocator = nullptr;
		delete m_pJPHPhysicsSystem;
		m_pJPHPhysicsSystem = nullptr;
	}

	void JoltPhysicsModule::SetupPhysics()
	{
		if (m_pJPHTempAllocator) CleanupPhysics();

		// We need a temp allocator for temporary allocations during the physics update. We're
		// pre-allocating 10 MB to avoid having to do allocations during the physics update. 
		// B.t.w. 10 MB is way too much for this example but it is a typical value you can use.
		// If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
		// malloc / free.
		const uint32_t tempAllocationSize = Settings().Value<unsigned int>("TemporaryAllocationSize");
		m_pJPHTempAllocator = new JPH::TempAllocatorImpl{ tempAllocationSize };

		// Now we can create the actual physics system.
		const uint32_t maxBodies = Settings().Value<unsigned int>("MaxBodies");
		const uint32_t numBodyMutexes = Settings().Value<unsigned int>("NumBodyMutexes");
		const uint32_t maxBodyPairs = Settings().Value<unsigned int>("MaxBodyPairs");
		const uint32_t maxContactConstraints = Settings().Value<unsigned int>("MaxContactConstraints");

		JPH::PhysicsSettings settings{};

		// Create physics system
		m_pJPHPhysicsSystem = new JPH::PhysicsSystem();
		m_pJPHPhysicsSystem->Init(maxBodies, numBodyMutexes, maxBodyPairs,
			maxContactConstraints, m_BPLayerImpl,
			m_ObjectVSBroadPhase, m_CollisionFilter);
		m_pJPHPhysicsSystem->SetPhysicsSettings(settings);

		// Restore gravity
		m_pJPHPhysicsSystem->SetGravity(m_Gravity);

		// A body activation listener gets notified when bodies activate and go to sleep
		// Note that this is called from a job so whatever you do here needs to be thread safe.
		// Registering one is entirely optional.
		m_pJPHPhysicsSystem->SetBodyActivationListener(&m_BodyActivationListener);

		// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
		// Note that this is called from a job so whatever you do here needs to be thread safe.
		// Registering one is entirely optional.
		m_pJPHPhysicsSystem->SetContactListener(&m_ContactListener);

		// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
		// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();

		std::map<JPH::ObjectLayer, JPH::BroadPhaseLayer> bpLayersMapping;
		bpLayersMapping.emplace(0, 0);
		for (size_t i = 0; i < Settings().ArraySize("BroadPhaseLayerMapping"); ++i)
		{
			const std::string valueStr = Settings().ArrayValue<std::string>("BroadPhaseLayerMapping", i);
			BPLayer bpLayer;
			Enum<BPLayer>().FromString(valueStr, bpLayer);
			bpLayersMapping.emplace((uint16_t)i + 1, JPH::BroadPhaseLayer(JPH::uint8(bpLayer)));
		}
		m_BPLayerImpl.SetObjectToBroadphase(std::move(bpLayersMapping));

		const size_t bpLayerCount = Enum< BPLayer>().NumValues();
		std::vector<LayerMask> bpCollisionMapping = std::vector<LayerMask>(bpLayerCount);
		for (size_t i = 0; i < bpLayerCount; i++)
		{
			std::string layerName;
			Enum<BPLayer>().ToString(BPLayer(i), layerName);
			const LayerMask mask = Settings().Value<LayerMask>(layerName + "CollisionMask");
			bpCollisionMapping[i] = mask;
		}
		m_ObjectVSBroadPhase.SetBPCollisionMapping(std::move(bpCollisionMapping));

		m_CharacterManager.SetPhysicsSystem(m_pJPHPhysicsSystem);
	}

	//glm::mat4 JoltPhysicsModule::GetBodyWorldTransform(uint32_t bodyID) const
	//{
	//	JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
	//	JPH::BodyID jphBodyID{ bodyID };
	//	if (jphBodyID.IsInvalid()) return {};
	//	return ToVec3(bodyInterface.GetPosition(jphBodyID));
	//}
	//
	//glm::mat4 JoltPhysicsModule::GetBodyCenterOfMassTransform(uint32_t bodyID) const
	//{
	//	JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
	//	JPH::BodyID jphBodyID{ bodyID };
	//	if (jphBodyID.IsInvalid()) return {};
	//	return ToVec3(bodyInterface.GetPosition(jphBodyID));
	//}

	void JoltPhysicsModule::SetCollisionMatrix(std::vector<std::vector<bool>>&& matrix)
	{
		m_CollisionMatrix = std::move(matrix);
	}

	bool JoltPhysicsModule::ShouldCollide(uint16_t layer1, uint16_t layer2) const
	{
		if (layer1 == 0 || layer2 == 0) return true;
		return ShouldCollidePass(layer1 - 1, layer2 - 1) || ShouldCollidePass(layer2 - 1, layer1 - 1);
	}

	bool JoltPhysicsModule::ShouldCollidePass(uint16_t layer1, uint16_t layer2) const
	{
		if (layer1 >= m_CollisionMatrix.size()) return false;
		if (layer2 >= m_CollisionMatrix[layer1].size()) return false;
		return m_CollisionMatrix[layer1][layer2];
	}

	void JoltPhysicsModule::RegisterContactCallback(ContactCallback callbackType, std::function<void(uint32_t, uint32_t)> callback)
	{
		m_ContactCallbacks[callbackType].push_back(callback);
	}

	void JoltPhysicsModule::RegisterActivationCallback(ActivationCallback callbackType, std::function<void(uint32_t)> callback)
	{
		m_ActivationCallbacks[callbackType].push_back(callback);
	}

	void JoltPhysicsModule::TriggerContactCallback(ContactCallback callbackType, uint32_t bodyID1, uint32_t bodyID2)
	{
		for (size_t i = 0; i < m_ContactCallbacks[callbackType].size(); i++)
		{
			m_ContactCallbacks[callbackType][i](bodyID1, bodyID2);
		}
	}

	void JoltPhysicsModule::TriggerActivationCallback(ActivationCallback callbackType, uint32_t bodyID)
	{
		for (size_t i = 0; i < m_ActivationCallbacks[callbackType].size(); i++)
		{
			m_ActivationCallbacks[callbackType][i](bodyID);
		}
	}

	void JoltPhysicsModule::LoadSettings(ModuleSettings& settings)
	{
		settings.RegisterValue<unsigned int>("TemporaryAllocationSize", 10 * 1024 * 1024);

		/* This is the max amount of rigid bodies that you can add to the physics system.If you try to add more you'll get an error.
		 * Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
		 */
		settings.RegisterValue<unsigned int>("MaxBodies", 1024);

		/* This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings. */
		settings.RegisterValue<unsigned int>("NumBodyMutexes", 0);

		/* This is the max amount of body pairs that can be queued at any time(the broad phase will detect overlapping
		 * body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
		 * too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
		 * Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
		 */
		settings.RegisterValue<unsigned int>("MaxBodyPairs", 1024);

		/* This is the maximum size of the contact constraint buffer.If more contacts(collisions between bodies) are detected than this
		 * number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
		 * Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
		 */
		settings.RegisterValue<unsigned int>("MaxContactConstraints", 1024);

		/* Refresh rate of physics world */
		settings.RegisterValue<unsigned int>("TickRate", 60);

		/* Broadphase layer mappings */
		settings.PushGroup("Broadphase Layers");
		settings.RegisterArray<BPLayer>("BroadPhaseLayerMapping");

		const size_t bpLayerCount = Enum<BPLayer>().NumValues();
		for (size_t i = 0; i < bpLayerCount; i++)
		{
			std::string layerName;
			Enum<BPLayer>().ToString(BPLayer(i), layerName);
			settings.RegisterValue<LayerMask>(layerName + "CollisionMask", 0);
		}
	}

	void JoltPhysicsModule::Initialize()
	{
		PhysicsModule::Initialize();

		m_ObjectVSBroadPhase.m_pLayers = &m_pEngine->GetLayerManager();

		Reflect::SetReflectInstance(&m_pEngine->Reflection());

		Reflect::RegisterEnum<BPLayer>();

		// Register allocation hook
		JPH::RegisterDefaultAllocator();

		// Install callbacks
		JPH::Trace = TraceImpl;
		//JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

		// Create a factory
		JPH::Factory::sInstance = new JPH::Factory();

		// Register all Jolt physics types
		JPH::RegisterTypes();

		// We need a job system that will execute physics jobs on multiple threads. Typically
		// you would implement the JobSystem interface yourself and let Jolt Physics run on top
		// of your own job scheduler. JobSystemThreadPool is an example implementation.
		m_pJPHJobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

#ifdef JPH_DEBUG_RENDERER
		DebugRenderer::sInstance = new JoltDebugRenderer(m_pEngine);
#endif

		SetupPhysics();

		// Next we can create a rigid body to serve as the floor, we make a large box
		// Create the settings for the collision volume (the shape). 
		// Note that for simple shapes (like boxes) you can also directly construct a BoxShape.
		//JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(100.0f, 1.0f, 100.0f));

		// Create the shape
		//JPH::ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
		//JPH::ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()

		// Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
		//JPH::BodyCreationSettings floor_settings(floor_shape, JPH::RVec3(0.0_r, -1.0_r, 0.0_r), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);

		// Create the actual rigid body
		//JPH::Body* floor = bodyInterface.CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr

		// Add it to the world
		//bodyInterface.AddBody(floor->GetID(), JPH::EActivation::DontActivate);

		// Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
		// You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
		// Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
		//m_pJPHPhysicsSystem->OptimizeBroadPhase();

		SceneManager* pScenes = m_pEngine->GetSceneManager();
		Glory::Utils::ECS::ComponentTypes* pComponentTypes = pScenes->ComponentTypesInstance();

		/* Physics Bodies */
		pComponentTypes->RegisterInvokaction<PhysicsBody>(Glory::Utils::ECS::InvocationType::Start, PhysicsSystem::OnStart);
		pComponentTypes->RegisterInvokaction<PhysicsBody>(Glory::Utils::ECS::InvocationType::Stop, PhysicsSystem::OnStop);
		pComponentTypes->RegisterInvokaction<PhysicsBody>(Glory::Utils::ECS::InvocationType::OnRemove, PhysicsSystem::OnStop);
		pComponentTypes->RegisterInvokaction<PhysicsBody>(Glory::Utils::ECS::InvocationType::OnValidate, PhysicsSystem::OnValidate);
		pComponentTypes->RegisterInvokaction<PhysicsBody>(Glory::Utils::ECS::InvocationType::Update, PhysicsSystem::OnUpdate);

		/* Character controllers */
		pComponentTypes->RegisterInvokaction<CharacterController>(Glory::Utils::ECS::InvocationType::Start, CharacterControllerSystem::OnStart);
		pComponentTypes->RegisterInvokaction<CharacterController>(Glory::Utils::ECS::InvocationType::Stop, CharacterControllerSystem::OnStop);
		pComponentTypes->RegisterInvokaction<CharacterController>(Glory::Utils::ECS::InvocationType::OnRemove, CharacterControllerSystem::OnStop);
		pComponentTypes->RegisterInvokaction<CharacterController>(Glory::Utils::ECS::InvocationType::OnValidate, CharacterControllerSystem::OnValidate);
		pComponentTypes->RegisterInvokaction<CharacterController>(Glory::Utils::ECS::InvocationType::Update, CharacterControllerSystem::OnUpdate);

		RegisterActivationCallback(ActivationCallback::Activated, PhysicsSystem::OnBodyActivated);
		RegisterActivationCallback(ActivationCallback::Deactivated, PhysicsSystem::OnBodyDeactivated);

		RegisterContactCallback(ContactCallback::Added, PhysicsSystem::OnContactAdded);
		RegisterContactCallback(ContactCallback::Persisted, PhysicsSystem::OnContactPersisted);
		RegisterContactCallback(ContactCallback::Removed, PhysicsSystem::OnContactRemoved);
	}

	void JoltPhysicsModule::PostInitialize()
	{
	}

	void JoltPhysicsModule::Cleanup()
	{
		// Destroy the factory
		delete JPH::Factory::sInstance;
		JPH::Factory::sInstance = nullptr;

		delete m_pJPHJobSystem;
		m_pJPHJobSystem = nullptr;

		CleanupPhysics();
	}

	void JoltPhysicsModule::Update()
	{
		for (size_t i = 0; i < size_t(ActivationCallback::Count); ++i)
		{
			for (size_t j = 0; j < m_LateActivationCallbacks[ActivationCallback(i)].size(); j++)
			{
				TriggerActivationCallback(ActivationCallback(i), m_LateActivationCallbacks[ActivationCallback(i)][j]);
			}
			m_LateActivationCallbacks[ActivationCallback(i)].clear();
		}

		for (size_t i = 0; i < size_t(ContactCallback::Count); ++i)
		{
			for (size_t j = 0; j < m_LateContactCallbacks[ContactCallback(i)].size(); j++)
			{
				const std::pair<uint32_t, uint32_t>& pair = m_LateContactCallbacks[ContactCallback(i)][j];
				TriggerContactCallback(ContactCallback(i), pair.first, pair.second);
			}
			m_LateContactCallbacks[ContactCallback(i)].clear();
		}

		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		
		// Now we're ready to simulate the body, keep simulating until it goes to sleep
		static uint32_t step = 0;
		
		// We simulate the physics world in discrete time steps.
		const float engineDeltaTime = m_pEngine->Time().GetDeltaTime<float, std::ratio<1, 1>>();
		const uint32_t tickRate = Settings().Value<unsigned int>("TickRate");
		static float timer = 0.0f;
		timer += engineDeltaTime;
		if (timer < 1.0f / tickRate) return;
		timer -= 1.0f / tickRate;

		const float deltaTime = 1.0f / tickRate;
		
		// Next step
		++step;
		
		// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
		const int collisionSteps = 1;
		
		// If you want more accurate step results you can do multiple sub steps within a collision step. Usually you would set this to 1.
		const int integrationSubSteps = 1;

		// Step the world
		m_pJPHPhysicsSystem->Update(deltaTime, collisionSteps, m_pJPHTempAllocator, m_pJPHJobSystem);

		m_CharacterManager.PostSimulation(1.0f);
	}

	void JoltPhysicsModule::Draw()
	{
		/* TODO: These need module settings! */

		if (!m_pJPHPhysicsSystem) return;

#ifdef JPH_DEBUG_RENDERER
		JPH::BodyManager::DrawSettings settings{};
		settings.mDrawShape = true;
		settings.mDrawShapeWireframe = true;
		settings.mDrawVelocity = true;
		settings.mDrawBoundingBox = true;
		settings.mDrawCenterOfMassTransform = true;
		m_pJPHPhysicsSystem->DrawBodies(settings, DebugRenderer::sInstance);
#endif
	}

	void JoltPhysicsModule::OnProcessData()
	{
		if (!m_pEngine->HasData("Physics")) return;
		std::vector<char>& buffer = m_pEngine->GetData("Physics");

		BinaryMemoryStream memoryStream{ buffer };
		BinaryStream* stream = &memoryStream;
		glm::vec3 gravity;
		stream->Read(gravity);
		SetGravity(gravity);

		size_t size;
		stream->Read(size);
		if (!size) return;

		m_CollisionMatrix.clear();
		m_CollisionMatrix.resize(size);
		for (size_t x = 0; x < m_CollisionMatrix.size(); ++x)
		{
			m_CollisionMatrix[x].resize(size);
			for (size_t y = 0; y < m_CollisionMatrix[x].size(); ++y)
			{
				bool value;
				stream->Read(value);
				m_CollisionMatrix[x][y] = value;
			}
		}
	}

	const std::type_info& JoltPhysicsModule::GetModuleType()
	{
		return typeid(JoltPhysicsModule);
	}
}
