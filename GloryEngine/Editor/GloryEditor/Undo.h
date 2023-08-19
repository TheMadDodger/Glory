#pragma once
#include "GloryEditor.h"
#include <string>
#include <vector>
#include <UUID.h>
#include <NodeRef.h>

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

		virtual bool Combine(IAction* pOther) { return false; }

	private:
		friend class Undo;
	};

	class Undo
	{
	public:
		static GLORY_EDITOR_API void StartRecord(const std::string& name, UUID uuid = 0, bool continuous = false);
		static GLORY_EDITOR_API void StopRecord();
		static GLORY_EDITOR_API void AddAction(IAction* action);
		static GLORY_EDITOR_API void Clear();

		static GLORY_EDITOR_API void YAMLEdit(Utils::YAMLFileRef& file, const std::filesystem::path& path, YAML::Node oldValue, YAML::Node newValue);

		static GLORY_EDITOR_API void DoUndo();
		static GLORY_EDITOR_API void DoRedo();

		static GLORY_EDITOR_API bool CanUndo();
		static GLORY_EDITOR_API bool CanRedo();

		static GLORY_EDITOR_API size_t GetHistorySize();
		static GLORY_EDITOR_API void ClearHistoryFrom(size_t index);
		static GLORY_EDITOR_API void ClearRewind();

		static GLORY_EDITOR_API const ActionRecord* RecordAt(const size_t index);
		static GLORY_EDITOR_API const size_t CurrentRewindIndex();
		static GLORY_EDITOR_API void JumpTo(size_t historyRewindIndex);

		template<typename T>
		static void ApplyYAMLEdit(Utils::YAMLFileRef& file, const std::filesystem::path& path, const T& oldValue, const T& newValue)
		{
			YAML::Node oldValueNode = YAML::Node(YAML::NodeType::Scalar);
			YAML::Node newValueNode = YAML::Node(YAML::NodeType::Scalar);
			oldValueNode = oldValue;
			newValueNode = newValue;
			file[path].Set(newValue);
			Undo::YAMLEdit(file, path, oldValueNode, newValueNode);
		}

	private:
		Undo();
		virtual ~Undo();

	private:
		static bool m_IsBusy;

		static std::vector<IAction*> m_RecordedActions;
		static std::string m_RecordingName;
		static UUID m_RecordingUUID;
		static bool m_RecordingContinuous;

		static std::vector<ActionRecord> m_ActionRecords;
		static size_t m_RewindIndex;
	};
}
