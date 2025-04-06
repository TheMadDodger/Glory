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
		FSMTransition() :
			m_Name("New Transition"), m_FromNode(0), m_ToNode(0) {}
		FSMTransition(const std::string& name, UUID from, UUID to, UUID id) :
			m_Name(name), m_FromNode(from), m_ToNode(to), m_ID(id) {}

		REFLECTABLE(FSMTransition,
			(std::string)(m_Name),
			(UUID)(m_FromNode),
			(UUID)(m_ToNode)
		);

		UUID m_ID;
	};

	struct FSMNode
	{
		FSMNode() : m_Name("New Node") {}
		FSMNode(const std::string& name, UUID id) : m_Name(name), m_ID(id) {}

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

		GLORY_API size_t NodeCount() const;
		GLORY_API size_t TransitionCount() const;
		GLORY_API const FSMNode& Node(size_t index) const;
		GLORY_API const FSMTransition& Transition(size_t index) const;
		GLORY_API FSMNode& NewNode(const std::string& name, UUID id=UUID());
		GLORY_API FSMTransition& NewTransition(const std::string& name, UUID from, UUID to, UUID id=UUID());
		GLORY_API size_t StartNodeIndex() const;
		GLORY_API UUID StartNodeID() const;
		GLORY_API void SetStartNodeIndex(size_t index);

	private:
		/** @brief Get a vector containing other resources referenced by this resource */
		virtual void References(Engine*, std::vector<UUID>&) const override;

		virtual void Serialize(BinaryStream& container) const override;
		virtual void Deserialize(BinaryStream& container) override;

	private:
		std::vector<FSMNode> m_Nodes;
		std::vector<FSMTransition> m_Transitions;
		size_t m_StartNodeIndex;
	};
}