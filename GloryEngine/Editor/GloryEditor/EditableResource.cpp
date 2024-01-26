#include "EditableResource.h"

namespace Glory::Editor
{
	EditableResource::EditableResource()
	{
		APPEND_TYPE(EditableResource);
	}

	void EditableResource::Save()
	{
		OnSave();
		m_Dirty = false;
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
