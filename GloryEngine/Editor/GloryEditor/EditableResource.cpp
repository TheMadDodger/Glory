#include "EditableResource.h"

namespace Glory::Editor
{
	EditableResource::EditableResource()
	{
		APPEND_TYPE(EditableResource);
	}

	void EditableResource::Save()
	{
		if (!m_Dirty) return;

		OnSave();
		m_Dirty = false;
		m_WasSaved = true;
	}

	void EditableResource::SetDirty(bool dirty)
	{
		m_Dirty = dirty;
	}

	bool EditableResource::IsDirty()
	{
		return m_Dirty;
	}
}
