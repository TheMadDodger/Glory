#pragma once
#include "Editor.h"
#include "EditableResource.h"

#include <MaterialData.h>

namespace Glory::Editor
{
	class MaterialEditor : public EditorTemplate<MaterialEditor, YAMLResource<MaterialData>>
	{
	public:
		MaterialEditor();
		virtual ~MaterialEditor();
		virtual bool OnGUI() override;

	private:
		bool PropertiesGUI(YAMLResource<MaterialData>* pMaterial, MaterialData* pMaterialData);
	};

	class StaticMaterialEditor : public EditorTemplate<StaticMaterialEditor, MaterialData>
	{
	public:
		StaticMaterialEditor();
		virtual ~StaticMaterialEditor();
		virtual bool OnGUI() override;

	private:
		void PropertiesGUI(MaterialData* pMaterialData);
	};
}
