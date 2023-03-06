#pragma once
#include "Object.h"
#include "ScriptExtensions.h"
#include "Object.h"
#include "ModuleMetaData.h"
#include "IScriptExtender.h"
#include "Versioning.h"
#include "ModuleSettings.h"

#include <typeinfo>

#define GLORY_MODULE_H																						\
extern "C" GLORY_API Glory::Module * OnLoadModule(Glory::GloryContext * pContext);							\
extern "C" GLORY_API const Glory::Version& ModuleVersion();

#define GLORY_MODULE_CPP(moduleName)																		\
Glory::Module* OnLoadModule(Glory::GloryContext* pContext)													\
{																											\
	Glory::GloryContext::SetContext(pContext);																\
	return new Glory::moduleName();																			\
}																											\
																											\
const Glory::Version& ModuleVersion()																		\
{																											\
	return Glory::moduleName::Version;																		\
}

#define GLORY_MODULE_VERSION_H																				\
static const Glory::Version Version;																		\
virtual const Glory::Version& ModuleVersion() const override { return Version; };


#define GLORY_MODULE_VERSION_CPP(moduleName, major, minor)													\
const Glory::VersionValue MODULE_VERSION_DATA[] = {															\
	{"Major", TOSTRING(major)},																				\
	{"Minor", TOSTRING(minor)},																				\
};																											\
																											\
const Glory::Version moduleName::Version{ MODULE_VERSION_DATA, 2 };

namespace Glory
{
	class Engine;
	class GloryContext;

	class Module : public Object
	{
	public:
		Module();
		virtual ~Module();

		virtual const std::type_info& GetModuleType() = 0;

		Engine* GetEngine();

		const ModuleMetaData& GetMetaData() const;
		void SetMetaData(const ModuleMetaData& metaData);

		const std::filesystem::path& GetPath() const;

		bool GetResourcePath(const std::string& resource, std::filesystem::path& path) const;

		void AddScriptingExtender(IScriptExtender* pScriptExtender);

		virtual const Version& ModuleVersion() const;

		ModuleSettings& Settings();

		void LoadSettings(const std::filesystem::path& settingsFile);

	protected:
		virtual void LoadSettings(ModuleSettings& settings) {};
		virtual void Initialize() = 0;
		virtual void PostInitialize() {};
		virtual void Cleanup() = 0;

		virtual void OnBeginFrame() {};
		virtual void OnEndFrame() {};

		virtual void OnGameThreadFrameStart() {};
		virtual void OnGameThreadFrameEnd() {};
		virtual void OnGraphicsThreadFrameStart() {};
		virtual void OnGraphicsThreadFrameEnd() {};

		virtual void Update() {};
		virtual void Draw() {};

		virtual bool HasPriority();

	protected:
		Engine* m_pEngine;
		std::filesystem::path m_ModulePath;

	private:
		friend class Engine;
		friend class ScriptingBinder;
		ModuleMetaData m_MetaData;
		ModuleSettings m_Settings;
		std::vector<IScriptExtender*> m_pScriptingExtender;
	};
}
