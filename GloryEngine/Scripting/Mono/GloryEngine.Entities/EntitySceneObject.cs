using GloryEngine.SceneManagement;
using System;

namespace GloryEngine.Entities
{
    public class EntitySceneObject : SceneObject
    {
        #region Props

        public override Scene Scene
        {
            get
            {
                if (_scene == null)
                    _scene = SceneManager.GetOpenScene<EntityScene>(_sceneID);
                return _scene;
            }
        }

        #endregion

        #region Constructor

        public EntitySceneObject() { }
        public EntitySceneObject(UInt64 objectID, UInt64 sceneID) : base(objectID, sceneID) { }

        #endregion
    }
}
