#pragma once

namespace Glory
{
	enum SerializedType : size_t
	{
		ST_Value = 0,
		ST_String = 1,
		ST_Object = 2,
		ST_Asset = 3,

		ST_Basic = 96, // Registered for us in the Reflect library as hash 96
		ST_Struct = 97, // Registered for us in the Reflect library as hash 97
		ST_Enum = 98, // Registered for us in the Reflect library as hash 98
		ST_Array = 99, // Registered for us in the Reflect library as hash 99
	};
}
