﻿using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
	public class CameraComponent : EntityComponent
	{
		#region Props

		/// <summary>
		/// Half angle of the Field of View in degrees
		/// </summary>
		public float HalfFOV
		{
			get => CameraComponent_GetHalfFOV(ref _entity, _objectID);
			set => CameraComponent_SetHalfFOV(ref _entity, _objectID, value);
		}

		/// <summary>
		/// How close objects can get to the camera before they are clipped
		/// </summary>
		public float Near
		{
			get => CameraComponent_GetNear(ref _entity, _objectID);
			set => CameraComponent_SetNear(ref _entity, _objectID, value);
		}

		/// <summary>
		/// How far the camera should render before objects are clipped
		/// </summary>
		public float Far
		{
			get => CameraComponent_GetFar(ref _entity, _objectID);
			set => CameraComponent_SetFar(ref _entity, _objectID, value);
		}

		/// <summary>
		/// The index of the display this camera should render to
		/// This is an internal display
		/// </summary>
		public int DisplayIndex
		{
			get => CameraComponent_GetDisplayIndex(ref _entity, _objectID);
			set => CameraComponent_SetDisplayIndex(ref _entity, _objectID, value);
		}

		/// <summary>
		/// Controls rendering order, ordered by high to low
		/// </summary>
		public int Priority
		{
			get => CameraComponent_GetPriority(ref _entity, _objectID);
			set => CameraComponent_SetPriority(ref _entity, _objectID, value);
		}

		/// <summary>
		/// Mask of layers this camera renders
		/// </summary>
		public LayerMask LayerMask
		{
			get => CameraComponent_GetLayerMask(ref _entity, _objectID);
			set => CameraComponent_SetLayerMask(ref _entity, _objectID, ref value);
		}

		/// <summary>
		/// The color the target is cleared with before rendering the camera to it
		/// </summary>
		public Vector4 ClearColor
		{
			get => CameraComponent_GetClearColor(ref _entity, _objectID);
			set => CameraComponent_SetClearColor(ref _entity, _objectID, ref value);
		}

		/// <summary>
		/// The Camera handle for this component
		/// </summary>
		public Camera Camera => new Camera(CameraComponent_GetCameraID(ref _entity, _objectID));

		#endregion

		#region API Methods

		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static float CameraComponent_GetHalfFOV(ref Entity entity, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static void CameraComponent_SetHalfFOV(ref Entity entity, UInt64 componentID, float halfFov);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static float CameraComponent_GetNear(ref Entity entity, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static void CameraComponent_SetNear(ref Entity entity, UInt64 componentID, float near);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static float CameraComponent_GetFar(ref Entity entity, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static void CameraComponent_SetFar(ref Entity entity, UInt64 componentID, float far);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static int CameraComponent_GetDisplayIndex(ref Entity entity, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static void CameraComponent_SetDisplayIndex(ref Entity entity, UInt64 componentID, int displayIndex);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static int CameraComponent_GetPriority(ref Entity entity, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static void CameraComponent_SetPriority(ref Entity entity, UInt64 componentID, int priority);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static LayerMask CameraComponent_GetLayerMask(ref Entity entity, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static void CameraComponent_SetLayerMask(ref Entity entity, UInt64 componentID, ref LayerMask pLayerMask);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static Vector4 CameraComponent_GetClearColor(ref Entity entity, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static void CameraComponent_SetClearColor(ref Entity entity, UInt64 componentID, ref Vector4 clearCol);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static UInt64 CameraComponent_GetCameraID(ref Entity entity, UInt64 componentID);

		#endregion
	}
}
