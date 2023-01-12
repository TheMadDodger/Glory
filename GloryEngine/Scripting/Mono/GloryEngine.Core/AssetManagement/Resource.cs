using System;
using System.Runtime.CompilerServices;

namespace GloryEngine
{
    public class Resource : Object
    {
        #region Props

        /// <summary>
        /// Name of the resource
        /// For main assets this is the file name
        /// For sub assets this is the generated name of the subasset
        /// </summary>
        public override string Name
        {
            get => Resource_GetName(_objectID);
            set => Resource_SetName(_objectID, value);
        }

        #endregion

        #region Constructor

        public Resource() : base() { }
        public Resource(UInt64 id) : base(id) { }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static string Resource_GetName(UInt64 objectID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void Resource_SetName(UInt64 objectID, string name);

        #endregion
    }
}
