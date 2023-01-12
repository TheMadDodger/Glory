using System.Runtime.CompilerServices;

namespace GloryEngine
{
    public class Profiler
    {
        #region API Methods

        /// <summary>
        /// Begin a profiling sample with a name
        /// This name will show up as the sample record name in the profiler
        /// Internally starts a timer
        /// </summary>
        /// <param name="name"></param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void BeginSample(string name);
        /// <summary>
        /// End the cirrent profile sample
        /// This stops the timer linked to that sample and writes to start and end time to the sample record
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void EndSample();

        #endregion
    }
}
