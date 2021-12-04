#include "SpinSystem.h"
#include "Registry.h"
#include <chrono>
#include <glm/gtx/matrix_decompose.hpp>

namespace Glory
{
	void SpinSystem::OnDraw(Registry* pRegistry, EntityID entity, Spin& pComponent)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count() * pComponent.m_Speed;

		Transform& transform = pRegistry->GetComponent<Transform>(entity);

		glm::mat4 world = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(world, transform.Scale, transform.Rotation, transform.Position, skew, perspective);
	}
}
