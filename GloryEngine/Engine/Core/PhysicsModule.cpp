#include "PhysicsModule.h"
#include "LayerMask.h"

namespace Glory
{
	PhysicsModule::PhysicsModule() : m_CollisionMatrix(std::vector<std::vector<bool>>())
	{
	}

	PhysicsModule::~PhysicsModule()
	{
		m_ContactCallbacks.clear();
		m_ActivationCallbacks.clear();
	}

	const std::type_info& PhysicsModule::GetModuleType()
	{
		return typeid(PhysicsModule);
	}

	void PhysicsModule::SetCollisionMatrix(std::vector<std::vector<bool>>&& matrix)
	{
		m_CollisionMatrix = std::move(matrix);
	}

	bool PhysicsModule::ShouldCollide(uint16_t layer1, uint16_t layer2) const
	{
		if (layer1 == 0 || layer2 == 0) return true;
		return ShouldCollidePass(layer1 - 1, layer2 - 1) || ShouldCollidePass(layer2 - 1, layer1 - 1);
	}

	bool PhysicsModule::ShouldCollidePass(uint16_t layer1, uint16_t layer2) const
	{
		if (layer1 >= m_CollisionMatrix.size()) return false;
		if (layer2 >= m_CollisionMatrix[layer1].size()) return false;
		return m_CollisionMatrix[layer1][layer2];
	}

	void PhysicsModule::RegisterContactCallback(ContactCallback callbackType, std::function<void(uint32_t, uint32_t)> callback)
	{
		m_ContactCallbacks[callbackType].push_back(callback);
	}

	void PhysicsModule::RegisterActivationCallback(ActivationCallback callbackType, std::function<void(uint32_t)> callback)
	{
		m_ActivationCallbacks[callbackType].push_back(callback);
	}

	void PhysicsModule::TriggerContactCallback(ContactCallback callbackType, uint32_t bodyID1, uint32_t bodyID2)
	{
		for (size_t i = 0; i < m_ContactCallbacks[callbackType].size(); i++)
		{
			m_ContactCallbacks[callbackType][i](bodyID1, bodyID2);
		}
	}

	void PhysicsModule::TriggerActivationCallback(ActivationCallback callbackType, uint32_t bodyID)
	{
		for (size_t i = 0; i < m_ActivationCallbacks[callbackType].size(); i++)
		{
			m_ActivationCallbacks[callbackType][i](bodyID);
		}
	}
}
