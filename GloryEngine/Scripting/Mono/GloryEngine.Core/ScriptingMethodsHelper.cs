using System;
using System.Collections.Generic;
using System.Reflection;

namespace GloryEngine
{
    internal static class ScriptingMethodsHelper
    {
        static Dictionary<Type, Dictionary<string, MethodInfo>> m_CachedMethods = new Dictionary<Type, Dictionary<string, MethodInfo>>();

        internal static void CallBasicMethod(object obj, string methodName)
        {
            if (obj == null) return;
			try
			{
                MethodInfo method = GetMethodInfo(obj.GetType(), methodName);
                if (method == null) return;
				method.Invoke(obj, null);
			}
			catch (Exception e)
			{
				Debug.LogError(e.ToString());
			}
        }

        internal static void Call1ArgsMethod(object obj, string methodName, UInt32 arg1)
        {
            if (obj == null) return;
            try
			{
                MethodInfo method = GetMethodInfo(obj.GetType(), methodName);
                if (method == null) return;
				object[] args = new object[1];
                args[0] = arg1;
				method.Invoke(obj, args);
			}
			catch (Exception e)
			{
				Debug.LogError(e.ToString());
			}
        }

        internal static void Call2ArgsMethod(object obj, string methodName, UInt32 arg1, UInt32 arg2)
        {
            if (obj == null) return;
            try
            {
                MethodInfo method = GetMethodInfo(obj.GetType(), methodName);
                if (method == null) return;
                object[] args =
                {
                    arg1,
                    arg2
                };
                method.Invoke(obj, args);
            }
            catch (Exception e)
            {
                Debug.LogError(e.ToString());
            }
        }

        internal static MethodInfo GetMethodInfo(Type type, string name)
        {
            if(!m_CachedMethods.ContainsKey(type))
                m_CachedMethods.Add(type, new Dictionary<string, MethodInfo>());

            if (!m_CachedMethods[type].ContainsKey(name))
            {
                MethodInfo method = type.GetMethod(name);
                if(method == null) return null;
                m_CachedMethods[type].Add(name, method);
            }

            return m_CachedMethods[type][name];
        }
    }
}
