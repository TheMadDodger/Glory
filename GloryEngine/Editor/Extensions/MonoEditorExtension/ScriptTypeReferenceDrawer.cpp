#include "ScriptTypeReferenceDrawer.h"

#include <PropertyDrawer.h>
#include <EditorApplication.h>
#include <GloryMonoScipting.h>
#include <MonoManager.h>
#include <CoreLibManager.h>
#include <EditorUI.h>

namespace Glory::Editor
{
	constexpr std::string_view Noone = "Noone";

	template<>
	inline bool PropertyDrawerTemplate<ScriptTypeReference>::OnGUI(const std::string& label, ScriptTypeReference* data, uint32_t flags) const
	{
		GloryMonoScipting* pScripting = EditorApplication::GetInstance()->GetEngine()->GetOptionalModule<GloryMonoScipting>();
		const MonoScriptManager& scriptManager = pScripting->GetMonoManager()->GetCoreLibManager()->ScriptManager();
		const int typeIndex = scriptManager.TypeIndexFromHash(data->m_Hash);
		size_t index = typeIndex + 1;

		std::vector<std::string_view> options{ scriptManager.TypeCount() + 1 };
		options[0] = Noone;
		for (size_t i = 0; i < scriptManager.TypeCount(); ++i)
		{
			options[i + 1] = scriptManager.TypeName(i);
		}

		const bool change = EditorUI::InputDropdown(label, options, &index, index == 0 ? Noone : scriptManager.TypeName(index - 1));
		if (change)
			data->m_Hash = index == 0 ? 0 : scriptManager.TypeHash(index - 1);

		return change;
	}
}

namespace YAML
{
	Emitter& YAML::operator<<(Emitter& out, const Glory::ScriptTypeReference& ref)
	{
		out << ref.m_Hash;
		return out;
	}
}
