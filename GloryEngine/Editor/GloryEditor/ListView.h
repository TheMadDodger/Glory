#pragma once
#include <GloryEditor.h>

#include <functional>

namespace Glory::Editor
{
	class ListView
	{
	public:
		GLORY_EDITOR_API ListView(const char* label);

	public:
		GLORY_EDITOR_API bool Draw(size_t elementCount);

		std::function<void(size_t)> OnDrawElement = NULL;
		std::function<void()> OnAdd = NULL;
		std::function<void(size_t)> OnResize = NULL;
		std::function<void(int)> OnRemove = NULL;

	private:
		const char* m_Label;
	};
}
