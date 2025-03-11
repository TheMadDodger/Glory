using System.Runtime.CompilerServices;
using System;
using System.Runtime.InteropServices;

namespace GloryEngine.UI
{
    /// <summary>
    /// UI Interaction component
    /// </summary>
    public class UIInteraction : UIComponent
    {
        #region Props

        /// <summary>
        /// Whether interactions are enabled for this component
        /// </summary>
        public bool Enabled
        {
            get => UIInteraction_GetEnabled(Element.UIScene.ID, Element.ID, _objectID);
            set => UIInteraction_SetEnabled(Element.UIScene.ID, Element.ID, _objectID, value);
        }

        /// <summary>
        /// Whether the element is currently hovered by the cursor
        /// </summary>
        public bool IsHovered => _isHovered;

        /// <summary>
        /// Whether the element is currently down by the cursor
        /// </summary>
        private bool IsDown => _isDown;

        /// <summary>
        /// Current interaction handler
        /// </summary>
        public IUIInteractionHandler InteractionHandler => _interactionHandler;

        #endregion

        #region Fields

        internal IUIInteractionHandler _interactionHandler = null;
        internal bool _isHovered = false;
        internal bool _isDown = false;

        #endregion

        #region Methods

        /// <summary>
        /// Set the interaction handler for this element
        /// </summary>
        /// <param name="handler">Handler to set</param>
        public void SetInteractionHandler(IUIInteractionHandler handler)
        {
            _interactionHandler = handler;
        }

        internal void OnHover()
        {
            _isHovered = true;
            _interactionHandler?.OnHover(this);
        }

        internal void OnUnHover()
        {
            _isHovered = false;
            _isDown = false;
            _interactionHandler?.OnUnHover(this);
        }

        internal void OnDown()
        {
            _isDown = true;
            _interactionHandler?.OnDown(this);
        }

        internal void OnUp()
        {
            _isDown = false;
            _interactionHandler?.OnUp(this);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool UIInteraction_GetEnabled(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void UIInteraction_SetEnabled(UInt64 sceneID, UInt64 objectID, UInt64 componentID, bool enabled);

        #endregion
    }
}
