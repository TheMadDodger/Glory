#pragma once
#include <MainWindow.h>

namespace Glory::Editor
{
	class UIMainWindow : public MainWindowTemplate<UIMainWindow>
	{
	private:
		virtual std::string_view Name() override;
		virtual void OnGui(float height) override;
		virtual void Initialize() override;
	};
}
