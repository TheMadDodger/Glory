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
        public static T GetComponent<T>(ref Entity entity) where T : EntityComponent, new()
        {
            UInt64 componentID = Component_GetComponentID(ref entity, typeof(T).Name);
            if (componentID == 0) return null;
            if (_componentCache.ContainsKey(componentID)) return _componentCache[componentID] is T cachedComp ? cachedComp : null;
            T component = new T();
            component.Initialize(entity, componentID);
            _componentCache.Add(componentID, component);
            return component;
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern UInt64 Component_GetComponentID(ref Entity entity, string name);

        #endregion
    }
}
