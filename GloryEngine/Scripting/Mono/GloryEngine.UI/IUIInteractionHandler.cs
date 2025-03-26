namespace GloryEngine.UI
{
    /// <summary>
    /// Interface for recieving interactions between the cursor and the UI element
    /// </summary>
    public interface IUIInteractionHandler
    {
        /// <summary>
        /// The cursor has entered the element bounds
        /// </summary>
        /// <param name="interaction">Component being hovered</param>
        void OnHover(UIInteraction interaction);
        /// <summary>
        /// The cursor has left the element bounds
        /// </summary>
        /// <param name="interaction">Component being hovered</param>
        void OnUnHover(UIInteraction interaction);
        /// <summary>
        /// The cursor has selected the element
        /// </summary>
        /// <param name="interaction">Component being hovered</param>
        void OnDown(UIInteraction interaction);
        /// <summary>
        /// The cursor has released the element
        /// </summary>
        /// <param name="interaction">Component being hovered</param>
        void OnUp(UIInteraction interaction);
    }
}
