using System;
using GloryEngine.SceneManagement;

namespace GloryEngine.Entities
{
    public class EntityScene : Scene
    {
        #region Methods

        protected override SceneObject CreateSceneObject(UInt64 objectID)
            => new EntitySceneObject(objectID, _objectID);

        #endregion
    }
}
