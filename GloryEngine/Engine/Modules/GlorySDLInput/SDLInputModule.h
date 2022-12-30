#pragma once
#include <InputModule.h>
#include <Glory.h>

namespace Glory
{
	class SDLInputModule : public InputModule
	{
	public:
		GLORY_API SDLInputModule();
		GLORY_API virtual ~SDLInputModule();

		GLORY_MODULE_VERSION_H;

	private:
		virtual void OnInitialize() override;
		virtual void OnCleanup() override;
		virtual void OnUpdate() override;

	private:
		std::hash<std::string> m_Hasher;
	};
}
