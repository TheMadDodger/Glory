#include "FSM.h"

namespace Glory
{
	FSMData::FSMData()
	{
		APPEND_TYPE(FSMData);
	}

	FSMData::~FSMData()
	{
	}

	void FSMData::References(Engine*, std::vector<UUID>&) const {}

	void FSMData::Serialize(BinaryStream& container) const
	{
		/* @todo */
	}

	void FSMData::Deserialize(BinaryStream& container)
	{
		/* @todo */
	}
}
