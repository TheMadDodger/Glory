#pragma once
#include <Module.h>
#include <Glory.h>

#include <map>
#include <string_view>

namespace Glory
{
	class FSMData;
	class FSMState;
	struct FSMNode;

	class LocalizeModule : public Module
	{
	public:
		GLORY_API LocalizeModule();
		GLORY_API virtual ~LocalizeModule();

		GLORY_API virtual const std::type_info& GetModuleType() override;

		void LoadStringTable(UUID tableID);
		void UnloadStringTable(UUID tableID);
		void Clear();

		GLORY_MODULE_VERSION_H(1,0,0);

	private:
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void Update() override;
		virtual void Cleanup() override;

		virtual void LoadSettings(ModuleSettings& settings) override;

	private:
		std::map<std::string_view, std::string_view> m_LoadedStrings;
	};
}
