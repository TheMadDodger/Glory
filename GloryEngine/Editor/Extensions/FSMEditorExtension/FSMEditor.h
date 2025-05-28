#pragma once
#include <MainWindow.h>

#include <UUID.h>

namespace Glory::Editor
{
    class FSMEditor : public MainWindowTemplate<FSMEditor>
    {
	public:
		FSMEditor();
		virtual ~FSMEditor();

		void SetFSM(UUID fsmID);
		UUID CurrentFSMID() const;
		UUID& SelectedNode();
		UUID& ActiveNode();
		UUID& LastActiveNode();
		UUID& DebuggingInstance();

		void DeleteNode(UUID nodeID);

	private:
		virtual std::string_view Name() override;
		virtual void OnGui(float height) override;
		virtual void Initialize() override;
		virtual void OnUpdate() override;

	private:
		UUID m_EditingFSM;
		UUID m_SelectedNode;
		std::vector<UUID> m_OpenFSMs;
		size_t m_EditingFSMIndex;
		UUID m_ActiveNodeID;
		UUID m_LastActiveNode;
		UUID m_DebuggingInstance;
    };
}
