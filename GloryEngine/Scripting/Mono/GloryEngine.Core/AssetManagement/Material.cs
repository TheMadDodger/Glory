using System;
using System.Runtime.CompilerServices;

namespace GloryEngine
{
    public class Material : Resource
    {
        #region Constructor

        public Material() : base() { }
        public Material(UInt64 id) : base(id) { }

        #endregion

        #region Methods

        public void SetFloat(string propertyName, float value)
        {
            Material_SetFloat(_objectID, propertyName, value);
        }

        public bool GetFloat(string propertyName, out float value)
        {
            return Material_GetFloat(_objectID, propertyName, out value);
        }

        public void SetDouble(string propertyName, double value)
        {
            Material_SetDouble(_objectID, propertyName, value);
        }

        public bool GetDouble(string propertyName, out double value)
        {
            return Material_GetDouble(_objectID, propertyName, out value);
        }

        public void SetInt(string propertyName, int value)
        {
            Material_SetInt(_objectID, propertyName, value);
        }

        public bool GetInt(string propertyName, out int value)
        {
            return Material_GetInt(_objectID, propertyName, out value);
        }

        public void SetUInt(string propertyName, uint value)
        {
            Material_SetUInt(_objectID, propertyName, value);
        }

        public bool GetUInt(string propertyName, out uint value)
        {
            return Material_GetUInt(_objectID, propertyName, out value);
        }

        public void SetBool(string propertyName, bool value)
        {
            Material_SetBool(_objectID, propertyName, value);
        }

        public bool GetBool(string propertyName, out bool value)
        {
            return Material_GetBool(_objectID, propertyName, out value);
        }

        public void SetVec2(string propertyName, Vector2 value)
        {
            Material_SetVec2(_objectID, propertyName, value);
        }

        public bool GeVec2(string propertyName, out Vector2 value)
        {
            return Material_GetVec2(_objectID, propertyName, out value);
        }

        public void SetVec3(string propertyName, Vector3 value)
        {
            Material_SetVec3(_objectID, propertyName, value);
        }

        public bool GeVec3(string propertyName, out Vector3 value)
        {
            return Material_GetVec3(_objectID, propertyName, out value);
        }

        public void SetVec4(string propertyName, Vector4 value)
        {
            Material_SetVec4(_objectID, propertyName, value);
        }

        public bool GeVec4(string propertyName, out Vector4 value)
        {
            return Material_GetVec4(_objectID, propertyName, out value);
        }

        public void SetTexture(string propertyName, Image value)
        {
            Material_SetTexture(_objectID, propertyName, value != null ? value.ID : 0);
        }

        public bool GetTexture(string propertyName, out Image value)
        {
            value = null;
            UInt64 imageID;
            if (!Material_GetTexture(_objectID, propertyName, out imageID)) return false;
            if (imageID == 0) return true;
            value = AssetManager.MakeResource<Image>(imageID);
            return true;
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Material_SetFloat(UInt64 materialID, string propertyName, float value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Material_GetFloat(UInt64 materialID, string propertyName, out float value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Material_SetDouble(UInt64 materialID, string propertyName, double value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Material_GetDouble(UInt64 materialID, string propertyName, out double value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Material_SetInt(UInt64 materialID, string propertyName, int value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Material_GetInt(UInt64 materialID, string propertyName, out int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Material_SetUInt(UInt64 materialID, string propertyName, uint value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Material_GetUInt(UInt64 materialID, string propertyName, out uint value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Material_SetBool(UInt64 materialID, string propertyName, bool value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Material_GetBool(UInt64 materialID, string propertyName, out bool value);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Material_SetVec2(UInt64 materialID, string propertyName, Vector2 value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Material_GetVec2(UInt64 materialID, string propertyName, out Vector2 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Material_SetVec3(UInt64 materialID, string propertyName, Vector3 value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Material_GetVec3(UInt64 materialID, string propertyName, out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Material_SetVec4(UInt64 materialID, string propertyName, Vector4 value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Material_GetVec4(UInt64 materialID, string propertyName, out Vector4 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Material_SetTexture(UInt64 materialID, string propertyName, UInt64 value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Material_GetTexture(UInt64 materialID, string propertyName, out UInt64 value);

        #endregion
    }
}
