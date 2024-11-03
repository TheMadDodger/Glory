using GloryEngine.SceneManagement;
using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
	/// <summary>
	/// Handle for a CameraComponent component
	/// </summary>
	public class CameraComponent : NativeComponent
    {
		#region Props

		/// <summary>
		/// Half angle of the Field of View in degrees
		/// </summary>
		public float HalfFOV
		{
			get => CameraComponent_GetHalfFOV(_object.Scene.ID, _object.ID, _objectID);
			set => CameraComponent_SetHalfFOV(_object.Scene.ID, _object.ID, _objectID, value);
		}

		/// <summary>
		/// How close objects can get to the camera before they are clipped
		/// </summary>
		public float Near
		{
			get => CameraComponent_GetNear(_object.Scene.ID, _object.ID, _objectID);
			set => CameraComponent_SetNear(_object.Scene.ID, _object.ID, _objectID, value);
		}

		/// <summary>
		/// How far the camera should render before objects are clipped
		/// </summary>
		public float Far
		{
			get => CameraComponent_GetFar(_object.Scene.ID, _object.ID, _objectID);
			set => CameraComponent_SetFar(_object.Scene.ID, _object.ID, _objectID, value);
		}

		/// <summary>
		/// The index of the display this camera should render to
		/// This is an internal display
		/// </summary>
		public int DisplayIndex
		{
			get => CameraComponent_GetDisplayIndex(_object.Scene.ID, _object.ID, _objectID);
			set => CameraComponent_SetDisplayIndex(_object.Scene.ID, _object.ID, _objectID, value);
		}

		/// <summary>
		/// Controls rendering order, ordered by high to low
		/// </summary>
		public int Priority
		{
			get => CameraComponent_GetPriority(_object.Scene.ID, _object.ID, _objectID);
			set => CameraComponent_SetPriority(_object.Scene.ID, _object.ID, _objectID, value);
		}

		/// <summary>
		/// Mask of layers this camera renders
		/// </summary>
		public LayerMask LayerMask
		{
			get => CameraComponent_GetLayerMask(_object.Scene.ID, _object.ID, _objectID);
			set => CameraComponent_SetLayerMask(_object.Scene.ID, _object.ID, _objectID, ref value);
		}

		/// <summary>
		/// The color the target is cleared with before rendering the camera to it
		/// </summary>
		public Vector4 ClearColor
		{
			get => CameraComponent_GetClearColor(_object.Scene.ID, _object.ID, _objectID);
			set => CameraComponent_SetClearColor(_object.Scene.ID, _object.ID, _objectID, ref value);
		}

		/// <summary>
		/// The Camera handle for this component
		/// </summary>
		public Camera Camera => new Camera(CameraComponent_GetCameraID(_object.Scene.ID, _object.ID, _objectID));

		/// <summary>
		/// Get the pick result of the last frame
		/// </summary>
		public PickResult PickResult
		{
			get
			{
                PickResultInternal resultInternal = CameraComponent_GetPickResult(_object.Scene.ID, _object.ID, _objectID);
				PickResult result = new PickResult();
                result.Position = resultInternal.Position;
				result.Normal = resultInternal.Normal;
				result.Object = Object.Scene.GetSceneObject(resultInternal.ObjectID);
				result.CameraID = resultInternal.CameraID;
				return result;
            }
		}

		/// <summary>
		/// Get the rendering resolution for this camera
		/// </summary>
		public Vector2 Resolution => CameraComponent_GetResolution(_object.Scene.ID, _object.ID, _objectID);

        #endregion

        #region Methods

        /// <summary>
        /// Tell the renderer to pick the backbuffer at the provided screen position during the next render.
        /// Picking happens between regular and late rendering, so late rendered objects are ignored.
        /// NOTE: You can only prepare 1 pick per camera per frame!
        /// </summary>
        /// <param name="screenPos">Position to pick at</param>
        public void PreparePick(Vector2 screenPos) => CameraComponent_PrepareNextPick(_object.Scene.ID, _object.ID, _objectID, ref screenPos);

		#endregion

		#region API Methods

		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static float CameraComponent_GetHalfFOV(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static void CameraComponent_SetHalfFOV(UInt64 sceneID, UInt64 objectID, UInt64 componentID, float halfFov);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static float CameraComponent_GetNear(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static void CameraComponent_SetNear(UInt64 sceneID, UInt64 objectID, UInt64 componentID, float near);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static float CameraComponent_GetFar(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static void CameraComponent_SetFar(UInt64 sceneID, UInt64 objectID, UInt64 componentID, float far);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static int CameraComponent_GetDisplayIndex(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static void CameraComponent_SetDisplayIndex(UInt64 sceneID, UInt64 objectID, UInt64 componentID, int displayIndex);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static int CameraComponent_GetPriority(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static void CameraComponent_SetPriority(UInt64 sceneID, UInt64 objectID, UInt64 componentID, int priority);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static LayerMask CameraComponent_GetLayerMask(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static void CameraComponent_SetLayerMask(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref LayerMask pLayerMask);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static Vector4 CameraComponent_GetClearColor(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static void CameraComponent_SetClearColor(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector4 clearCol);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static UInt64 CameraComponent_GetCameraID(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void CameraComponent_PrepareNextPick(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Vector2 screenPos);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static PickResultInternal CameraComponent_GetPickResult(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Vector2 CameraComponent_GetResolution(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        #endregion
    }
}
