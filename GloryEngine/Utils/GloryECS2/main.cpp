#include <crtdbg.h>

#include <chrono>
#include <print>

#include "EntityRegistry.h"
#include "RegistryFactory.h"
#include "ComponentManager.h"

#define NO_COPY(c) c(c&) = delete;\
c(const c&) = delete;\
c& operator=(c&) = delete;\
c& operator=(const c&) = delete;

#define DEFAULT_MOVE(c) c(c&&) noexcept = default;\
c& operator=(c&&) = default;

namespace Glory::Utils::ECS
{
	struct Transform
	{
	public:
		Transform(): x(0.0f), y(0.0f), z(0.0f) {};

		NO_COPY(Transform);
		DEFAULT_MOVE(Transform);

		float x, y, z;
	};

	struct Velocity
	{
	public:
		Velocity() : x(0.0f), y(0.0f), z(0.0f) {};

		NO_COPY(Velocity);
		DEFAULT_MOVE(Velocity);

		float x, y, z;
	};

	class TransformManager : public ComponentManager<Transform>
	{
	public:
		TransformManager(EntityRegistry* pRegistry): ComponentManager(pRegistry) {};
		virtual ~TransformManager() = default;
	};

	class VelocityManager : public ComponentManager<Velocity>
	{
	public:
		VelocityManager(EntityRegistry* pRegistry): ComponentManager(pRegistry) {};
		virtual ~VelocityManager() = default;

		virtual void OnInitialize() override
		{
			Bind(DoUpdate, &VelocityManager::DoUpdateImpl);
		}

		void DoUpdateImpl(EntityID entity, Velocity& velocity, float dt)
		{
			Transform& transform = m_pRegistry->GetComponent<Transform>(entity);
			transform.x += dt*velocity.x;
			transform.y += dt*velocity.y;
			transform.z += dt*velocity.z;
		}
	};
}

using namespace Glory::Utils::ECS;

static uint64_t Now()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();
}

static float TimeSinceSeconds(uint64_t timestamp)
{
	return (Now() - timestamp)/(1000.0f*1000.0f);
}

int main(int argc, char* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	RegistryFactory factory;
	factory.RegisterComponentManager<TransformManager>();
	factory.RegisterComponentManager<VelocityManager>();

	EntityRegistry registry;
	factory.PopulateRegisry(registry);

	const size_t entityCount = 10000;
	for (size_t i = 0; i < entityCount; ++i)
	{
		auto entity = registry.CreateEntity();
		registry.AddComponent<Transform>(entity);
		Velocity& velocity = registry.AddComponent<Velocity>(entity);
		velocity.x = (std::rand()%1000)/1000.0f;
		velocity.y = (std::rand()%1000)/1000.0f;
		velocity.z = (std::rand()%1000)/1000.0f;
		if (i <= 20) continue;
		EntityID parent = EntityID(std::rand()%i);
		if (parent == 0ull) continue;
		registry.SetParent(entity, parent);
	}

	const uint64_t startTime = Now();
	float lastTime = TimeSinceSeconds(startTime);
	{
		registry.Sort();
		const float time = TimeSinceSeconds(startTime);
		double sortTime = time - lastTime;
		std::println("Sorting took {} seconds", sortTime);
	}

	double averageDeltaTime = 0.0f;
	double longestDeltaTime = 0.0f;
	double shortestDeltaTime = DBL_MAX;
	const size_t runCount = 50000;

	std::println("Running {} updates on {} entities", runCount, entityCount);
	for (size_t i = 0; i < runCount; ++i)
	{
		const float time = TimeSinceSeconds(startTime);
		double deltaTime = time - lastTime;
		averageDeltaTime += deltaTime;
		lastTime = time;

		shortestDeltaTime = std::min(shortestDeltaTime, deltaTime);
		longestDeltaTime = std::max(longestDeltaTime, deltaTime);

		registry.Update(float(deltaTime));
	}

	averageDeltaTime /= runCount;
	std::println("Average delta time: {}", averageDeltaTime);
	std::println("Longest delta time: {}", longestDeltaTime);
	std::println("Shortest delta time: {}", shortestDeltaTime);
	std::println("Average framerate: {}", 60.0/averageDeltaTime);

	return 0;
}