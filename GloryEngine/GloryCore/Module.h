#pragma once
#include "Object.h"
#include "ScriptExtensions.h"
#include <typeinfo>
#include "Object.h"
#include "ModuleMetaData.h"

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
	};
}