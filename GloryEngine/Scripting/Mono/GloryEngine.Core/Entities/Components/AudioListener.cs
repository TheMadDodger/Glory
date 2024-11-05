using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities.Components
{
    /// <summary>
    /// Audio simulation settings
    /// </summary>
    public struct AudioSimulationSettings
    {
        public bool m_Enable;
        public bool m_Direct;
        public bool m_Reflection;
        public bool m_Pathing;
    };

    /// <summary>
    /// Audio listener component
    /// </summary>
    public class AudioListener : NativeComponent
    {
        #region Props

        /// <summary>
        /// Whether this listener is active
        /// </summary>
        public bool Enabled
        {
            get => AudioListener_GetEnabled(Object.Scene.ID, Object.ID, _objectID);
            set => AudioListener_SetEnabled(Object.Scene.ID, Object.ID, _objectID, value);
        }

        /// <summary>
        /// Simulation settings for this listener
        /// </summary>
        public ref AudioSimulationSettings SimulationSettings
            => ref AudioListener_GetSimulationSettings(Object.Scene.ID, Object.ID, _objectID);

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool AudioListener_GetEnabled(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AudioListener_SetEnabled(UInt64 sceneID, UInt64 objectID, UInt64 componentID, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern ref AudioSimulationSettings AudioListener_GetSimulationSettings(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        #endregion
    }
}
