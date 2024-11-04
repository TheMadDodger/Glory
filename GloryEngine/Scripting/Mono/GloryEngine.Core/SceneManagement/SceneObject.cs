using GloryEngine.Entities;
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace GloryEngine.SceneManagement
{
    /// <summary>
    /// Handle for an entity object
    /// </summary>
    public class SceneObject : Object
    {
        #region Props

        /// <summary>
        /// The scene this object exists in
        /// </summary>
        public virtual Scene Scene
        {
            get
            {
                if (_destroyed)
                {
                    Debug.LogError("This SceneObject has been marked for destruction.");
                    return null;
                }
                return _scene;
            }
            private set { }
        }

        /// <summary>
        /// The index of the hierarchy order relative to the parent object
        /// Or within the scene if no parent
        /// </summary>
        public uint SiblingIndex
        {
            get
            {
                if (_destroyed)
                {
                    Debug.LogError("This SceneObject has been marked for destruction.");
                    return 0;
                }
                return SceneObject_GetSiblingIndex(_scene.ID, _objectID);
            }
            set
            {
                if (_destroyed)
                {
                    Debug.LogError("This SceneObject has been marked for destruction.");
                    return;
                }
                SceneObject_SetSiblingIndex(_scene.ID, _objectID, value);
            }
        }

        /// <summary>
        /// Number of children parented to this object in the hierarchy
        /// </summary>
        public uint ChildCount
        {
            get
            {
                if (_destroyed)
                {
                    Debug.LogError("This SceneObject has been marked for destruction.");
                    return 0;
                }
                return SceneObject_GetChildCount(_scene.ID, _objectID);
            }
        }

        /// <summary>
        /// The object this object is parented to in the hierarchy
        /// Null if no parent
        /// Set to null to unparent
        /// </summary>
        public SceneObject Parent
        {
            get
            {
                if (_destroyed)
                {
                    Debug.LogError("This SceneObject has been marked for destruction.");
                    return null;
                }
                UInt64 objectID = SceneObject_GetParent(_scene.ID, _objectID);
                return objectID != 0 ? Scene.GetSceneObject(objectID) : null;
            }
            set
            {
                if (_destroyed)
                {
                    Debug.LogError("This SceneObject has been marked for destruction.");
                    return;
                }
                SceneObject_SetParent(_scene.ID, _objectID, value != null ? value.ID : 0);
            }
        }

        /// <summary>
        /// Name of the object
        /// </summary>
        public override string Name
        {
            get
            {
                if (_destroyed)
                {
                    Debug.LogError("This SceneObject has been marked for destruction.");
                    return null;
                }
                return SceneObject_GetName(_scene.ID, _objectID);
            }
            set
            {
                if (_destroyed)
                {
                    Debug.LogError("This SceneObject has been marked for destruction.");
                    return;
                }
                SceneObject_SetName(_scene.ID, _objectID, value);
            }
        }

        #endregion

        #region Fields

        private Scene _scene;
        internal bool _destroyed = false;

        private Dictionary<UInt64, EntityComponent> _componentCache = new Dictionary<ulong, EntityComponent>();

        #endregion

        #region Constructor

        internal SceneObject() { }
        internal SceneObject(UInt64 objectID, Scene scene) : base(objectID) { _scene = scene; }

        #endregion

        #region Methods

        /// <summary>
        /// Gets a component by type on the Entity that owns this component.
        /// Can be either a native or script component.
        /// </summary>
        /// <typeparam name="T">Type of the component to get</typeparam>
        /// <returns>The component that matches the type, null if the Entity does not have the component</returns>
        public T GetComponent<T>() where T : class
        {
            Type type = typeof(T);
            if (!type.IsSubclassOf(typeof(NativeComponent)))
            {
                foreach (EntityComponent comp in _componentCache.Values)
                {
                    Type compType = comp.GetType();
                    if (compType != type && !type.IsAssignableFrom(compType)) continue;
                    return comp as T;
                }
                return null;
            }

            UInt64 componentID = SceneObject_GetComponentID(_scene.ID, _objectID, typeof(T).Name);
            if (componentID == 0)
                return null;

            if (_componentCache.ContainsKey(componentID)) return _componentCache[componentID] is T cachedComp ? cachedComp : null;
            EntityComponent component = Activator.CreateInstance(typeof(T)) as EntityComponent;
            component.Initialize(this, componentID);
            _componentCache.Add(componentID, component);
            return component as T;
        }

        /// <summary>
        /// Add a new component to this entity
        /// </summary>
        /// <typeparam name="T">Type of component to add</typeparam>
        /// <returns>The newly constructed component or null if failed</returns>
        public T AddComponent<T>() where T : class
        {
            UInt64 componentID = 0;
            Type type = typeof(T);
            if (type.IsSubclassOf(typeof(NativeComponent)))
            {
                componentID = SceneObject_AddComponent(_scene.ID, _objectID, type.Name);
            }
            else
            {
                int typeIndex = Scene.SceneManager.Engine.GetTypeIndex<T>();
                if (typeIndex == -1) return null;
                componentID = SceneObject_AddScriptComponent(_scene.ID, _objectID, typeIndex);
            }

            if (componentID == 0) return null;
            EntityComponent component = Activator.CreateInstance(type) as EntityComponent;
            component.Initialize(this, componentID);
            _componentCache.Add(componentID, component);
            return component as T;
        }

        /// <summary>
        /// Remove a component from this entity
        /// </summary>
        /// <typeparam name="T">The type of component to remove</typeparam>
        public void RemoveComponent<T>() where T : class
        {
            Type type = typeof(T);
            if (!type.IsSubclassOf(typeof(NativeComponent)))
            {
                foreach (EntityComponent comp in _componentCache.Values)
                {
                    Type compType = comp.GetType();
                    if (compType != type && !type.IsAssignableFrom(compType)) continue;
                    SceneObject_RemoveComponentByID(_scene.ID, _objectID, comp.ID);
                    _componentCache.Remove(comp.ID);
                    return;
                }
                return;
            }
            UInt64 componentID = SceneObject_RemoveComponent(_scene.ID, _objectID, typeof(T).Name);
            if (componentID == 0) return;
            _componentCache.Remove(componentID);
        }

        /// <summary>
        /// Remove a component from this entity
        /// </summary>
        /// <param name="component">Component to remove</param>
        public void RemoveComponent(EntityComponent component)
        {
            SceneObject_RemoveComponentByID(_scene.ID, _objectID, component.ID);
            _componentCache.Remove(component.ID);
        }

        /// <summary>
        /// Check if this entity has a certain component
        /// </summary>
        /// <typeparam name="T">The type of component to check for</typeparam>
        /// <returns>True if the entity has the component</returns>
        public bool HasComponent<T>() where T : class
        {
            Type type = typeof(T);
            if (!type.IsSubclassOf(typeof(NativeComponent)))
            {
                foreach (EntityComponent comp in _componentCache.Values)
                {
                    Type compType = comp.GetType();
                    if (compType != type && !type.IsAssignableFrom(compType)) continue;
                    return true;
                }
                return false;
            }
            return SceneObject_GetComponentID(_scene.ID, _objectID, typeof(T).Name) != 0;
        }

        internal EntityBehaviour CreateScriptComponent(Type type, UInt64 componentID)
        {
            if (_componentCache.ContainsKey(componentID))
                _componentCache.Remove(componentID);
            EntityBehaviour entityBehaviour = Activator.CreateInstance(type) as EntityBehaviour;
            entityBehaviour.Initialize(this, componentID);
            _componentCache.Add(componentID, entityBehaviour);
            return entityBehaviour;
        }

        internal EntityBehaviour GetScript(UInt64 componentID)
        {
            if (!_componentCache.ContainsKey(componentID)) return null;
            EntityComponent component = _componentCache[componentID];
            if (!(component is EntityBehaviour)) return null;
            return _componentCache[componentID] as EntityBehaviour;
        }

        internal void OnObjectDestroy()
        {
            _destroyed = true;
            foreach (var component in _componentCache.Values)
            {
                component._destroyed = true;
            }
            _componentCache.Clear();
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static string SceneObject_GetName(UInt64 sceneID, UInt64 objectID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void SceneObject_SetName(UInt64 sceneID, UInt64 objectID, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static uint SceneObject_GetSiblingIndex(UInt64 sceneID, UInt64 objectID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void SceneObject_SetSiblingIndex(UInt64 sceneID, UInt64 objectID, uint index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static uint SceneObject_GetChildCount(UInt64 sceneID, UInt64 objectID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static UInt64 SceneObject_GetChild(UInt64 sceneID, UInt64 objectID, uint index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static UInt64 SceneObject_GetParent(UInt64 sceneID, UInt64 objectID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void SceneObject_SetParent(UInt64 sceneID, UInt64 objectID, UInt64 parentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 SceneObject_GetComponentID(UInt64 sceneID, UInt64 objectID, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 SceneObject_AddComponent(UInt64 sceneID, UInt64 objectID, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 SceneObject_AddScriptComponent(UInt64 sceneID, UInt64 objectID, int typeIndex);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 SceneObject_RemoveComponent(UInt64 sceneID, UInt64 objectID, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SceneObject_RemoveComponentByID(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        #endregion
    }
}
