#pragma once
#include <functional>

namespace Glory::Editor
{
	class ListView
	{
	public:
		ListView(const char* label);

	public:
		bool Draw(size_t elementCount);

		std::function<void(size_t)> OnDrawElement;
		std::function<void()> OnAdd;
		std::function<void(size_t)> OnResize;
		std::function<void(int)> OnRemove;

	private:
		const char* m_Label;
	};
}
