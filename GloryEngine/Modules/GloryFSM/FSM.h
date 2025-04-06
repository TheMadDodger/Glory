#pragma once
#include <string>
#include <vector>

#include <Resource.h>

#include <Reflection.h>
#include <UUID.h>

namespace Glory
{
	/** @brief Transition data */
	struct FSMTransition
	{
		/** @brief Constructor */
		FSMTransition() :
			m_Name("New Transition"), m_FromNode(0), m_ToNode(0) {}
		/** @override
		 * @param name Name of the transition
		 * @param from Node the transition comes from
		 * @param from Node to transition to
		 * @param id ID of the transition
		 */
		FSMTransition(const std::string& name, UUID from, UUID to, UUID id) :
			m_Name(name), m_FromNode(from), m_ToNode(to), m_ID(id) {}

		REFLECTABLE(FSMTransition,
			(std::string)(m_Name),
			(UUID)(m_FromNode),
			(UUID)(m_ToNode)
		);

		UUID m_ID;
	};

	/** @brief Node data */
	struct FSMNode
	{
		/** @brief Constructor */
		FSMNode() : m_Name("New Node") {}
		/** @override
		 * @param name Name of the node
		 * @param id ID of the node
		 */
		FSMNode(const std::string& name, UUID id) : m_Name(name), m_ID(id) {}

		REFLECTABLE(FSMNode,
			(std::string)(m_Name)
		);

		UUID m_ID;
	};

	/** @brief FSM resource */
	class FSMData : public Resource
	{
	public:
		/** @brief Constructor */
		GLORY_API FSMData();
		/** @brief Destructor */
		GLORY_API virtual ~FSMData();

		/** @brief Get number of nodes in this state machine */
		GLORY_API size_t NodeCount() const;
		/** @brief Get number of transitions in this state machine */
		GLORY_API size_t TransitionCount() const;
		/** @brief Get a node in this state machine
		 * @param index Index of the node to get
		 */
		GLORY_API const FSMNode& Node(size_t index) const;
		/** @brief Get a transition in this state machine
		 * @param index Index of the transition to get
		 */
		GLORY_API const FSMTransition& Transition(size_t index) const;
		/** @brief Add a new node to the state machine
		 * @param name Name of the state
		 * @param id ID of the node
		 */
		GLORY_API FSMNode& NewNode(const std::string& name, UUID id=UUID());
		/** @brief Add a new transition to the state machine
		 * @param name Name of the transition
		 * @param from Node the transition comes from
		 * @param from Node to transition to
		 * @param id ID of the transition
		 */
		GLORY_API FSMTransition& NewTransition(const std::string& name, UUID from, UUID to, UUID id=UUID());
		/** @brief Index of the node the state machine starts in */
		GLORY_API size_t StartNodeIndex() const;
		/** @brief ID of the node the state machine starts in */
		GLORY_API UUID StartNodeID() const;
		/** @brief Set the node this state machine starts in
		 * @param index Index of the starting node
		 */
		GLORY_API void SetStartNodeIndex(size_t index);

		GLORY_API FSMNode* FindNode(std::string_view name);

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

	class FSMModule;
	class FSMState
	{
	public:
		GLORY_API FSMState(FSMModule* pModule, UUID originalFSMID, UUID instanceID);
		GLORY_API void SetCurrentState(UUID stateID);

	private:
		FSMModule* m_pModule;
		UUID m_OriginalFSMID;
		UUID m_InstanceID;
		UUID m_CurrentState;
	};
}