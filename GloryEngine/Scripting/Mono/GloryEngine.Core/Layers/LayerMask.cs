using System;

namespace GloryEngine
{
	public struct LayerMask
	{
		public LayerMask(UInt64 mask) { Mask = mask; }
		private UInt64 Mask;

		public static implicit operator UInt64(LayerMask mask) => mask.Mask;
		public static explicit operator LayerMask(UInt64 mask) => new LayerMask(mask);

		public override string ToString()
		{
			return LayerManager.LayerMaskToString(ref this);
		}
	}
}
