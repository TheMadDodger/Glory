using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    public class LightComponent : EntityComponent
    {
		#region Props

		public Vector4 Color
        {
			get => LightComponent_GetColor(ref _entity, _componentID);
			set => LightComponent_SetColor(ref _entity, _componentID, ref value);

		}

		public float Intensity
        {
			get => LightComponent_GetIntensity(ref _entity, _componentID);
			set => LightComponent_SetIntensity(ref _entity, _componentID, value);
		}

		public float Range
        {
			get => LightComponent_GetRange(ref _entity, _componentID);
			set => LightComponent_SetRange(ref _entity, _componentID, value);
		}

		#endregion

		#region API Methods

		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern static Vector4 LightComponent_GetColor(ref Entity entity, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern static void LightComponent_SetColor(ref Entity entity, UInt64 componentID, ref Vector4 color);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern static float LightComponent_GetIntensity(ref Entity entity, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern static void LightComponent_SetIntensity(ref Entity entity, UInt64 componentID, float intensity);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern static float LightComponent_GetRange(ref Entity entity, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern static void LightComponent_SetRange(ref Entity entity, UInt64 componentID, float range);

		#endregion
	}
}
