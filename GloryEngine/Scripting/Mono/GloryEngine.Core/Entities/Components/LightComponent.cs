using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    /// <summary>
    /// Handle for a LightComponent component
    /// </summary>
    public class LightComponent : NativeComponent
    {
		#region Props

		/// <summary>
		/// Color of the light
		/// </summary>
		public Vector4 Color
        {
			get => LightComponent_GetColor(Object.Scene.ID, Object.ID, _objectID);
			set => LightComponent_SetColor(Object.Scene.ID, Object.ID, _objectID, ref value);
		}

		/// <summary>
		/// Intensity of the light
		/// </summary>
		public float Intensity
        {
			get => LightComponent_GetIntensity(Object.Scene.ID, Object.ID, _objectID);
			set => LightComponent_SetIntensity(Object.Scene.ID, Object.ID, _objectID, value);
		}

		/// <summary>
		/// Range of the light
		/// </summary>
		public float Range
        {
			get => LightComponent_GetRange(Object.Scene.ID, Object.ID, _objectID);
			set => LightComponent_SetRange(Object.Scene.ID, Object.ID, _objectID, value);
		}

		#endregion

		#region API Methods

		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static Vector4 LightComponent_GetColor(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void LightComponent_SetColor(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector4 color);
		[MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float LightComponent_GetIntensity(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void LightComponent_SetIntensity(UInt64 sceneID, UInt64 objectID, UInt64 componentID, float intensity);
		[MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float LightComponent_GetRange(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void LightComponent_SetRange(UInt64 sceneID, UInt64 objectID, UInt64 componentID, float range);

		#endregion
	}
}
