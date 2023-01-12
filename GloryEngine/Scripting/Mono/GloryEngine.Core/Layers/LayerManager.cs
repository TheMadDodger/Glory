using System.Runtime.CompilerServices;

namespace GloryEngine
{
	public static class LayerManager
    {
		#region API Methods

		/// <summary>
		/// Add a new layer
		/// </summary>
		/// <param name="name"></param>
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void AddLayer(string name);
		/// <summary>
		/// Get a layer by name
		/// </summary>
		/// <param name="name">Name of the layer</param>
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern Layer GetLayerByName(string name);
		/// <summary>
		/// Convert a layer mask to ',' separated string
		/// </summary>
		/// <param name="layerMask">Mask to convert</param>
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern string LayerMaskToString(ref LayerMask layerMask);
		/// <summary>
		/// Get the index of a specific layer
		/// </summary>
		/// <param name="pLayer">The layer to get the index from</param>
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern int GetLayerIndex(ref Layer pLayer);
		/// <summary>
		/// Get a layer at a specific index
		/// </summary>
		/// <param name="index">Index of the layer</param>
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern Layer GetLayerAtIndex(int index);

		#endregion
	}
}
