using System;
using System.Collections.Generic;

namespace GloryEngine
{
    /// <summary>
    /// Asset manager
    /// </summary>
    public class AssetManager
    {
        #region Props

        /// <summary>
        /// Engine instance
        /// </summary>
        public Engine Engine => _engine;

        #endregion

        #region Fields

        private Engine _engine;
        private Dictionary<UInt64, Resource> _resources = new Dictionary<ulong, Resource>();

        #endregion

        #region Constructor

        internal AssetManager(Engine engine)
        {
            _engine = engine;
        }

        #endregion

        #region Methods

        /// <summary>
        /// Get a loaded resource by ID
        /// </summary>
        /// <param name="id">ID of the resource</param>
        /// <returns></returns>
        public Resource GetResource(UInt64 id)
        {
            if (!_resources.ContainsKey(id)) return null;
            return _resources[id];
        }

        /// <summary>
        /// Get a resource by type
        /// </summary>
        /// <typeparam name="T">Type of the resource to get</typeparam>
        /// <param name="id">ID of the resource</param>
        /// <returns></returns>
        public T Get<T>(UInt64 id) where T : Resource, new()
        {
            Resource resource = GetResource(id);
            if(resource == null) resource = MakeResource<T>(id);
            return resource as T;
        }

        internal Resource MakeResource(UInt64 id, string typeString)
        {
            if (_resources.ContainsKey(id)) return _resources[id];
            Type type = Type.GetType(typeString);
            if (type == null)
            {
                Debug.LogError(string.Format("Mono resource could not be created, the type {0} does not exist", typeString));
                return null;
            }
            Resource resource = Activator.CreateInstance(type, this, id) as Resource;
            _resources.Add(id, resource);
            return resource;
        }

        internal T MakeResource<T>(UInt64 id) where T : Resource, new()
        {
            if (_resources.ContainsKey(id)) return _resources[id] as T;
            System.Type type = typeof(T);
            T resource = Activator.CreateInstance(type, this, id) as T;
            _resources.Add(id, resource);
            return resource;
        }

        internal void Clear()
        {
            _resources = new Dictionary<ulong, Resource>();
            Debug.LogNotice("Asset cache cleared");
        }

        #endregion
    }
}
