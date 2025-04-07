#include "FSM.h"
#include "FSMModule.h"

#include <BinaryStream.h>
#include <AssetManager.h>
#include <Engine.h>

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
		for (size_t i = 0; i < m_Transitions.size(); ++i)
		{
			if (m_Transitions[i].m_ID != id) continue;
			return &m_Transitions[i];
		}
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

	const FSMProperty* FSMData::Property(UUID id) const
	{
		for (size_t i = 0; i < m_Properties.size(); ++i)
		{
			if (m_Properties[i].m_ID != id) continue;
			return &m_Properties[i];
		}
		return nullptr;
	}

	size_t FSMData::PropertyIndex(UUID id) const
	{
		for (size_t i = 0; i < m_Properties.size(); ++i)
		{
			if (m_Properties[i].m_ID != id) continue;
			return i;
		}
		return m_Properties.size();
	}

	size_t FSMData::PropertyIndex(std::string_view name) const
	{
		for (size_t i = 0; i < m_Nodes.size(); ++i)
		{
			if (m_Properties[i].m_Name != name) continue;
			return i;
		}
		return m_Properties.size();
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
		m_PropertyData(pFSM->PropertyCount()*sizeof(float)), m_PropertyDataChanged(true), m_FirstUpdate(true) {}

	void FSMState::SetCurrentState(UUID stateID)
	{
		m_CurrentState = stateID;
		m_FirstUpdate = true;
		m_PropertyDataChanged = true;
	}

	void FSMState::SetCurrentState(UUID stateID, FSMData* pFSM)
	{
		const FSMNode* node = pFSM->Node(m_CurrentState);
		if (!m_FirstUpdate && node && m_pModule->ExitCallback) m_pModule->ExitCallback(*this, *node);

		m_CurrentState = stateID;
		m_FirstUpdate = true;
		m_PropertyDataChanged = true;
	}

	UUID FSMState::CurrentState() const
	{
		return m_CurrentState;
	}

	UUID FSMState::ID() const
	{
		return m_InstanceID;
	}

	UUID FSMState::OriginalFSMID() const
	{
		return m_OriginalFSMID;
	}

	void FSMState::SetPropertyValue(FSMData* pFSM, std::string_view name, void* data)
	{
		const size_t propIndex = pFSM->PropertyIndex(name);
		if (propIndex == pFSM->PropertyCount()) return;
		char* propData = &m_PropertyData[propIndex*sizeof(float)];
		std::memcpy(propData, data, sizeof(float));
		m_PropertyDataChanged = true;
	}

	void FSMState::Update()
	{
		if (!m_PropertyDataChanged) return;
		if (m_CurrentState == 0) return;

		AssetManager& assets = m_pModule->GetEngine()->GetAssetManager();

		Resource* pFSMResource = assets.FindResource(m_OriginalFSMID);
		if (!pFSMResource) return;
		FSMData* pFSM = static_cast<FSMData*>(pFSMResource);

		const FSMNode* node = pFSM->Node(m_CurrentState);

		if (m_FirstUpdate)
		{
			if (m_pModule->EntryCallback) m_pModule->EntryCallback(*this, *node);
			m_FirstUpdate = false;
		}

		for (size_t i = 0; i < node->m_Transitions.size(); ++i)
		{
			const UUID transitionID = node->m_Transitions[i];
			const FSMTransition* transition = pFSM->Transition(transitionID);

			if (transition->m_TransitionOp == FSMTransitionOP::Custom)
			{
				/* @todo: Trigger custom condition */
				continue;
			}

			if (transition->m_Property == 0) continue;
			const size_t propIndex = pFSM->PropertyIndex(transition->m_Property);
			if (propIndex == pFSM->PropertyCount()) continue;
			const FSMProperty& prop = pFSM->Property(propIndex);

			const char* propData = &m_PropertyData[propIndex*sizeof(float)];

			bool triggerTransition = false;

			switch (transition->m_TransitionOp)
			{
			case FSMTransitionOP::Trigger:
				if (prop.m_Type != FSMPropertyType::Trigger) continue;
				triggerTransition = *reinterpret_cast<const int*>(propData) > 0;
				break;
			case FSMTransitionOP::On:
				if (prop.m_Type != FSMPropertyType::Bool) continue;
				triggerTransition = *reinterpret_cast<const int*>(propData) != 0;
				break;
			case FSMTransitionOP::Off:
				if (prop.m_Type != FSMPropertyType::Bool) continue;
				triggerTransition = *reinterpret_cast<const int*>(propData) == 0;
				break;
			case FSMTransitionOP::Equal:
				if (prop.m_Type == FSMPropertyType::Bool || prop.m_Type != FSMPropertyType::Trigger) continue;
				triggerTransition = *reinterpret_cast<const float*>(propData) == transition->m_CompareValue;
				break;
			case FSMTransitionOP::Greater:
				if (prop.m_Type == FSMPropertyType::Bool || prop.m_Type != FSMPropertyType::Trigger) continue;
				triggerTransition = *reinterpret_cast<const float*>(propData) > transition->m_CompareValue;
				break;
			case FSMTransitionOP::GreaterOrEqual:
				if (prop.m_Type == FSMPropertyType::Bool || prop.m_Type != FSMPropertyType::Trigger) continue;
				triggerTransition = *reinterpret_cast<const float*>(propData) >= transition->m_CompareValue;
				break;
			case FSMTransitionOP::Less:
				if (prop.m_Type == FSMPropertyType::Bool || prop.m_Type != FSMPropertyType::Trigger) continue;
				triggerTransition = *reinterpret_cast<const float*>(propData) < transition->m_CompareValue;
				break;
			case FSMTransitionOP::LessOrEqual:
				if (prop.m_Type == FSMPropertyType::Bool || prop.m_Type != FSMPropertyType::Trigger) continue;
				triggerTransition = *reinterpret_cast<const float*>(propData) <= transition->m_CompareValue;
				break;
			}

			if (triggerTransition)
			{
				const FSMNode* nextNode = pFSM->Node(transition->m_ToNode);
				if (!nextNode) return;
				const UUID oldState = m_CurrentState;
				m_CurrentState = transition->m_ToNode;
				if (m_pModule->ExitCallback) m_pModule->ExitCallback(*this, *node);
				if (m_pModule->EntryCallback) m_pModule->EntryCallback(*this, *nextNode);
				break;
			}
		}

		/* Reset all triggers */
		for (size_t i = 0; i < pFSM->PropertyCount(); ++i)
		{
			const FSMProperty& prop = pFSM->Property(i);
			if (prop.m_Type != FSMPropertyType::Trigger) continue;
			std::memset(&m_PropertyData[i*sizeof(float)], 0, sizeof(float));
		}

		m_PropertyDataChanged = false;
	}
}
