#pragma once
#include <MainWindow.h>

#include <UUID.h>
#include <EntityID.h>

#include <glm/vec2.hpp>

namespace Glory
{
	class UIDocument;
}

namespace Glory::Editor
{
	class UIMainWindow : public MainWindowTemplate<UIMainWindow>
	{
	public:
		UIMainWindow();
		virtual ~UIMainWindow();

		void SetDocument(UUID documentID);

		UUID CurrentDocumentID() const;
		UIDocument* CurrentDocument();
		const glm::uvec2& Resolution() const;
		UUID& SelectedEntity();
		UIDocument* FindEditingDocument(UUID uuid) const;

		void SetResolution(const glm::uvec2& resolution);

	private:
		virtual std::string_view Name() override;
		virtual void OnGui(float height) override;
		virtual void Initialize() override;

	private:
		UUID m_EditingDocument;
		std::vector<UIDocument*> m_pDocuments;
		size_t m_EditingDocumentIndex;
		glm::uvec2 m_Resolution;
		UUID m_SelectedEntity;
	};
}
