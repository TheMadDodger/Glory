//#pragma once
//#include <SerializedProperty.h>
//
//#define PROPERTY_DRAWER(x) Spartan::Editor::PropertyDrawer::RegisterPropertyDrawer<x>()
//
//namespace Spartan::Editor
//{
//	struct AssetReferencePropertyDrawer;
//
//	class PropertyDrawer
//	{
//	public:
//		PropertyDrawer();
//		virtual ~PropertyDrawer();
//		virtual bool OnGUI(Serialization::SerializedProperty& prop) const;
//
//		template<class T>
//		static void RegisterPropertyDrawer()
//		{
//			m_PropertyDrawers.push_back(new T());
//		}
//
//		static bool DrawProperty(Serialization::SerializedProperty& prop);
//
//	public:
//		virtual const std::type_info& GetPropertyType() const;
//
//	private:
//		static void Cleanup();
//
//	private:
//		friend class EditorApp;
//		static vector<PropertyDrawer*> m_PropertyDrawers;
//		static AssetReferencePropertyDrawer* m_pAssetReferencePropDrawer;
//	};
//
//	template<typename PropertyType>
//	class PropertyDrawerTemplate : public PropertyDrawer
//	{
//	protected:
//		virtual const std::type_info& GetPropertyType() const override { return typeid(PropertyType); }
//	};
//}
