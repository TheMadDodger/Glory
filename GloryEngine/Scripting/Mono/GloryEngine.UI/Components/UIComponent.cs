using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.UI
{
    /// <summary>
    /// Base class for UI components
    /// </summary>
    public class UIComponent : Object
    {
        #region Props

        /// <summary>
        /// Transform component linked to the entity that owns this component
        /// </summary>
        public UITransform Transform
        {
            get
            {
                if (_transform != null) return _transform;
                _transform = GetComponent<UITransform>();
                return _transform;
            }
            private set { }
        }

        /// <summary>
        /// Active state of the component
        /// </summary>
        public bool Active
        {
            get => UIComponent_GetActive(Element.UIScene.ID, Element.ID, _objectID);
            set => UIComponent_SetActive(Element.UIScene.ID, Element.ID, _objectID, value);
        }

        /// <summary>
        /// UI Element
        /// </summary>
        public UIElement Element => _object;

        #endregion

        #region Fields

        private UIElement _object;
        private UITransform _transform = null;
        internal bool _destroyed = false;

        #endregion

        #region Methods

        /// <summary>
        /// Gets a component by type on the Entity that owns this component
        /// </summary>
        /// <typeparam name="T">Type of the component to get</typeparam>
        /// <returns>The component that matches the type, null if the Entity does not have the component</returns>
        public T GetComponent<T>() where T : class
        {
            return Element.GetComponent<T>();
        }

        internal void OnComponentDestroy()
        {
            _destroyed = true;
        }

        internal void Initialize(UIElement sceneObject, UInt64 componentID)
        {
            _object = sceneObject;
            _objectID = componentID;
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool UIComponent_GetActive(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UIComponent_SetActive(UInt64 sceneID, UInt64 objectID, UInt64 componentID, bool active);

        #endregion
    }
}
