#pragma once
#include <Undo.h>

#include <string_view>

namespace Glory
{
	class Engine;
	class UIDocument;
	namespace Utils
	{
		struct YAMLFileRef;
	}
}

namespace Glory::Editor
{
    class AddUIElementAction : public IAction
    {
	public:
		AddUIElementAction(uint32_t type, UUID uuid, UUID parent, size_t siblingIndex);
		virtual ~AddUIElementAction() = default;

		static UUID AddElement(Engine* pEngine, UIDocument* pDocument, Utils::YAMLFileRef& file,
			std::string_view name, uint32_t type, UUID parent, size_t siblingIndex);

	private:
		void OnUndo(const ActionRecord& actionRecord) override;
		void OnRedo(const ActionRecord& actionRecord) override;

	private:
		const uint32_t m_Type;
		const UUID m_ID;
		const UUID m_Parent;
		const size_t m_SiblingIndex;
    };
}
