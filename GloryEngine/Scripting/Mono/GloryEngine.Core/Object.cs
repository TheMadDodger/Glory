using System;
using System.Runtime.CompilerServices;

namespace GloryEngine
{
    /// <summary>
    /// Basic object handle
    /// </summary>
    public class Object
    {
        #region Props

        /// <summary>
        /// ID of this object
        /// </summary>
        public UInt64 ID => _objectID;

        /// <summary>
        /// Name of the object
        /// </summary>
        public virtual string Name
        {
            get => Object_GetName(_objectID);
            set => Object_SetName(_objectID, value);
        }

        #endregion

        #region Fields

        /// <summary>
        /// ID of this object
        /// </summary>
        protected UInt64 _objectID;

        #endregion

        #region Constructor

        public Object()
        {
            _objectID = 0;
        }

        public Object(UInt64 id)
        {
            _objectID = id;
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static string Object_GetName(UInt64 objectID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void Object_SetName(UInt64 objectID, string name);

        #endregion
    }
}
