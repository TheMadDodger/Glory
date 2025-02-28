#pragma once
#include <string_view>

namespace Glory::Editor
{
	class Toolbar;

	class MainWindow
	{
	public:
		virtual std::string_view Name() = 0;
		virtual void OnGui(float height) = 0;
		virtual void Initialize() = 0;
	};

	class SceneEditingMainWindow : public MainWindow
	{
	public:
		SceneEditingMainWindow();
		virtual ~SceneEditingMainWindow();

		static const float TOOLBAR_SIZE;

		virtual std::string_view Name() override;
		virtual void OnGui(float height) override;
		virtual void Initialize() override;

	private:
		void Dockspace(float height);

	private:
		Toolbar* m_pToolbar;
	};

	class TestMainWindow : public MainWindow
	{
	public:
		virtual std::string_view Name() override;
		virtual void OnGui(float height) override;
		virtual void Initialize() override;
	};
}
