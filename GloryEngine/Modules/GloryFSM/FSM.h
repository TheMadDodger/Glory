#pragma once
#include <string>
#include <vector>

#include <Resource.h>

#include <Reflection.h>
#include <UUID.h>

REFLECTABLE_ENUM_NS(Glory, FSMPropertyType, Number, Bool, Trigger);
REFLECTABLE_ENUM_NS(Glory, FSMTransitionOP, Trigger, On, Off, Equal, Greater, GreaterOrEqual, Less, LessOrEqual, Custom);

namespace Glory
{
	/** @brief Transition data */
	struct FSMTransition
	{
		/** @brief Constructor */
		FSMTransition() :
			m_Name("New Transition"), m_FromNode(0), m_ToNode(0), m_Property(0),
			m_TransitionOp(FSMTransitionOP::Trigger), m_CompareValue(0.0f) {}
		/** @override
		 * @param name Name of the transition
		 * @param from Node the transition comes from
		 * @param from Node to transition to
		 * @param id ID of the transition
		 */
		FSMTransition(const std::string& name, UUID from, UUID to, UUID id) :
			m_Name(name), m_FromNode(from), m_ToNode(to), m_ID(id), m_Property(0),
			m_TransitionOp(FSMTransitionOP::Trigger), m_CompareValue(0.0f) {}

		REFLECTABLE(FSMTransition,
			(std::string)(m_Name),
			(UUID)(m_FromNode),
			(UUID)(m_ToNode),
			(UUID)(m_Property),
			(FSMTransitionOP)(m_TransitionOp),
			(float)(m_CompareValue)
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

		std::vector<UUID> m_Transitions;
		UUID m_ID;
	};

	/** @brief Property data */
	struct FSMProperty
	{
		/** @brief Constructor */
		FSMProperty() : m_Name("New Property"), m_Type(FSMPropertyType::Number) {}
		/** @override
		 * @param name Name of the property
		 * @param id ID of the property
		 */
		FSMProperty(const std::string& name, FSMPropertyType type, UUID id) : m_Name(name), m_Type(type), m_ID(id) {}

		REFLECTABLE(FSMProperty,
			(std::string)(m_Name),
			(FSMPropertyType)(m_Type)
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
		/** @overload */
		GLORY_API const FSMNode* Node(UUID id) const;
		/** @brief Get a transition in this state machine
		 * @param index Index of the transition to get
		 */
		GLORY_API const FSMTransition& Transition(size_t index) const;
		/** @overload */
		GLORY_API const FSMTransition* Transition(UUID id) const;
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

		/** @brief Find a state/node by name
		 * @param name Name of the state to find
		 */
		GLORY_API const FSMNode* FindNode(std::string_view name) const;
		/** @brief Find a transition by name
		 * @param name Name of the transition to find
		 */
		GLORY_API const FSMTransition* FindTransition(std::string_view name) const;

		/** @brief Find the index of a node/state by id
		 * @param id ID of the node
		 */
		GLORY_API size_t NodeIndex(UUID id) const;
		/** @brief Find the index of a transition by id
		 * @param id ID of the transition
		 */
		GLORY_API size_t TransitionIndex(UUID id) const;

		GLORY_API FSMProperty& NewProperty(const std::string& name, FSMPropertyType type, UUID id);
		GLORY_API size_t PropertyCount() const;
		GLORY_API const FSMProperty& Property(size_t index) const;

	private:
		/** @brief Get a vector containing other resources referenced by this resource */
		virtual void References(Engine*, std::vector<UUID>&) const override;

		virtual void Serialize(BinaryStream& container) const override;
		virtual void Deserialize(BinaryStream& container) override;

	private:
		std::vector<FSMNode> m_Nodes;
		std::vector<FSMTransition> m_Transitions;
		std::vector<FSMProperty> m_Properties;
		size_t m_StartNodeIndex;
	};

	class FSMModule;

	/** @brief Runtime state of a finite state machine */
	class FSMState
	{
	public:
		/** @biref Constructor
		 * @param pModule The FSM module
		 * @param originalFSMID ID of the FSM data resource
		 * @param instanceID ID if this state instance
		 */
		GLORY_API FSMState(FSMModule* pModule, FSMData* pFSM, UUID instanceID);
		/** @biref Set the current state of this machine
		 * @param stateID ID of the state to set
		 */
		GLORY_API void SetCurrentState(UUID stateID);

		/** @brief ID of this state instance */
		GLORY_API UUID ID() const;
		/** @brief ID of the finite state machine data resource */
		GLORY_API UUID OriginalFSMID() const;

	private:
		FSMModule* m_pModule;
		UUID m_OriginalFSMID;
		UUID m_InstanceID;
		UUID m_CurrentState;
		std::vector<char> m_PropertyData;
	};
}