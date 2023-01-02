using System.Runtime.CompilerServices;

namespace GloryEngine
{
	public static class LayerManager
    {
		#region API Methods

		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void AddLayer(string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern Layer GetLayerByName(string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern string LayerMaskToString(ref LayerMask layerMask);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern int GetLayerIndex(ref Layer pLayer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern Layer GetLayerAtIndex(int index);

		#endregion
	}
}
