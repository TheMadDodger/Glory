using GloryEngine.Entities;
using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.UI
{
    public enum UITarget
    {
        CameraOverlay,
    }

    public enum ResolutionMode
    {
        CameraScale,
        Fixed,
    }

    public class UIRenderer : NativeComponent
    {
        #region Props

        public UIDocument Document
        {
            get
            {
                UInt64 documentID = UIRenderer_GetDocumentID(Object.Scene.ID, Object.ID, _objectID);
                if (documentID == 0) return null;
                return Object.Scene.SceneManager.Engine.AssetManager.Get<UIDocument>(documentID);
            }
            set => UIRenderer_SetDocumentID(Object.Scene.ID, Object.ID, _objectID, value != null ? value.ID : 0);
        }

        public UITarget Target
        {
            get => UIRenderer_GetTarget(Object.Scene.ID, Object.ID, _objectID);
            set => UIRenderer_SetTarget(Object.Scene.ID, Object.ID, _objectID, value);
        }

        public ResolutionMode ResolutionMode
        {
            get => UIRenderer_GetResolutionMode(Object.Scene.ID, Object.ID, _objectID);
            set => UIRenderer_SetResolutionMode(Object.Scene.ID, Object.ID, _objectID, value);
        }

        public Vector2 Resolution
        {
            get => UIRenderer_GetResolution(Object.Scene.ID, Object.ID, _objectID);
            set => UIRenderer_SetResolution(Object.Scene.ID, Object.ID, _objectID, value);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 UIRenderer_GetDocumentID(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UIRenderer_SetDocumentID(UInt64 sceneID, UInt64 objectID, UInt64 componentID, UInt64 documentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UITarget UIRenderer_GetTarget(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UIRenderer_SetTarget(UInt64 sceneID, UInt64 objectID, UInt64 componentID, UITarget target);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern ResolutionMode UIRenderer_GetResolutionMode(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UIRenderer_SetResolutionMode(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ResolutionMode mode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector2 UIRenderer_GetResolution(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UIRenderer_SetResolution(UInt64 sceneID, UInt64 objectID, UInt64 componentID, Vector2 resolution);

        #endregion
    }
}
