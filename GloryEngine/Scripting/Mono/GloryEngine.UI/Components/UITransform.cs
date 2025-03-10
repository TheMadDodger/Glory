using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.UI
{
    /// <summary>
    /// UI Transform component
    /// </summary>
    public class UITransform : UIComponent
    {
        #region Props

        /// <summary>
        /// Position of the element rect
        /// </summary>
        public Vector2 Position
        {
            get => UITransform_GetPosition(Element.UIScene.ID, Element.ID, _objectID).xy;
            set => UITransform_SetPosition(Element.UIScene.ID, Element.ID, _objectID, new Vector3(value));
        }

        /// <summary>
        /// Size of the element rect
        /// </summary>
        public Vector2 Size
        {
            get => UITransform_GetSize(Element.UIScene.ID, Element.ID, _objectID).xy;
            set => UITransform_SetSize(Element.UIScene.ID, Element.ID, _objectID, new Vector3(value));
        }

        /// <summary>
        /// Pivot of the element rect from 0-1
        /// </summary>
        public Vector2 Pivot
        {
            get => UITransform_GetPivot(Element.UIScene.ID, Element.ID, _objectID).xy;
            set => UITransform_SetPivot(Element.UIScene.ID, Element.ID, _objectID, new Vector3(value));
        }

        /// <summary>
        /// Additional rotation of the element
        /// </summary>
        public float Rotation
        {
            get => UITransform_GetRotation(Element.UIScene.ID, Element.ID, _objectID);
            set => UITransform_SetRotation(Element.UIScene.ID, Element.ID, _objectID, value);
        }

        /// <summary>
        /// Additional scale of the element
        /// </summary>
        public Vector2 Scale
        {
            get => UITransform_GetScale(Element.UIScene.ID, Element.ID, _objectID).xy;
            set => UITransform_SetScale(Element.UIScene.ID, Element.ID, _objectID, new Vector3(value));
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 UITransform_GetPosition(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UITransform_SetPosition(UInt64 sceneID, UInt64 objectID, UInt64 componentID, Vector3 pos);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 UITransform_GetSize(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UITransform_SetSize(UInt64 sceneID, UInt64 objectID, UInt64 componentID, Vector3 size);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 UITransform_GetPivot(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UITransform_SetPivot(UInt64 sceneID, UInt64 objectID, UInt64 componentID, Vector3 pivot);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float UITransform_GetRotation(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UITransform_SetRotation(UInt64 sceneID, UInt64 objectID, UInt64 componentID, float rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 UITransform_GetScale(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UITransform_SetScale(UInt64 sceneID, UInt64 objectID, UInt64 componentID, Vector3 scale);

        #endregion
    }
}
