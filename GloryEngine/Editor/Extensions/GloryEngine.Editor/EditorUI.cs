
using System.Runtime.CompilerServices;

namespace GloryEngine.Editor
{
    public class EditorUI
    {
        #region Methods

        public static bool InputFloat(string label, ref float value)
        {
            return EditorUI_InputFloat(label, ref value);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static bool EditorUI_InputFloat(string label, ref float value);

        #endregion
    }
}
