#include "Script.h"

namespace Glory
{
	Script::Script()
	{
		APPEND_TYPE(Script);
	}

	Script::Script(FileData* pFileData) : FileData(pFileData)
	{
		APPEND_TYPE(Script);
	}

	Script::~Script()
	{
	}
}
