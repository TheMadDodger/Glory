using System.Runtime.CompilerServices;
using System;
using GloryEngine.Entities;

namespace GloryEngine.UI
{
    /// <summary>
    /// UI Text component
    /// </summary>
    public class UIText : UIComponent
    {
        #region Props

        /// <summary>
        /// Font being used to render the text
        /// </summary>
        public Font Font
        {
            get
            {
                UInt64 fontID = UIText_GetFont(Element.UIScene.ID, Element.ID, _objectID);
                if (fontID == 0) return null;
                return Element.UIScene.Renderer.Object.Scene.SceneManager.Engine.AssetManager.Get<Font>(fontID);
            }
            set => UIText_SetFont(Element.UIScene.ID, Element.ID, _objectID, value != null ? value.ID : 0);
        }

        /// <summary>
        /// Text being rendered
        /// </summary>
        public string Text
        {
            get => UIText_GetText(Element.UIScene.ID, Element.ID, _objectID);
            set => UIText_SetText(Element.UIScene.ID, Element.ID, _objectID, value);
        }

        /// <summary>
        /// Font scale
        /// </summary>
        public float Scale
        {
            get => UIText_GetScale(Element.UIScene.ID, Element.ID, _objectID);
            set => UIText_SetScale(Element.UIScene.ID, Element.ID, _objectID, value);
        }

        /// <summary>
        /// Color of the text
        /// </summary>
        public Vector4 Color
        {
            get => UIText_GetColor(Element.UIScene.ID, Element.ID, _objectID);
            set => UIText_SetColor(Element.UIScene.ID, Element.ID, _objectID, value);
        }

        /// <summary>
        /// Text alignment
        /// </summary>
        public Alignment Alignment
        {
            get => UIText_GetAlignment(Element.UIScene.ID, Element.ID, _objectID);
            set => UIText_SetAlignment(Element.UIScene.ID, Element.ID, _objectID, value);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 UIText_GetFont(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UIText_SetFont(UInt64 sceneID, UInt64 objectID, UInt64 componentID, UInt64 fontID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string UIText_GetText(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UIText_SetText(UInt64 sceneID, UInt64 objectID, UInt64 componentID, string text);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float UIText_GetScale(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UIText_SetScale(UInt64 sceneID, UInt64 objectID, UInt64 componentID, float scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector4 UIText_GetColor(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UIText_SetColor(UInt64 sceneID, UInt64 objectID, UInt64 componentID, Vector4 color);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Alignment UIText_GetAlignment(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UIText_SetAlignment(UInt64 sceneID, UInt64 objectID, UInt64 componentID, Alignment alignment);

        #endregion
    }
}
