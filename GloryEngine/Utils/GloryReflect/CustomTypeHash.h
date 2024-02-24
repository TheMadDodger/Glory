#pragma once

namespace Glory::Utils::Reflect
{
	enum class CustomTypeHash: uint32_t
	{
		Basic = 96,
		Struct = 97,
		Enum = 98,
		Array = 99,
	};
}