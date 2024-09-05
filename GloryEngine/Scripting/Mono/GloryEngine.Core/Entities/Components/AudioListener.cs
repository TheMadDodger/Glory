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
    public class AudioListener : EntityComponent
    {
        #region Props

        /// <summary>
        /// Whether this listener is active
        /// </summary>
        public bool Enabled
        {
            get => AudioListener_GetEnabled(ref _entity, _objectID);
            set => AudioListener_SetEnabled(ref _entity, _objectID, value);
        }

        /// <summary>
        /// Simulation settings for this listener
        /// </summary>
        public ref AudioSimulationSettings SimulationSettings
            => ref AudioListener_GetSimulationSettings(ref _entity, _objectID);

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool AudioListener_GetEnabled(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AudioListener_SetEnabled(ref Entity entity, UInt64 componentID, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern ref AudioSimulationSettings AudioListener_GetSimulationSettings(ref Entity entity, UInt64 componentID);

        #endregion
    }
}
