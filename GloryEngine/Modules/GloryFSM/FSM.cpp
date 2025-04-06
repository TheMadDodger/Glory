#include "FSM.h"

#include <BinaryStream.h>

namespace Glory
{
	FSMData::FSMData(): m_StartNodeIndex(0)
	{
		APPEND_TYPE(FSMData);
	}

	FSMData::~FSMData()
	{
	}

	size_t FSMData::NodeCount() const
	{
		return m_Nodes.size();
	}

	size_t FSMData::TransitionCount() const
	{
		return m_Transitions.size();
	}

	const FSMNode& FSMData::Node(size_t index) const
	{
		return m_Nodes[index];
	}

	const FSMTransition& FSMData::Transition(size_t index) const
	{
		return m_Transitions[index];
	}

	FSMNode& FSMData::NewNode(const std::string& name, UUID id)
	{
		return m_Nodes.emplace_back(name, id);
	}

	FSMTransition& FSMData::NewTransition(const std::string& name, UUID from, UUID to, UUID id)
	{
		return m_Transitions.emplace_back(name, from, to, id);
	}

	size_t FSMData::StartNodeIndex() const
	{
		return m_StartNodeIndex;
	}

	UUID FSMData::StartNodeID() const
	{
		if (m_StartNodeIndex >= m_Nodes.size()) return 0;
		return m_Nodes[m_StartNodeIndex].m_ID;
	}

	void FSMData::SetStartNodeIndex(size_t index)
	{
		if (m_StartNodeIndex >= m_Nodes.size()) return;
		m_StartNodeIndex = index;
	}

	void FSMData::References(Engine*, std::vector<UUID>&) const {}

	void FSMData::Serialize(BinaryStream& container) const
	{
		container.Write(m_StartNodeIndex);
		container.Write(m_Nodes.size());
		for (size_t i = 0; i < m_Nodes.size(); ++i)
		{
			container.Write(m_Nodes[i].m_Name).Write(m_Nodes[i].m_ID);
		}

		container.Write(m_Transitions.size());
		for (size_t i = 0; i < m_Transitions.size(); ++i)
		{
			container.Write(m_Transitions[i].m_Name).Write(m_Transitions[i].m_ID)
				.Write(m_Transitions[i].m_FromNode).Write(m_Transitions[i].m_ToNode);
		}
	}

	void FSMData::Deserialize(BinaryStream& container)
	{
		container.Read(m_StartNodeIndex);
		size_t size;
		container.Read(size);
		m_Nodes.resize(size);
		for (size_t i = 0; i < size; ++i)
		{
			container.Read(m_Nodes[i].m_Name).Read(m_Nodes[i].m_ID);
		}

		container.Read(size);
		m_Transitions.resize(size);
		for (size_t i = 0; i < size; ++i)
		{
			container.Read(m_Transitions[i].m_Name).Read(m_Transitions[i].m_ID)
				.Read(m_Transitions[i].m_FromNode).Read(m_Transitions[i].m_ToNode);
		}
	}
}
