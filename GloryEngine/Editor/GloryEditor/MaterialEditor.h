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

		static const char* GetMaterialError(YAMLResource<MaterialData>* pMaterial);

	private:
		bool ShaderGUI(YAMLResource<MaterialData>* pMaterial);
		bool PropertiesGUI(YAMLResource<MaterialData>* pMaterial, MaterialData* pMaterialData);
	};
}
