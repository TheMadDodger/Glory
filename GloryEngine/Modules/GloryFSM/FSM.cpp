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
		const size_t index = NodeIndex(from);
		_ASSERT(index != m_Nodes.size());
		m_Nodes[index].m_Transitions.push_back(id);
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

	const FSMNode* FSMData::FindNode(std::string_view name) const
	{
		for (size_t i = 0; i < m_Nodes.size(); ++i)
		{
			if (m_Nodes[i].m_Name != name) continue;
			return &m_Nodes[i];
		}
		return nullptr;
	}

	const FSMTransition* FSMData::FindTransition(std::string_view name) const
	{
		for (size_t i = 0; i < m_Transitions.size(); ++i)
		{
			if (m_Transitions[i].m_Name != name) continue;
			return &m_Transitions[i];
		}
		return nullptr;
	}

	const FSMNode* FSMData::Node(UUID id) const
	{
		for (size_t i = 0; i < m_Nodes.size(); ++i)
		{
			if (m_Nodes[i].m_ID != id) continue;
			return &m_Nodes[i];
		}
		return nullptr;
	}

	const FSMTransition* FSMData::Transition(UUID id) const
	{
		return nullptr;
	}

	size_t FSMData::NodeIndex(UUID id) const
	{
		for (size_t i = 0; i < m_Nodes.size(); ++i)
		{
			if (m_Nodes[i].m_ID != id) continue;
			return i;
		}
		return m_Nodes.size();
	}

	size_t FSMData::TransitionIndex(UUID id) const
	{
		for (size_t i = 0; i < m_Nodes.size(); ++i)
		{
			if (m_Transitions[i].m_ID != id) continue;
			return i;
		}
		return m_Transitions.size();
	}

	FSMProperty& FSMData::NewProperty(const std::string& name, FSMPropertyType type, UUID id)
	{
		return m_Properties.emplace_back(name, type, id);
	}

	size_t FSMData::PropertyCount() const
	{
		return m_Properties.size();
	}

	const FSMProperty& FSMData::Property(size_t index) const
	{
		return m_Properties[index];
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
				.Write(m_Transitions[i].m_FromNode).Write(m_Transitions[i].m_ToNode)
				.Write(m_Transitions[i].m_Property).Write(m_Transitions[i].m_TransitionOp)
				.Write(m_Transitions[i].m_CompareValue);
		}

		container.Write(m_Properties.size());
		for (size_t i = 0; i < m_Properties.size(); ++i)
		{
			container.Write(m_Properties[i].m_Name).Write(m_Properties[i].m_Type).Write(m_Properties[i].m_ID);
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
				.Read(m_Transitions[i].m_FromNode).Read(m_Transitions[i].m_ToNode)
				.Read(m_Transitions[i].m_Property).Read(m_Transitions[i].m_TransitionOp)
				.Read(m_Transitions[i].m_CompareValue);
		}

		container.Read(size);
		m_Properties.resize(size);
		for (size_t i = 0; i < size; ++i)
		{
			container.Read(m_Properties[i].m_Name).Read(m_Properties[i].m_Type)
				.Read(m_Properties[i].m_ID);
		}
	}

	FSMState::FSMState(FSMModule* pModule, FSMData* pFSM, UUID instanceID) :
		m_pModule(pModule), m_OriginalFSMID(pFSM->GetUUID()), m_InstanceID(instanceID), m_CurrentState(0),
		m_PropertyData(pFSM->PropertyCount()*sizeof(float)) {}

	void FSMState::SetCurrentState(UUID stateID)
	{
		m_CurrentState = stateID;
	}

	UUID FSMState::ID() const
	{
		return m_InstanceID;
	}

	UUID FSMState::OriginalFSMID() const
	{
		return m_OriginalFSMID;
	}
}
