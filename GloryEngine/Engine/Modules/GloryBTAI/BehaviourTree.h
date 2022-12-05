#pragma once
#include "Blackboard.h"
#include <functional>

enum BehaviourTreeState
{
	RUNNING,
	FAILED,
	SUCCESS,
};

class BT_Node
{
public:
	BT_Node();
	~BT_Node();

	virtual BehaviourTreeState Execute(Glory::Blackboard* pBlackboard) = 0;

	void AddChild(BT_Node* pChild)
	{
		m_pChildren.push_back(pChild);
	}

protected:
	std::vector<BT_Node*> m_pChildren;
};

class BT_Selector : public BT_Node
{
public:
	virtual BehaviourTreeState Execute(Glory::Blackboard* pBlackboard) override
	{
		if (m_CurrentChildIndex >= m_pChildren.size())
		{
			m_CurrentChildIndex = 0;
			return BehaviourTreeState::FAILED;
		}

		BehaviourTreeState state = m_pChildren[m_CurrentChildIndex]->Execute(pBlackboard);
		++m_CurrentChildIndex;

		switch (state)
		{
		case RUNNING:
			return BehaviourTreeState::RUNNING;
		case FAILED:
			return BehaviourTreeState::RUNNING;
		case SUCCESS:
			m_CurrentChildIndex = 0;
			return BehaviourTreeState::SUCCESS;
		}
	}

private:
	size_t m_CurrentChildIndex;
};

class BT_Sequence : public BT_Node
{
public:
	virtual BehaviourTreeState Execute(Glory::Blackboard* pBlackboard) override
	{
		if (m_CurrentChildIndex >= m_pChildren.size())
		{
			m_CurrentChildIndex = 0;
			return BehaviourTreeState::SUCCESS;
		}

		BehaviourTreeState state = m_pChildren[m_CurrentChildIndex]->Execute(pBlackboard);
		++m_CurrentChildIndex;

		switch (state)
		{
		case RUNNING:
			return BehaviourTreeState::RUNNING;
		case FAILED:
			return BehaviourTreeState::FAILED;
		case SUCCESS:
			return BehaviourTreeState::RUNNING;
		}
	}

private:
	size_t m_CurrentChildIndex;
};

inline bool DoesValueExixst(Glory::Blackboard* pBlackboard)
{
	int value;
	return pBlackboard->Get("value", value);
}

class BT_Leaf : public BT_Node
{
public:
	BT_Leaf(std::function<bool(Glory::Blackboard*)> func);
	~BT_Leaf();

	virtual BehaviourTreeState Execute(Glory::Blackboard* pBlackboard) override
	{
		if (m_Func(pBlackboard))
		{
			return BehaviourTreeState::SUCCESS;
		}
	}

private:
	std::function<bool(Glory::Blackboard*)> m_Func;
};

class BehaviourTree
{
public:
	BehaviourTree(BT_Node* pRootNode) : m_pRootNode(pRootNode), m_pBlackboard(new Glory::Blackboard()) {}
	~BehaviourTree();

	void Execute()
	{
		BehaviourTreeState state = BehaviourTreeState::RUNNING;
		while (state == BehaviourTreeState::RUNNING)
		{
			state = m_pRootNode->Execute(m_pBlackboard);
		}
	}

private:
	BT_Node* m_pRootNode;
	Glory::Blackboard* m_pBlackboard;
};

inline void CreateTree()
{
	BT_Node* pRootNode = new BT_Selector();

	BT_Node* pSeq1Node = new BT_Sequence();
	BT_Node* pSeq2Node = new BT_Sequence();
	pRootNode->AddChild(pSeq1Node);
	pRootNode->AddChild(pSeq2Node);

	pSeq1Node->AddChild(new BT_Leaf(DoesValueExixst));

	BehaviourTree* pTree = new BehaviourTree(pRootNode);

	pTree->Execute();
}