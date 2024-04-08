using System.Collections.Generic;
using System.Reflection;
using System;

namespace GloryEngine.Editor
{
    public class Editor
    {
        #region Static Fields

        private static Dictionary<object, Editor> _activeEditors = new Dictionary<object, Editor>();

        #endregion

        #region Internal Methods

        internal static bool DrawInspectorGUI(object @object)
        {
            if (@object == null)
            {
                Debug.LogError("Object is null!");
                return false;
            }

            Editor editor = GetEditor(@object);
            if (editor == null) return false;
            return editor.OnGui();
        }

        internal static Editor GetEditor(object @object)
        {
            if (_activeEditors.ContainsKey(@object)) return _activeEditors[@object];

            foreach (var assembly in AppDomain.CurrentDomain.GetAssemblies())
            {
                foreach (Type type in assembly.GetTypes())
                {
                    ObjectEditorAttribute attribute = type.GetCustomAttribute<ObjectEditorAttribute>();
                    if (attribute == null) continue;
                    if (attribute.EditedType != @object.GetType()) continue;
                    Editor editor = Activator.CreateInstance(type) as Editor;
                    editor._target = @object;
                    _activeEditors.Add(@object, editor);
                    return editor;
                }
            }

            Editor newEditor = new Editor();
            newEditor._target = @object;
            _activeEditors.Add(@object, newEditor);
            return newEditor;
        }

        #endregion

        #region Fields

        protected object _target = null;

        #endregion

        #region Constructor

        #endregion

        #region Methods

        protected virtual bool OnGui()
        {
            foreach(var field in _target.GetType().GetFields())
            {
                if (!field.IsPublic) continue;
                Type fieldType = field.FieldType;

                if (fieldType == typeof(float))
                {
                    float value = (float)field.GetValue(_target);
                    if (EditorUI.InputFloat(field.Name, ref value))
                    {
                        field.SetValue(_target, value);
                    }
                }

            }

            return false;
        }

        #endregion
    }
}
