#pragma once
#include <string>
#include <vector>

#include <Resource.h>

#include <Reflection.h>
#include <UUID.h>

namespace Glory
{
	struct FSMTransition
	{
		FSMTransition() : m_Name("New Transition"), m_FromNode(0), m_ToNode(0) {}
		FSMTransition(const std::string& name, UUID from, UUID to) : m_Name(name), m_FromNode(from), m_ToNode(to) {}

		REFLECTABLE(FSMTransition,
			(std::string)(m_Name),
			(UUID)(m_FromNode),
			(UUID)(m_ToNode)
		);
	};

	struct FSMNode
	{
		FSMNode() : m_Name("New Node") {}
		FSMNode(const std::string& name) : m_Name(name) {}

		REFLECTABLE(FSMNode,
			(std::string)(m_Name)
		);

		UUID m_ID;
	};

	class FSMData : public Resource
	{
	public:
		GLORY_API FSMData();
		GLORY_API virtual ~FSMData();

	private:
		/** @brief Get a vector containing other resources referenced by this resource */
		virtual void References(Engine*, std::vector<UUID>&) const override;

		virtual void Serialize(BinaryStream& container) const override;
		virtual void Deserialize(BinaryStream& container) override;

	private:
		std::vector<FSMNode> m_Nodes;
		std::vector<FSMTransition> m_Transitions;
	};
}