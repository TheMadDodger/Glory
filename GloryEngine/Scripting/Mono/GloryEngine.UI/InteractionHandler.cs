using GloryEngine.SceneManagement;
using System;

namespace GloryEngine.UI
{
    /// <summary>
    /// Internal handler for UI interactions
    /// </summary>
    static internal class InteractionHandler
    {
        #region Methods

        static internal void OnElementHover(Engine engine, UInt64 sceneID, UInt64 objectID, UInt64 elementID, UInt64 componentID)
        {
            Scene scene = engine.SceneManager.GetOpenScene(sceneID);
            if (scene == null) return;
            SceneObject sceneObject = scene.GetSceneObject(objectID);
            if (sceneObject == null) return;
            UIRenderer renderer = sceneObject.GetComponent<UIRenderer>();
            if (renderer == null) return;
            UIElement element = renderer.RenderDocument.GetUIElement(elementID);
            if (element == null) return;
            UIInteraction interaction = element.GetComponent<UIInteraction>(componentID);
            if (interaction == null) return;
            interaction.OnHover();
        }

        static internal void OnElementUnHover(Engine engine, UInt64 sceneID, UInt64 objectID, UInt64 elementID, UInt64 componentID)
        {
            Scene scene = engine.SceneManager.GetOpenScene(sceneID);
            if (scene == null) return;
            SceneObject sceneObject = scene.GetSceneObject(objectID);
            if (sceneObject == null) return;
            UIRenderer renderer = sceneObject.GetComponent<UIRenderer>();
            if (renderer == null) return;
            UIElement element = renderer.RenderDocument.GetUIElement(elementID);
            if (element == null) return;
            UIInteraction interaction = element.GetComponent<UIInteraction>(componentID);
            if (interaction == null) return;
            interaction.OnUnHover();
        }

        static internal void OnElementDown(Engine engine, UInt64 sceneID, UInt64 objectID, UInt64 elementID, UInt64 componentID)
        {
            Scene scene = engine.SceneManager.GetOpenScene(sceneID);
            if (scene == null) return;
            SceneObject sceneObject = scene.GetSceneObject(objectID);
            if (sceneObject == null) return;
            UIRenderer renderer = sceneObject.GetComponent<UIRenderer>();
            if (renderer == null) return;
            UIElement element = renderer.RenderDocument.GetUIElement(elementID);
            if (element == null) return;
            UIInteraction interaction = element.GetComponent<UIInteraction>(componentID);
            if (interaction == null) return;
            interaction.OnDown();
        }

        static internal void OnElementUp(Engine engine, UInt64 sceneID, UInt64 objectID, UInt64 elementID, UInt64 componentID)
        {
            Scene scene = engine.SceneManager.GetOpenScene(sceneID);
            if (scene == null) return;
            SceneObject sceneObject = scene.GetSceneObject(objectID);
            if (sceneObject == null) return;
            UIRenderer renderer = sceneObject.GetComponent<UIRenderer>();
            if (renderer == null) return;
            UIElement element = renderer.RenderDocument.GetUIElement(elementID);
            if (element == null) return;
            UIInteraction interaction = element.GetComponent<UIInteraction>(componentID);
            if (interaction == null) return;
            interaction.OnUp();
        }

        #endregion
    }
}
