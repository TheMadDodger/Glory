using System.Runtime.CompilerServices;

namespace GloryEngine
{
    public class Profiler
    {
        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void BeginSample(string name);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void EndSample();

        #endregion
    }
}
