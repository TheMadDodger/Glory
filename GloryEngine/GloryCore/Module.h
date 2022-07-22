#pragma once
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
		void SetMetaData(const std::filesystem::path& path);

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

	private:
		friend class Engine;
		ModuleMetaData m_MetaData;
	};
}