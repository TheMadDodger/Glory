#include "ShapePropertyDrawer.h"
#include "EditorUI.h"

namespace Glory::Editor
{
	class ShapeDrawers
	{
	public:
		static bool Draw(ShapeProperty* shapeProperty)
		{
			if (SHAPE_DRAWERS.find(shapeProperty->m_ShapeType) == SHAPE_DRAWERS.end())
			{
				ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "%s", "Unknown shape");
				return false;
			}
			return SHAPE_DRAWERS.at(shapeProperty->m_ShapeType)->OnGUI(shapeProperty);
		}

		static void SetShape(ShapeProperty* shapeProperty)
		{
			if (SHAPE_DRAWERS.find(shapeProperty->m_ShapeType) == SHAPE_DRAWERS.end()) return;
			SHAPE_DRAWERS.at(shapeProperty->m_ShapeType)->SetShapeInternal(shapeProperty);
		}

		static void Cleanup()
		{
			for (auto drawer : SHAPE_DRAWERS)
			{
				delete drawer.second;
			}
			SHAPE_DRAWERS.clear();
		}

	protected:
		virtual bool OnGUI(ShapeProperty* shapeProperty) const = 0;
		virtual void SetShapeInternal(ShapeProperty* shapeProperty) const = 0;

	private:
		static std::map<ShapeType, ShapeDrawers*> SHAPE_DRAWERS;
	};

	template<typename T>
	class ShapeDrawer : public ShapeDrawers
	{
		bool OnGUI(ShapeProperty* shapeProperty) const override
		{
			T* data = shapeProperty->ShapePointer<T>();

			const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(ResourceType::GetHash<T>());
			bool change = false;
			for (size_t i = 0; i < pTypeData->FieldCount(); ++i)
			{
				const GloryReflect::FieldData* pField = pTypeData->GetFieldData(i);
				size_t offset = pField->Offset();
				void* pAddress = (void*)((char*)(data)+offset);
				change |= PropertyDrawer::DrawProperty(pField, pAddress, 0);
			}

			return change;
		}

		void SetShapeInternal(ShapeProperty* shapeProperty) const override
		{
			shapeProperty->SetShape(T());
		}
	};

#define SHAPE_DRAWER(shape) {ShapeType::shape, new ShapeDrawer<shape>()}
	std::map<ShapeType, ShapeDrawers*> ShapeDrawers::SHAPE_DRAWERS = {
		SHAPE_DRAWER(Sphere),
		SHAPE_DRAWER(Box),
	};

	ShapePropertyDrawer::~ShapePropertyDrawer()
	{
		ShapeDrawers::Cleanup();
	}

	bool ShapePropertyDrawer::Draw(const std::string& label, void* data, uint32_t typeHash, uint32_t flags) const
	{
		ShapeProperty oldValue = *(ShapeProperty*)data;
		if (OnGUI(label, (ShapeProperty*)data, flags))
		{
			ShapeProperty newValue = *(ShapeProperty*)data;
			ValueChangeAction* pAction = new ValueChangeAction(PropertyDrawer::GetCurrentFieldStack());
			pAction->SetOldValue(&oldValue);
			pAction->SetNewValue(&newValue);
			Undo::AddAction(pAction);
			return true;
		}
		return false;
	}

	bool ShapePropertyDrawer::Draw(const std::string& label, std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, uint32_t flags) const
	{
		ShapeProperty value;
		memcpy((void*)&value, (void*)&buffer[offset], size);
		ShapeProperty originalValue = value;
		if (OnGUI(label, &value, flags))
		{
			Undo::AddAction(new PropertyAction<ShapeProperty>(label, originalValue, value));
		}
		if (originalValue == value) return false;
		memcpy((void*)&buffer[offset], (void*)&value, size);
		return true;
	}

	bool ShapePropertyDrawer::OnGUI(const std::string& label, ShapeProperty* data, uint32_t flags) const
	{
		const ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (ImGui::TreeNodeEx("val", node_flags, EditorUI::MakeCleanName(label).data()))
		{
			if (EditorUI::InputEnum<ShapeType>("Shape Type", &data->m_ShapeType))
			{
				/* Assign new default shape */
				ShapeDrawers::SetShape(data);
			}

			ShapeDrawers::Draw(data);
			ImGui::TreePop();
		}
		return false;
	}
}
