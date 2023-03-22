#include "PhysicsModule.h"

namespace Glory
{
	PhysicsModule::PhysicsModule() : m_CollisionMatrix(std::vector<std::vector<bool>>())
	{
	}

	PhysicsModule::~PhysicsModule()
	{
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
}
