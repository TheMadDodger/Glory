using System;
using System.Runtime.CompilerServices;

namespace GloryEngine
{
    /// <summary>
    /// Material
    /// </summary>
    public class Material : Resource
    {
        #region Constructor

        public Material() : base() { }
        public Material(AssetManager manager, UInt64 id) : base(manager, id) { }

        #endregion

        #region Methods

        /// <summary>
        /// Set the value of a float property
        /// If the property is not found nothing happens
        /// </summary>
        /// <param name="propertyName">Name of the property</param>
        /// <param name="value">Float value to set the property with</param>
        public void SetFloat(string propertyName, float value)
        {
            Material_SetFloat(_objectID, propertyName, value);
        }

        /// <summary>
        /// Get the current float value of a property
        /// </summary>
        /// <param name="propertyName">Name of the property</param>
        /// <param name="value">Value to write to</param>
        /// <returns>True if successfull, false if the property does not exist</returns>
        public bool GetFloat(string propertyName, out float value)
        {
            return Material_GetFloat(_objectID, propertyName, out value);
        }

        /// <summary>
        /// Set the value of a double property
        /// If the property is not found nothing happens
        /// </summary>
        /// <param name="propertyName">Name of the property</param>
        /// <param name="value">double value to set the property with</param>
        public void SetDouble(string propertyName, double value)
        {
            Material_SetDouble(_objectID, propertyName, value);
        }

        /// <summary>
        /// Get the current double value of a property
        /// </summary>
        /// <param name="propertyName">Name of the property</param>
        /// <param name="value">Value to write to</param>
        /// <returns>True if successfull, false if the property does not exist</returns>
        public bool GetDouble(string propertyName, out double value)
        {
            return Material_GetDouble(_objectID, propertyName, out value);
        }

        /// <summary>
        /// Set the value of an int property
        /// If the property is not found nothing happens
        /// </summary>
        /// <param name="propertyName">Name of the property</param>
        /// <param name="value">Integer value to set the property with</param>
        public void SetInt(string propertyName, int value)
        {
            Material_SetInt(_objectID, propertyName, value);
        }

        /// <summary>
        /// Get the current int value of a property
        /// </summary>
        /// <param name="propertyName">Name of the property</param>
        /// <param name="value">Value to write to</param>
        /// <returns>True if successfull, false if the property does not exist</returns>
        public bool GetInt(string propertyName, out int value)
        {
            return Material_GetInt(_objectID, propertyName, out value);
        }

        /// <summary>
        /// Set the value of an unsigned int property
        /// If the property is not found nothing happens
        /// </summary>
        /// <param name="propertyName">Name of the property</param>
        /// <param name="value">Unsigned integer value to set the property with</param>
        public void SetUInt(string propertyName, uint value)
        {
            Material_SetUInt(_objectID, propertyName, value);
        }

        /// <summary>
        /// Get the current unsigned int value of a property
        /// </summary>
        /// <param name="propertyName">Name of the property</param>
        /// <param name="value">Value to write to</param>
        /// <returns>True if successfull, false if the property does not exist</returns>
        public bool GetUInt(string propertyName, out uint value)
        {
            return Material_GetUInt(_objectID, propertyName, out value);
        }

        /// <summary>
        /// Set the value of a bool property
        /// If the property is not found nothing happens
        /// </summary>
        /// <param name="propertyName">Name of the property</param>
        /// <param name="value">Boolean value to set the property with</param>
        public void SetBool(string propertyName, bool value)
        {
            Material_SetBool(_objectID, propertyName, value);
        }

        /// <summary>
        /// Get the current bool value of a property
        /// </summary>
        /// <param name="propertyName">Name of the property</param>
        /// <param name="value">Value to write to</param>
        /// <returns>True if successfull, false if the property does not exist</returns>
        public bool GetBool(string propertyName, out bool value)
        {
            return Material_GetBool(_objectID, propertyName, out value);
        }

        /// <summary>
        /// Set the value of a Vector2 property
        /// If the property is not found nothing happens
        /// </summary>
        /// <param name="propertyName">Name of the property</param>
        /// <param name="value">Vector2 value to set the property with</param>
        public void SetVec2(string propertyName, Vector2 value)
        {
            Material_SetVec2(_objectID, propertyName, value);
        }

        /// <summary>
        /// Get the current Vector2 value of a property
        /// </summary>
        /// <param name="propertyName">Name of the property</param>
        /// <param name="value">Value to write to</param>
        /// <returns>True if successfull, false if the property does not exist</returns>
        public bool GetVec2(string propertyName, out Vector2 value)
        {
            return Material_GetVec2(_objectID, propertyName, out value);
        }

        /// <summary>
        /// Set the value of a Vector3 property
        /// If the property is not found nothing happens
        /// </summary>
        /// <param name="propertyName">Name of the property</param>
        /// <param name="value">Vector3 value to set the property with</param>
        public void SetVec3(string propertyName, Vector3 value)
        {
            Material_SetVec3(_objectID, propertyName, value);
        }

        /// <summary>
        /// Get the current Vector3 value of a property
        /// </summary>
        /// <param name="propertyName">Name of the property</param>
        /// <param name="value">Value to write to</param>
        /// <returns>True if successfull, false if the property does not exist</returns>
        public bool GetVec3(string propertyName, out Vector3 value)
        {
            return Material_GetVec3(_objectID, propertyName, out value);
        }

        /// <summary>
        /// Set the value of a Vector4 property
        /// If the property is not found nothing happens
        /// </summary>
        /// <param name="propertyName">Name of the property</param>
        /// <param name="value">Vector4 value to set the property with</param>
        public void SetVec4(string propertyName, Vector4 value)
        {
            Material_SetVec4(_objectID, propertyName, value);
        }

        /// <summary>
        /// Get the current Vector4 value of a property
        /// </summary>
        /// <param name="propertyName">Name of the property</param>
        /// <param name="value">Value to write to</param>
        /// <returns>True if successfull, false if the property does not exist</returns>
        public bool GetVec4(string propertyName, out Vector4 value)
        {
            return Material_GetVec4(_objectID, propertyName, out value);
        }

        /// <summary>
        /// Bind a texture to a material property
        /// If the property is not found nothing happens
        /// </summary>
        /// <param name="propertyName">Name of the property</param>
        /// <param name="value">Texture to bind the property to</param>
        public void SetTexture(string propertyName, Texture value)
        {
            Material_SetTexture(_objectID, propertyName, value != null ? value.ID : 0);
        }

        /// <summary>
        /// Get the texture currently bound to a property
        /// </summary>
        /// <param name="propertyName">Name of the property</param>
        /// <param name="value">The out Texture</param>
        /// <returns>True if successfull, false if the property does not exist</returns>
        public bool GetTexture(string propertyName, out Texture value)
        {
            value = null;
            UInt64 imageID;
            if (!Material_GetTexture(_objectID, propertyName, out imageID)) return false;
            if (imageID == 0) return true;
            value = Manager.MakeResource<Texture>(imageID);
            return true;
        }

        /// <summary>
        /// Create a material instance from this material
        /// </summary>
        /// <returns>The newly created material instance</returns>
        [System.Obsolete("Material.CreateMaterialInstance has been deprecated, please use Material.CreateCopy instead.")]
        public MaterialInstance CreateMaterialInstance()
        {
            throw new System.Exception("Material.CreateMaterialInstance has been deprecated, please use Material.CreateCopy instead.");
        }

        /// <summary>
        /// Create a runtime copy from this material
        /// </summary>
        /// <returns>The newly created material</returns>
        public Material CreateCopy()
        {
            UInt64 instanceID = Material_CreateCopy(_objectID);
            return instanceID != 0 ? Manager.MakeResource<Material>(instanceID) : null;
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

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 Material_CreateCopy(UInt64 materialID);

        #endregion
    }
}
