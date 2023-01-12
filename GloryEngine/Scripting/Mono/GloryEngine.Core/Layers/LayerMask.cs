using System;

namespace GloryEngine
{
	/// <summary>
	/// A mask representing 1or more layers
	/// Default layer if mask = 0
	/// </summary>
	public struct LayerMask
	{
		public LayerMask(UInt64 mask) { Mask = mask; }
		/// <summary>
		/// Mask flags
		/// </summary>
		private UInt64 Mask;

		public static implicit operator UInt64(LayerMask mask) => mask.Mask;
		public static explicit operator LayerMask(UInt64 mask) => new LayerMask(mask);

		/// <summary>
		/// Gets all layers enabled by this mask in a string separated by ','
		/// </summary>
		public override string ToString()
		{
			return LayerManager.LayerMaskToString(ref this);
		}
	}
}
