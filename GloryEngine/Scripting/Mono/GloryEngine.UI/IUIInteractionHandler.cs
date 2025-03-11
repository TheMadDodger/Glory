namespace GloryEngine.UI
{
    public interface IUIInteractionHandler
    {
        void OnHover(UIInteraction interaction);
        void OnUnHover(UIInteraction interaction);
        void OnDown(UIInteraction interaction);
        void OnUp(UIInteraction interaction);
    }
}
