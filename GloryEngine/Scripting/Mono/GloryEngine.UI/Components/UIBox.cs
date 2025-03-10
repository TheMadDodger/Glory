using System.Runtime.CompilerServices;
using System;

namespace GloryEngine.UI
{
    /// <summary>
    /// UI Box component
    /// </summary>
    public class UIBox : UIComponent
    {
        #region Props

        /// <summary>
        /// Color tint of the box
        /// </summary>
        public Vector4 Color
        {
            get => UIBox_GetColor(Element.UIScene.ID, Element.ID, _objectID);
            set => UIBox_SetColor(Element.UIScene.ID, Element.ID, _objectID, value);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector4 UIBox_GetColor(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UIBox_SetColor(UInt64 sceneID, UInt64 objectID, UInt64 componentID, Vector4 color);

        #endregion
    }
}
