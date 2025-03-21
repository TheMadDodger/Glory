#pragma once
#include <EditorWindow.h>

namespace Glory::Editor
{
	class EnvironmentGenerator : public EditorWindowTemplate<EnvironmentGenerator>
	{
	public:
		EnvironmentGenerator();
		~EnvironmentGenerator();

	private:
		virtual void OnGUI() override;
		virtual void OnOpen() override;
		virtual void OnClose() override;

		void EnvironmentPass();

		UUID m_CurrentCubemap;
		bool m_Generate;
	};
}
