#pragma once
#include "Object.h"
#include "ScriptExtensions.h"
#include <typeinfo>
#include "Object.h"
#include "ModuleMetaData.h"
#include "IScriptExtender.h"
#include "Versioning.h"

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

/* Log a message if the module is not present */
#define REQUIRE_MODULE_MESSAGE(engine, moduleName, message, level, returnValue)								\
moduleName* p##moduleName = engine->Get##moduleName();														\
if(!p##moduleName)																							\
{																											\
	Debug::Log##level(message);																				\
	return returnValue;																						\
}

/* Check if a module is present, if not return the returnValue */
#define REQUIRE_MODULE(engine, moduleName, returnValue)														\
moduleName* p##moduleName = engine->Get##moduleName();														\
if(!p##moduleName) return returnValue;

/* Call a method on a function, but only if the module is present, return otherwise */
#define REQUIRE_MODULE_CALL(engine, moduleName, func, returnValue)											\
REQUIRE_MODULE(engine, moduleName, returnValue)																\
p##moduleName->func;

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

	protected:
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
		std::vector<IScriptExtender*> m_pScriptingExtender;
	};
}
