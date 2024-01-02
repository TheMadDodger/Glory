#include "Undo.h"
#include "Debug.h"
#include <YAMLAction.h>

namespace Glory::Editor
{
	std::vector<IAction*> Undo::m_RecordedActions;
	std::string Undo::m_RecordingName;
	bool Undo::m_RecordingContinuous;

	std::vector<ActionRecord> Undo::m_ActionRecords;
	size_t Undo::m_RewindIndex = 0;
	UUID Undo::m_RecordingUUID;

	bool Undo::m_IsBusy = false;

	void Undo::StartRecord(const std::string& name, UUID uuid, bool continuous)
	{
		if (m_IsBusy) return;

		if (m_RecordingName != "")
		{
			m_pEngine->GetDebug().LogError("Recording already started, call StopRecord before starting a new one! Name: " + m_RecordingName);
			return;
		}

		m_RecordingName = name;
		m_RecordingUUID = uuid;
		m_RecordingContinuous = continuous;
	}

	void Undo::StopRecord()
	{
		if (m_IsBusy) return;

		if (m_RecordingName == "")
		{
			m_pEngine->GetDebug().LogError("Recording not yet started, call StartRecord to start recording!");
			return;
		}

		if (m_RecordedActions.size() <= 0)
		{
			m_RecordingName = "";
			m_RecordedActions.clear();
			return;
		}

		if (m_RewindIndex != 0) ClearRewind();

		size_t size = m_ActionRecords.size();
		m_ActionRecords.push_back(ActionRecord(m_RecordingName, m_RecordingUUID));
		m_ActionRecords[size].Actions = m_RecordedActions;

		m_RecordingName = "";
		m_RecordedActions.clear();
	}

	void Undo::AddAction(IAction* action)
	{
		if (m_RecordingName == "")
		{
			delete action;
			return;
		}

		if (m_RecordingContinuous && m_ActionRecords.size())
		{
			const size_t index = m_ActionRecords.size() - m_RewindIndex - 1;
			ActionRecord& record = m_ActionRecords[index];
			if (record.Name == m_RecordingName && record.ObjectID == m_RecordingUUID)
			{
				for (size_t i = 0; i < record.Actions.size(); ++i)
				{
					if (record.Actions[i]->Combine(action))
					{
						delete action;
						return;
					}
				}
			}
		}

		m_RecordedActions.push_back(action);
	}

	void Undo::Clear()
	{
		for (size_t i = 0; i < m_ActionRecords.size(); ++i)
		{
			for (size_t j = 0; j < m_ActionRecords[i].Actions.size(); ++j)
			{
				delete m_ActionRecords[i].Actions[j];
			}
			m_ActionRecords[i].Actions.clear();
		}
		m_ActionRecords.clear();
		m_RewindIndex = 0;
	}

	void Undo::YAMLEdit(Utils::YAMLFileRef& file, const std::filesystem::path& path, YAML::Node oldValue, YAML::Node newValue)
	{
		YAMLAction* pAction = new YAMLAction(file, path, oldValue, newValue);
		AddAction(pAction);
		file[path].Set(newValue);
	}

	void Undo::DoUndo()
	{
		if (!CanUndo()) return;
		m_IsBusy = true;
		++m_RewindIndex;
		size_t index = m_ActionRecords.size() - m_RewindIndex;
		for (int i = (int)m_ActionRecords[index].Actions.size() - 1; i >= 0; --i)
		{
			m_ActionRecords[index].Actions[i]->OnUndo(m_ActionRecords[index]);
		}
		m_IsBusy = false;
	}

	void Undo::DoRedo()
	{
		if (!CanRedo()) return;
		m_IsBusy = true;
		size_t index = m_ActionRecords.size() - m_RewindIndex;
		for (size_t i = 0; i < m_ActionRecords[index].Actions.size(); i++)
		{
			m_ActionRecords[index].Actions[i]->OnRedo(m_ActionRecords[index]);
		}
		--m_RewindIndex;
		m_IsBusy = false;
	}

	bool Undo::CanUndo()
	{
		if (m_IsBusy) return false;
		return (m_ActionRecords.size() - m_RewindIndex > 0);
	}

	bool Undo::CanRedo()
	{
		if (m_IsBusy) return false;
		return m_RewindIndex > 0;
	}

	size_t Undo::GetHistorySize()
	{
		return m_ActionRecords.size();
	}

	void Undo::ClearHistoryFrom(size_t index)
	{
		size_t currentSize = m_ActionRecords.size();
		if (index >= currentSize) return;
		for (size_t i = index; i < m_ActionRecords.size(); i++)
		{
			for (size_t j = 0; j < m_ActionRecords[i].Actions.size(); j++)
			{
				delete m_ActionRecords[i].Actions[j];
			}
			m_ActionRecords[i].Actions.clear();
		}
		m_ActionRecords.erase(m_ActionRecords.begin() + index, m_ActionRecords.end());

		if (currentSize - m_RewindIndex <= index) return;
		m_RewindIndex = 0;
	}

	void Undo::ClearRewind()
	{
		size_t last = m_ActionRecords.size() - 1;
		for (size_t i = 0; i < m_RewindIndex; i++)
		{
			size_t index = last - i;
			for (size_t i = 0; i < m_ActionRecords[index].Actions.size(); i++)
			{
				delete m_ActionRecords[index].Actions[i];
			}
			m_ActionRecords[index].Actions.clear();
		}

		m_ActionRecords.erase(m_ActionRecords.end() - m_RewindIndex, m_ActionRecords.end());
		m_RewindIndex = 0;
	}

	const ActionRecord* Undo::RecordAt(const size_t index)
	{
		return &m_ActionRecords[index];
	}

	const size_t Undo::CurrentRewindIndex()
	{
		return m_RewindIndex;
	}

	void Undo::JumpTo(size_t historyRewindIndex)
	{
		if (historyRewindIndex > m_ActionRecords.size()) return;
		size_t last = m_ActionRecords.size() - 1;
		int diff = (int)(m_RewindIndex - historyRewindIndex);

		for (size_t i = 0; i < std::abs(diff); i++)
		{
			if (diff < 0) DoUndo();
			else DoRedo();
		}
	}

	Undo::Undo()
	{
	}

	Undo::~Undo()
	{
	}

	ActionRecord::ActionRecord(const std::string name, UUID uuid) : Name(name), ObjectID(uuid)
	{
	}

	//const ActionRecord& ActionRecord::operator=(const ActionRecord&)
	//{
	//	
	//}

	ActionRecord::~ActionRecord()
	{
	}
}
