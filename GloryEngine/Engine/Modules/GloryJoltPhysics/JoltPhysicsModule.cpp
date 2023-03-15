#include "JoltPhysicsModule.h"
#include <Debug.h>
#include <cstdarg>

using namespace JPH;
using namespace JPH::literals;

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(JoltPhysicsModule, 0, 1);

	// Layer that objects can be in, determines which other objects it can collide with
// Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
// layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics simulation
// but only if you do collision testing).
	namespace Layers
	{
		static constexpr JPH::uint8 NON_MOVING = 0;
		static constexpr JPH::uint8 MOVING = 1;
		static constexpr JPH::uint8 NUM_LAYERS = 2;
	};

	/// Class that determines if two object layers can collide
	class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
	{
	public:
		virtual bool					ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
		{
			switch (inObject1)
			{
			case Layers::NON_MOVING:
				return inObject2 == Layers::MOVING; // Non moving only collides with moving
			case Layers::MOVING:
				return true; // Moving collides with everything
			default:
				JPH_ASSERT(false);
				return false;
			}
		}
	};

	// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
	// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
	// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
	// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
	// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
	namespace BroadPhaseLayers
	{
		static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
		static constexpr JPH::BroadPhaseLayer MOVING(1);
		static constexpr JPH::uint NUM_LAYERS(2);
	};

	// BroadPhaseLayerInterface implementation
	// This defines a mapping between object and broadphase layers.
	class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
	{
	public:
		BPLayerInterfaceImpl()
		{
			// Create a mapping table from object to broad phase layer
			mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
			mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
		}

		virtual JPH::uint					GetNumBroadPhaseLayers() const override
		{
			return BroadPhaseLayers::NUM_LAYERS;
		}

		virtual JPH::BroadPhaseLayer			GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
		{
			JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
			return mObjectToBroadPhase[inLayer];
		}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
		{
			switch ((BroadPhaseLayer::Type)inLayer)
			{
			case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
			case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
			default:													JPH_ASSERT(false); return "INVALID";
			}
		}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

	private:
		JPH::BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
	};

	/// Class that determines if an object layer can collide with a broadphase layer
	class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
	{
	public:
		virtual bool				ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
		{
			switch (inLayer1)
			{
			case Layers::NON_MOVING:
				return inLayer2 == BroadPhaseLayers::MOVING;
			case Layers::MOVING:
				return true;
			default:
				JPH_ASSERT(false);
				return false;
			}
		}
	};

	// An example contact listener
	class MyContactListener : public JPH::ContactListener
	{
	public:
		// See: ContactListener
		virtual JPH::ValidateResult	OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override
		{
			Debug::LogNotice("Contact validate callback");

			// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
			return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
		}

		virtual void			OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
		{
			Debug::LogNotice("A contact was added");
		}

		virtual void			OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
		{
			Debug::LogNotice("A contact was persisted");
		}

		virtual void			OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override
		{
			Debug::LogNotice("A contact was removed");
		}
	};

	// An example activation listener
	class MyBodyActivationListener : public JPH::BodyActivationListener
	{
	public:
		virtual void		OnBodyActivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override
		{
			Debug::LogNotice("A body got activated");
		}

		virtual void		OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override
		{
			Debug::LogNotice("A body went to sleep");
		}
	};




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
		//Debug::LogInfo(buffer);
	}




	MyBodyActivationListener body_activation_listener;
	MyContactListener contact_listener;

	// Create mapping table from object layer to broadphase layer
	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
	BPLayerInterfaceImpl broad_phase_layer_interface;

	// Create class that filters object vs broadphase layers
	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
	ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;

	// Create class that filters object vs object layers
	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
	ObjectLayerPairFilterImpl object_vs_object_layer_filter;

	JoltPhysicsModule::JoltPhysicsModule()
		: m_pJPHTempAllocator(nullptr), m_pJPHJobSystem(nullptr), m_pJPHPhysicsSystem(nullptr)
	{
	}

	JoltPhysicsModule::~JoltPhysicsModule()
	{
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



















		// Register allocation hook
		JPH::RegisterDefaultAllocator();

		// Install callbacks
		JPH::Trace = TraceImpl;
		//JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

		// Create a factory
		JPH::Factory::sInstance = new JPH::Factory();

		// Register all Jolt physics types
		JPH::RegisterTypes();

		// We need a temp allocator for temporary allocations during the physics update. We're
		// pre-allocating 10 MB to avoid having to do allocations during the physics update. 
		// B.t.w. 10 MB is way too much for this example but it is a typical value you can use.
		// If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
		// malloc / free.
		const uint32_t tempAllocationSize = Settings().Value<unsigned int>("TemporaryAllocationSize");
		m_pJPHTempAllocator = new JPH::TempAllocatorImpl{ tempAllocationSize };

		// We need a job system that will execute physics jobs on multiple threads. Typically
		// you would implement the JobSystem interface yourself and let Jolt Physics run on top
		// of your own job scheduler. JobSystemThreadPool is an example implementation.
		m_pJPHJobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

		// Now we can create the actual physics system.
		const uint32_t maxBodies = Settings().Value<unsigned int>("MaxBodies");
		const uint32_t numBodyMutexes = Settings().Value<unsigned int>("NumBodyMutexes");
		const uint32_t maxBodyPairs = Settings().Value<unsigned int>("MaxBodyPairs");
		const uint32_t maxContactConstraints = Settings().Value<unsigned int>("MaxContactConstraints");

		m_pJPHPhysicsSystem = new PhysicsSystem();

		m_pJPHPhysicsSystem->Init(maxBodies, numBodyMutexes, maxBodyPairs,
			maxContactConstraints, broad_phase_layer_interface,
			object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

		// A body activation listener gets notified when bodies activate and go to sleep
		// Note that this is called from a job so whatever you do here needs to be thread safe.
		// Registering one is entirely optional.
		m_pJPHPhysicsSystem->SetBodyActivationListener(&body_activation_listener);

		// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
		// Note that this is called from a job so whatever you do here needs to be thread safe.
		// Registering one is entirely optional.
		m_pJPHPhysicsSystem->SetContactListener(&contact_listener);

		// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
		// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();

		// Next we can create a rigid body to serve as the floor, we make a large box
		// Create the settings for the collision volume (the shape). 
		// Note that for simple shapes (like boxes) you can also directly construct a BoxShape.
		JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(100.0f, 1.0f, 100.0f));

		// Create the shape
		JPH::ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
		JPH::ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()

		// Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
		JPH::BodyCreationSettings floor_settings(floor_shape, JPH::RVec3(0.0_r, -1.0_r, 0.0_r), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);

		// Create the actual rigid body
		JPH::Body* floor = bodyInterface.CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr
		m_FloorID = floor->GetID();

		// Add it to the world
		bodyInterface.AddBody(floor->GetID(), JPH::EActivation::DontActivate);

		// Now create a dynamic body to bounce on the floor
		// Note that this uses the shorthand version of creating and adding a body to the world
		BodyCreationSettings sphere_settings(new SphereShape(0.5f), RVec3(0.0_r, 2.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		m_SphereID = bodyInterface.CreateAndAddBody(sphere_settings, EActivation::Activate);

		// Now you can interact with the dynamic body, in this case we're going to give it a velocity.
		// (note that if we had used CreateBody then we could have set the velocity straight on the body before adding it to the physics system)
		bodyInterface.SetLinearVelocity(m_SphereID, JPH::Vec3(0.0f, -5.0f, 0.0f));

		// Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
		// You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
		// Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
		m_pJPHPhysicsSystem->OptimizeBroadPhase();
	}

	void JoltPhysicsModule::Initialize()
	{
		
	}

	void JoltPhysicsModule::PostInitialize()
	{
	}

	void JoltPhysicsModule::Cleanup()
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();

		// Remove the sphere from the physics system. Note that the sphere itself keeps all of its state and can be re-added at any time.
		bodyInterface.RemoveBody(m_SphereID);

		// Destroy the sphere. After this the sphere ID is no longer valid.
		bodyInterface.DestroyBody(m_SphereID);

		// Remove and destroy the floor
		bodyInterface.RemoveBody(m_FloorID);
		bodyInterface.DestroyBody(m_FloorID);

		// Destroy the factory
		delete JPH::Factory::sInstance;
		JPH::Factory::sInstance = nullptr;

		delete m_pJPHTempAllocator;
		m_pJPHTempAllocator = nullptr;
		delete m_pJPHJobSystem;
		m_pJPHJobSystem = nullptr;
		delete m_pJPHPhysicsSystem;
		m_pJPHPhysicsSystem = nullptr;
	}

	void JoltPhysicsModule::Update()
	{
		JPH::BodyInterface& bodyInterface = m_pJPHPhysicsSystem->GetBodyInterface();
		
		// Now we're ready to simulate the body, keep simulating until it goes to sleep
		static uint32_t step = 0;
		if (!bodyInterface.IsActive(m_SphereID)) return;
		
		
		// We simulate the physics world in discrete time steps. 60 Hz is a good rate to update the physics system.
		const uint32_t tickRate = Settings().Value<unsigned int>("TickRate");
		const float deltaTime = 1.0f / tickRate;
		
		// Next step
		++step;
		
		// Output current position and velocity of the sphere
		JPH::RVec3 position = bodyInterface.GetCenterOfMassPosition(m_SphereID);
		JPH::Vec3 velocity = bodyInterface.GetLinearVelocity(m_SphereID);
		
		std::stringstream stream;
		stream << "Step " << step << ": Position = (" << position.GetX() << ", " << position.GetY() << ", " << position.GetZ() << "), Velocity = (" << velocity.GetX() << ", " << velocity.GetY() << ", " << velocity.GetZ() << ")";
		Debug::LogInfo(stream.str());
		
		// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
		const int collisionSteps = 1;
		
		// If you want more accurate step results you can do multiple sub steps within a collision step. Usually you would set this to 1.
		const int integrationSubSteps = 1;
		
		// Step the world
		m_pJPHPhysicsSystem->Update(deltaTime, collisionSteps, integrationSubSteps, m_pJPHTempAllocator, m_pJPHJobSystem);
	}
}
