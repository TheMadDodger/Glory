using System;

namespace GloryEngine.Editor
{
    public class ObjectEditorAttribute : Attribute
    {
        #region Properties

        public Type EditedType => _editedType;

        #endregion

        #region Fields

        private Type _editedType;

        #endregion

        #region Constructor

        public ObjectEditorAttribute(Type type)
        {
            _editedType = type;
        }

        #endregion
    }
}
