using System;
using System.Reflection;

namespace GloryEngine
{
    internal static class ScriptingMethodsHelper
    {
        internal static void CallBasicMethod(object obj, string methodName)
        {
			try
			{
				MethodInfo method = obj.GetType().GetMethod(methodName);
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
			try
			{
                Debug.LogInfo(arg1.ToString());
				MethodInfo method = obj.GetType().GetMethod(methodName);
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
            try
            {
                MethodInfo method = obj.GetType().GetMethod(methodName);
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
    }
}
