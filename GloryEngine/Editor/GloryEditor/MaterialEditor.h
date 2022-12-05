#pragma once
#include "Editor.h"
#include <MaterialData.h>

namespace Glory::Editor
{
	class MaterialEditor : public EditorTemplate<MaterialEditor, MaterialData>
	{
	public:
		MaterialEditor();
		virtual ~MaterialEditor();
		virtual bool OnGUI() override;

	private:
		void ShaderGUI(MaterialData* pMaterial);
		bool PropertiesGUI(MaterialData* pMaterial);

		static void UpdateMaterial(MaterialData* pMaterial);
	};
}
