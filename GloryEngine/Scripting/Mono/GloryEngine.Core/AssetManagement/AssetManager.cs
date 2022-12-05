﻿using System;
using System.Collections.Generic;

namespace GloryEngine
{
    public static class AssetManager
    {
        #region Fields

        private static Dictionary<UInt64, Resource> _resources = new Dictionary<ulong, Resource>();

        #endregion

        #region Methods

        public static Resource GetResource(UInt64 id)
        {
            if (!_resources.ContainsKey(id)) return null;
            return _resources[id];
        }

        //public static IntPtr GetGenericMethod(MethodInfo method, Type type)
        //{
        //    return method.MakeGenericMethod(type).MethodHandle.Value;
        //}

        public static Resource MakeResource(UInt64 id, string typeString)
        {
            if (_resources.ContainsKey(id)) return _resources[id];
            Type type = Type.GetType(typeString);
            if (type == null)
            {
                Debug.LogError(string.Format("Mono resource could not be created, the type {0} does not exist", typeString));
                return null;
            }
            Resource resource = Activator.CreateInstance(type, id) as Resource;
            _resources.Add(id, resource);
            Debug.LogInfo(string.Format("New {0} mono resource with ID {1} created", typeString, id));
            return resource;
        }

        public static T MakeResource<T>(UInt64 id) where T : Resource, new()
        {
            if (_resources.ContainsKey(id)) return _resources[id] as T;
            System.Type type = typeof(T);
            T resource = Activator.CreateInstance(type, id) as T;
            _resources.Add(id, resource);
            return resource;
        }

        #endregion
    }
}
