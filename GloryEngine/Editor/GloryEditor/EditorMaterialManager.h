#pragma once
#include <ThreadedVar.h>
#include <UUID.h>

namespace Glory
{
	class MaterialData;
	class MaterialInstanceData;
	class Engine;

namespace Utils
{
	struct YAMLFileRef;
	struct NodeValueRef;
}

namespace Editor
{
	struct AssetCallbackData;

	class EditorMaterialManager
	{
	public:
		EditorMaterialManager(Engine* pEngine);
		~EditorMaterialManager();

		void Start();
		void Stop();

		void LoadIntoMaterial(Utils::YAMLFileRef& file, MaterialData* pMaterial) const;

	private:
		void AssetAddedCallback(const AssetCallbackData& callback);
		void AssetUpdatedCallback(const AssetCallbackData& callback);

		void ReadShadersInto(Utils::NodeValueRef& shaders, MaterialData* pMaterial) const;
		void ReadPropertiesInto(Utils::NodeValueRef& properties, MaterialData* pMaterial) const;

	private:
		ThreadedUMap<UUID, MaterialData*> m_pMaterialDatas;

		Engine* m_pEngine;
		UUID m_AssetRegisteredCallback;
		UUID m_AssetUpdatedCallback;
	};
}
}