using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    /// <summary>
    /// Handle for a LightComponent component
    /// </summary>
    public class LightComponent : EntityComponent
    {
		#region Props

		/// <summary>
		/// Color of the light
		/// </summary>
		public Vector4 Color
        {
			get => LightComponent_GetColor(ref _entity, _objectID);
			set => LightComponent_SetColor(ref _entity, _objectID, ref value);

		}

		/// <summary>
		/// Intensity of the light
		/// </summary>
		public float Intensity
        {
			get => LightComponent_GetIntensity(ref _entity, _objectID);
			set => LightComponent_SetIntensity(ref _entity, _objectID, value);
		}

		/// <summary>
		/// Range of the light
		/// </summary>
		public float Range
        {
			get => LightComponent_GetRange(ref _entity, _objectID);
			set => LightComponent_SetRange(ref _entity, _objectID, value);
		}

		#endregion

		#region API Methods

		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static Vector4 LightComponent_GetColor(ref Entity entity, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void LightComponent_SetColor(ref Entity entity, UInt64 componentID, ref Vector4 color);
		[MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float LightComponent_GetIntensity(ref Entity entity, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void LightComponent_SetIntensity(ref Entity entity, UInt64 componentID, float intensity);
		[MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float LightComponent_GetRange(ref Entity entity, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void LightComponent_SetRange(ref Entity entity, UInt64 componentID, float range);

		#endregion
	}
}
