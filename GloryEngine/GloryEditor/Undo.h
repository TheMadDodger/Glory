#pragma once
#include "GloryEditor.h"
#include <string>
#include <vector>
#include <UUID.h>

namespace Glory::Editor
{
	class IAction;

	struct ActionRecord
	{
		ActionRecord(const std::string name, UUID uuid);
		virtual ~ActionRecord();

		std::vector<IAction*> Actions;
		std::string Name;
		UUID ObjectID;

	private:
		//ActionRecord(const ActionRecord&&) = delete;
		//const ActionRecord& operator=(const ActionRecord&) = delete;
	};

	class IAction
	{
	protected:
		virtual void OnUndo(const ActionRecord& actionRecord) = 0;
		virtual void OnRedo(const ActionRecord& actionRecord) = 0;

	private:
		friend class Undo;
	};

	class Undo
	{
	public:
		static GLORY_EDITOR_API void StartRecord(const std::string& name, UUID uuid = 0);
		static GLORY_EDITOR_API void StopRecord();
		static GLORY_EDITOR_API void AddAction(IAction* action);
		static GLORY_EDITOR_API void Clear();

		static GLORY_EDITOR_API void DoUndo();
		static GLORY_EDITOR_API void DoRedo();

		static GLORY_EDITOR_API bool CanUndo();
		static GLORY_EDITOR_API bool CanRedo();

	private:
		static GLORY_EDITOR_API void ClearRewind();

	private:
		Undo();
		virtual ~Undo();

	private:
		static bool m_IsBusy;

		static std::vector<IAction*> m_RecordedActions;
		static std::string m_RecordingName;
		static UUID m_RecordingUUID;

		static std::vector<ActionRecord> m_ActionRecords;
		static size_t m_RewindIndex;
	};
}
