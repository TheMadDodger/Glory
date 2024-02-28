#pragma once
#include <UUID.h>

namespace Glory::Editor
{
	class PackagePopup
	{
	public:
		PackagePopup();
		~PackagePopup();

		void Draw();

	private:
		bool m_Open{ false };
		bool m_Canceling{ false };
		UUID m_PackagingStart;
	};
}
