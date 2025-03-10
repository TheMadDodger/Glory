using GloryEngine.Entities;
using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.UI
{
    /// <summary>
    /// UI target
    /// </summary>
    public enum UITarget
    {
        CameraOverlay,
    }

    /// <summary>
    /// Resolution mode
    /// </summary>
    public enum ResolutionMode
    {
        CameraScale,
        Fixed,
    }

    /// <summary>
    /// UI Renderer component
    /// </summary>
    public class UIRenderer : NativeComponent
    {
        #region Props

        /// <summary>
        /// The current instance of the UI document for rendering
        /// </summary>
        public UIScene RenderDocument
        {
            get
            {
                UInt64 docID = UIRenderer_GetRenderDocumentID(Object.Scene.ID, Object.ID, _objectID);
                if (docID == 0) return null;
                if (_document == null)
                    _document = new UIScene(this);
                _document.ID = docID;
                return _document;
            }
            private set { }
        }

        /// <summary>
        /// Document to render
        /// </summary>
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

        /// <summary>
        /// Target to render the result to
        /// </summary>
        public UITarget Target
        {
            get => UIRenderer_GetTarget(Object.Scene.ID, Object.ID, _objectID);
            set => UIRenderer_SetTarget(Object.Scene.ID, Object.ID, _objectID, value);
        }

        /// <summary>
        /// How to handle resolution
        /// </summary>
        public ResolutionMode ResolutionMode
        {
            get => UIRenderer_GetResolutionMode(Object.Scene.ID, Object.ID, _objectID);
            set => UIRenderer_SetResolutionMode(Object.Scene.ID, Object.ID, _objectID, value);
        }

        /// <summary>
        /// Resolution of the UI texture
        /// </summary>
        public Vector2 Resolution
        {
            get => UIRenderer_GetResolution(Object.Scene.ID, Object.ID, _objectID).xy;
            set => UIRenderer_SetResolution(Object.Scene.ID, Object.ID, _objectID, new Vector3(value.x, value.y, 0.0f));
        }

        #endregion

        #region Fields

        private UIScene _document;

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 UIRenderer_GetRenderDocumentID(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

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
        private static extern Vector3 UIRenderer_GetResolution(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UIRenderer_SetResolution(UInt64 sceneID, UInt64 objectID, UInt64 componentID, Vector3 resolution);

        #endregion
    }
}
