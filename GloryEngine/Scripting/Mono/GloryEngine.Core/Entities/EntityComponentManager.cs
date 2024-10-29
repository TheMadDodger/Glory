using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    static class EntityComponentManager
    {
        #region Fields

        private static Dictionary<UInt64, EntityComponent> _componentCache = new Dictionary<ulong, EntityComponent>();

        #endregion

        #region Methods

        /// <summary>
        /// Gets a native component by type on an Entity
        /// </summary>
        /// <typeparam name="T">Type of the native component to get</typeparam>
        /// <returns>The component that matches the type, null if the Entity does not have the component</returns>
        internal static T GetComponent<T>(ref Entity entity) where T : EntityComponent, new()
        {
            UInt64 componentID = Component_GetComponentID(ref entity, typeof(T).Name);
            if (componentID == 0)
            {
                Erase(componentID);
                return null;
            }
            if (_componentCache.ContainsKey(componentID)) return _componentCache[componentID] is T cachedComp ? cachedComp : null;
            T component = new T();
            component.Initialize(entity, componentID);
            _componentCache.Add(componentID, component);
            return component;
        }

        internal static bool HasComponent<T>(ref Entity entity) where T : EntityComponent, new()
        {
            UInt64 componentID = Component_GetComponentID(ref entity, typeof(T).Name);
            return componentID != 0;
        }

        internal static T AddComponent<T>(ref Entity entity) where T : EntityComponent, new()
        {
            UInt64 componentID = Component_AddComponent(ref entity, typeof(T).Name);
            if (componentID == 0) return null;
            T component = new T();
            component.Initialize(entity, componentID);
            _componentCache.Add(componentID, component);
            return component;
        }

        internal static void RemoveComponent<T>(ref Entity entity) where T : EntityComponent, new()
        {
            UInt64 componentID = Component_RemoveComponent(ref entity, typeof(T).Name);
            if (componentID == 0) return;
            _componentCache.Remove(componentID);
        }

        internal static void RemoveComponent(ref Entity entity, EntityComponent component)
            => RemoveComponent(ref entity, component.ID);

        internal static void RemoveComponent(ref Entity entity, UInt64 componentID)
        {
            Component_RemoveComponentByID(ref entity, componentID);
            _componentCache.Remove(componentID);
        }

        /// <summary>
        /// Used by the C++ to erase the component from the cache
        /// Never call this from CS
        /// </summary>
        /// <param name="componentID"></param>
        internal static void Erase(UInt64 componentID)
        {
            _componentCache.Remove(componentID);
        }

        internal static void Clear()
        {
            _componentCache.Clear();
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 Component_GetComponentID(ref Entity entity, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 Component_AddComponent(ref Entity entity, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 Component_RemoveComponent(ref Entity entity, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Component_RemoveComponentByID(ref Entity entity, UInt64 componentID);

        #endregion
    }
}
