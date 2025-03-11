using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace GloryEngine.UI
{
    public class UIElement : Object
    {
        #region Props

        /// <summary>
        /// Whether this object is active
        /// </summary>
        public bool Active
        {
            get => UIElement_GetActive(_scene.ID, _objectID);
            set => UIElement_SetActive(_scene.ID, _objectID, value);
        }

        /// <summary>
        /// The scene this object exists in
        /// </summary>
        public virtual UIScene UIScene
        {
            get
            {
                if (_destroyed)
                {
                    Debug.LogError("This UIElement has been marked for destruction.");
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
                    Debug.LogError("This UIElement has been marked for destruction.");
                    return 0;
                }
                return UIElement_GetSiblingIndex(_scene.ID, _objectID);
            }
            set
            {
                if (_destroyed)
                {
                    Debug.LogError("This UIElement has been marked for destruction.");
                    return;
                }
                UIElement_SetSiblingIndex(_scene.ID, _objectID, value);
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
                    Debug.LogError("This UIElement has been marked for destruction.");
                    return 0;
                }
                return UIElement_GetChildCount(_scene.ID, _objectID);
            }
        }

        /// <summary>
        /// The object this object is parented to in the hierarchy
        /// Null if no parent
        /// Set to null to unparent
        /// </summary>
        public UIElement Parent
        {
            get
            {
                if (_destroyed)
                {
                    Debug.LogError("This UIElement has been marked for destruction.");
                    return null;
                }
                UInt64 objectID = UIElement_GetParent(_scene.ID, _objectID);
                return objectID != 0 ? UIScene.GetUIElement(objectID) : null;
            }
            set
            {
                if (_destroyed)
                {
                    Debug.LogError("This UIElement has been marked for destruction.");
                    return;
                }
                UIElement_SetParent(_scene.ID, _objectID, value != null ? value.ID : 0);
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
                    Debug.LogError("This UIElement has been marked for destruction.");
                    return null;
                }
                return UIElement_GetName(_scene.ID, _objectID);
            }
            set
            {
                if (_destroyed)
                {
                    Debug.LogError("This UIElement has been marked for destruction.");
                    return;
                }
                UIElement_SetName(_scene.ID, _objectID, value);
            }
        }

        #endregion

        #region Fields

        private UIScene _scene;

        internal bool _destroyed = false;

        private Dictionary<UInt64, UIComponent> _componentCache = new Dictionary<ulong, UIComponent>();

        #endregion

        #region Constructor

        internal UIElement() { }
        internal UIElement(UInt64 objectID, UIScene scene) : base(objectID) { _scene = scene; }

        #endregion

        #region Methods

        /// <summary>
        /// Gets a component by type on the Entity that owns this component.
        /// Must be a UIComponent.
        /// </summary>
        /// <typeparam name="T">Type of the component to get</typeparam>
        /// <returns>The component that matches the type, null if the Entity does not have the component</returns>
        public T GetComponent<T>() where T : class
        {
            Type type = typeof(T);
            if (!type.IsSubclassOf(typeof(UIComponent)))
                throw new Exception("Cannot get a non-native component on a UIElement");

            UInt64 componentID = UIElement_GetComponentID(_scene.ID, _objectID, typeof(T).Name);
            if (componentID == 0)
                return null;

            if (_componentCache.ContainsKey(componentID)) return _componentCache[componentID] is T cachedComp ? cachedComp : null;
            UIComponent component = Activator.CreateInstance(typeof(T)) as UIComponent;
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
            if (type.IsSubclassOf(typeof(UIComponent)))
                componentID = UIElement_AddComponent(_scene.ID, _objectID, type.Name);
            else
                throw new Exception("Cannot add a non UIComponent on a UIElement");

            if (componentID == 0)
                throw new Exception("Failed to create component");
            // Script components get added automatically, so check if the component was already added
            if (_componentCache.ContainsKey(componentID))
                return _componentCache[componentID] as T;

            UIComponent component = Activator.CreateInstance(type) as UIComponent;
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
            if (!type.IsSubclassOf(typeof(UIComponent)))
                return;
            UInt64 componentID = UIElement_RemoveComponent(_scene.ID, _objectID, typeof(T).Name);
            if (componentID == 0) return;
            _componentCache.Remove(componentID);
        }

        /// <summary>
        /// Remove a component from this entity
        /// </summary>
        /// <param name="component">Component to remove</param>
        public void RemoveComponent(UIComponent component)
        {
            UIElement_RemoveComponentByID(_scene.ID, _objectID, component.ID);
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
            if (!type.IsSubclassOf(typeof(UIComponent)))
                return false;
            return UIElement_GetComponentID(_scene.ID, _objectID, typeof(T).Name) != 0;
        }

        /// <summary>
        /// Get a child in the hierarchy of this object
        /// </summary>
        /// <param name="index">Index of the child to get</param>
        /// <returns>The child UIElement or null if an invalid index was provided</returns>
        public UIElement GetChild(int index)
        {
            UInt64 childID = UIElement_GetChild(_scene.ID, _objectID, (uint)index);
            if (childID == 0) return null;
            return UIScene.GetUIElement(childID);
        }

        /// <summary>
        /// Find an element in the children of this object with a name, the search only goes one tree deep
        /// </summary>
        /// <param name="name">Name of the element to find</param>
        /// <returns>The first element which name matches the one supplied or null if none found</returns>
        public UIElement FindUIElement(string name)
        {
            UInt64 objectID = UIElement_FindElement(_scene.ID, _objectID, name);
            if (objectID == 0) return null;
            return UIScene.GetUIElement(objectID);
        }

        /// <summary>
        /// Gets a component by ID directly, for internal use only.
        /// Must be a UIComponent.
        /// </summary>
        /// <typeparam name="T">Type to cast the component to</typeparam>
        /// <returns>The component with the same ID and type, null if type casting failed</returns>
        internal T GetComponent<T>(UInt64 componentID) where T : class
        {
            Type type = typeof(T);
            if (!type.IsSubclassOf(typeof(UIComponent)))
                throw new Exception("Cannot get a non-native component on a UIElement");

            if (_componentCache.ContainsKey(componentID)) return _componentCache[componentID] is T cachedComp ? cachedComp : null;
            UIComponent component = Activator.CreateInstance(typeof(T)) as UIComponent;
            component.Initialize(this, componentID);
            _componentCache.Add(componentID, component);
            return component as T;
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
        private extern static bool UIElement_GetActive(UInt64 sceneID, UInt64 objectID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void UIElement_SetActive(UInt64 sceneID, UInt64 objectID, bool active);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static string UIElement_GetName(UInt64 sceneID, UInt64 objectID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void UIElement_SetName(UInt64 sceneID, UInt64 objectID, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static uint UIElement_GetSiblingIndex(UInt64 sceneID, UInt64 objectID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void UIElement_SetSiblingIndex(UInt64 sceneID, UInt64 objectID, uint index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static uint UIElement_GetChildCount(UInt64 sceneID, UInt64 objectID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static UInt64 UIElement_GetChild(UInt64 sceneID, UInt64 objectID, uint index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static UInt64 UIElement_GetParent(UInt64 sceneID, UInt64 objectID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void UIElement_SetParent(UInt64 sceneID, UInt64 objectID, UInt64 parentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 UIElement_GetComponentID(UInt64 sceneID, UInt64 objectID, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 UIElement_AddComponent(UInt64 sceneID, UInt64 objectID, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 UIElement_RemoveComponent(UInt64 sceneID, UInt64 objectID, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void UIElement_RemoveComponentByID(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 UIElement_FindElement(UInt64 sceneID, UInt64 objectID, string name);

        #endregion
    }
}
