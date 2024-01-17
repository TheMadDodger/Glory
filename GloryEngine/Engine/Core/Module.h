#pragma once
#include "Object.h"
#include "ModuleMetaData.h"
#include "ModuleSettings.h"
#include "Version.h"

#include <typeinfo>

#define GLORY_MODULE_H																						\
extern "C" GLORY_API Glory::Module * OnLoadModule();														\
extern "C" GLORY_API const char* ModuleVersion();

#define GLORY_MODULE_CPP(moduleName)																		\
Glory::Module* OnLoadModule()																				\
{																											\
	return new Glory::moduleName();																			\
}																											\
																											\
const char* ModuleVersion()																					\
{																											\
	return Glory::moduleName::VersionStr;																	\
}

#define GLORY_MODULE_VERSION_CPP(moduleName)																\
const Glory::Version Glory::moduleName::Version = Version::Parse(VersionStr);

#define GLORY_MODULE_VERSION_H(major, minor, subMinor)														\
static constexpr char* VersionStr = TOSTRING(major.minor.subMinor);											\
static const Glory::Version Version;																		\
virtual const Glory::Version& ModuleVersion() const override { return Version; };

/* Log a message if the module is not present */
#define REQUIRE_MODULE_MESSAGE(engine, moduleName, message, level, returnValue)								\
moduleName* p##moduleName = engine->GetMainModule<moduleName>();											\
if(!p##moduleName)																							\
{																											\
	engine->GetDebug().Log##level(message);																	\
	return returnValue;																						\
}

/* Check if a module is present, if not return the returnValue */
#define REQUIRE_MODULE(engine, moduleName, returnValue)														\
moduleName* p##moduleName = engine->GetMainModule<moduleName>();											\
if(!p##moduleName) return returnValue;

/* Call a method on a function, but only if the module is present, return otherwise */
#define REQUIRE_MODULE_CALL(engine, moduleName, func, returnValue)											\
REQUIRE_MODULE(engine, moduleName, returnValue)																\
p##moduleName->func;

namespace Glory
{
	class Engine;

	class IScriptExtender;

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
		bool m_IsInitialized;

	private:
		friend class Engine;
		friend class ScriptingBinder;
		ModuleMetaData m_MetaData;
		ModuleSettings m_Settings;
		std::vector<IScriptExtender*> m_pScriptingExtender;
	};
}
