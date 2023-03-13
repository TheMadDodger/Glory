#include "JoltPhysicsModule.h"
#include <Debug.h>
#include <cstdarg>

using namespace JPH::literals;

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(JoltPhysicsModule, 0, 1);

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

	JoltPhysicsModule::JoltPhysicsModule()
	{
	}

	JoltPhysicsModule::~JoltPhysicsModule()
	{
	}

	void JoltPhysicsModule::LoadSettings(ModuleSettings& settings)
	{
		settings.RegisterValue<unsigned int>("MaxBodies", 1024);
		settings.RegisterValue<unsigned int>("NumBodyMutexes", 0);
		settings.RegisterValue<unsigned int>("MaxBodyPairs", 1024);
		settings.RegisterValue<unsigned int>("MaxContactConstraints", 1024);
		settings.RegisterValue<unsigned int>("TickRate", 60);
	}

	void JoltPhysicsModule::Initialize()
	{
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
		JPH::TempAllocatorImpl temp_allocator(10 * 1024 * 1024);

		// We need a job system that will execute physics jobs on multiple threads. Typically
		// you would implement the JobSystem interface yourself and let Jolt Physics run on top
		// of your own job scheduler. JobSystemThreadPool is an example implementation.
		JPH::JobSystemThreadPool job_system(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

		// This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
		// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
		const uint32_t cMaxBodies = 1024;

		// This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
		const uint32_t cNumBodyMutexes = 0;

		// This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
		// body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
		// too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
		// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
		const uint32_t cMaxBodyPairs = 1024;

		// This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
		// number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
		// Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
		const uint32_t cMaxContactConstraints = 1024;

		// Create mapping table from object layer to broadphase layer
		// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
		//JPH::BPLayerInterfaceImpl broad_phase_layer_interface;

		// Create class that filters object vs broadphase layers
		// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
		//JPH::ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;

		// Create class that filters object vs object layers
		// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
		//JPH::ObjectLayerPairFilterImpl object_vs_object_layer_filter;

		// Now we can create the actual physics system.
		//JPH::PhysicsSystem physics_system;
		//physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

		// A body activation listener gets notified when bodies activate and go to sleep
		// Note that this is called from a job so whatever you do here needs to be thread safe.
		// Registering one is entirely optional.
		//JPH::MyBodyActivationListener body_activation_listener;
		//physics_system.SetBodyActivationListener(&body_activation_listener);

		// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
		// Note that this is called from a job so whatever you do here needs to be thread safe.
		// Registering one is entirely optional.
		//JPH::MyContactListener contact_listener;
		//physics_system.SetContactListener(&contact_listener);

		// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
		// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
		//JPH::BodyInterface& body_interface = physics_system.GetBodyInterface();

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
		//JPH::Body* floor = body_interface.CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr

		// Add it to the world
		//body_interface.AddBody(floor->GetID(), JPH::EActivation::DontActivate);

		// Now create a dynamic body to bounce on the floor
		// Note that this uses the shorthand version of creating and adding a body to the world
		//JPH::BodyCreationSettings sphere_settings(new JPH::SphereShape(0.5f), JPH::RVec3(0.0_r, 2.0_r, 0.0_r), JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, Layers::MOVING);
		//JPH::BodyID sphere_id = body_interface.CreateAndAddBody(sphere_settings, JPH::EActivation::Activate);

		// Now you can interact with the dynamic body, in this case we're going to give it a velocity.
		// (note that if we had used CreateBody then we could have set the velocity straight on the body before adding it to the physics system)
		//body_interface.SetLinearVelocity(sphere_id, JPH::Vec3(0.0f, -5.0f, 0.0f));

		// We simulate the physics world in discrete time steps. 60 Hz is a good rate to update the physics system.
		const float cDeltaTime = 1.0f / 60.0f;

		// Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
		// You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
		// Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
		//physics_system.OptimizeBroadPhase();

		// Now we're ready to simulate the body, keep simulating until it goes to sleep
		//uint32_t step = 0;
		//while (body_interface.IsActive(sphere_id))
		//{
		//	// Next step
		//	++step;
		//
		//	// Output current position and velocity of the sphere
		//	JPH::RVec3 position = body_interface.GetCenterOfMassPosition(sphere_id);
		//	JPH::Vec3 velocity = body_interface.GetLinearVelocity(sphere_id);
		//
		//	std::stringstream stream;
		//	stream << "Step " << step << ": Position = (" << position.GetX() << ", " << position.GetY() << ", " << position.GetZ() << "), Velocity = (" << velocity.GetX() << ", " << velocity.GetY() << ", " << velocity.GetZ() << ")";
		//	Debug::LogInfo(stream.str());
		//
		//	// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
		//	const int cCollisionSteps = 1;
		//
		//	// If you want more accurate step results you can do multiple sub steps within a collision step. Usually you would set this to 1.
		//	const int cIntegrationSubSteps = 1;
		//
		//	// Step the world
		//	physics_system.Update(cDeltaTime, cCollisionSteps, cIntegrationSubSteps, &temp_allocator, &job_system);
		//}

		// Remove the sphere from the physics system. Note that the sphere itself keeps all of its state and can be re-added at any time.
		//body_interface.RemoveBody(sphere_id);

		// Destroy the sphere. After this the sphere ID is no longer valid.
		//body_interface.DestroyBody(sphere_id);

		// Remove and destroy the floor
		//body_interface.RemoveBody(floor->GetID());
		//body_interface.DestroyBody(floor->GetID());

		// Destroy the factory
		delete JPH::Factory::sInstance;
		JPH::Factory::sInstance = nullptr;
	}

	void JoltPhysicsModule::PostInitialize()
	{
	}

	void JoltPhysicsModule::Cleanup()
	{
	}
}
