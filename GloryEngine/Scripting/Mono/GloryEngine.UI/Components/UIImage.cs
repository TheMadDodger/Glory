using System.Runtime.CompilerServices;
using System;

namespace GloryEngine.UI
{
    /// <summary>
    /// UI Image component
    /// </summary>
    public class UIImage : UIComponent
    {
        #region Props

        /// <summary>
        /// The image being rendered
        /// </summary>
        public Texture Image
        {
            get
            {
                UInt64 imageID = UIImage_GetImage(Element.UIScene.ID, Element.ID, _objectID);
                if (imageID == 0) return null;
                return Element.UIScene.Renderer.Object.Scene.SceneManager.Engine.AssetManager.Get<Texture>(imageID);
            }
            set => UIImage_SetImage(Element.UIScene.ID, Element.ID, _objectID, value != null ? value.ID : 0);
        }

        /// <summary>
        /// Color tint of the image
        /// </summary>
        public Vector4 Color
        {
            get => UIImage_GetColor(Element.UIScene.ID, Element.ID, _objectID);
            set => UIImage_SetColor(Element.UIScene.ID, Element.ID, _objectID, value);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 UIImage_GetImage(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UIImage_SetImage(UInt64 sceneID, UInt64 objectID, UInt64 componentID, UInt64 imageID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector4 UIImage_GetColor(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UIImage_SetColor(UInt64 sceneID, UInt64 objectID, UInt64 componentID, Vector4 color);

        #endregion
    }
}
