using System;
using System.Runtime.CompilerServices;

namespace GloryEngine
{
    /// <summary>
    /// Raw text file resource
    /// </summary>
    public class TextFile : Resource
    {
        #region Props

        /// <summary>
        /// Full body text of the text file
        /// </summary>
        public string FullBody => TextFile_GetFullBody(_objectID);

        #endregion

        #region Constructor

        public TextFile() : base() { }
        public TextFile(AssetManager manager, UInt64 id) : base(manager, id) { }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string TextFile_GetFullBody(UInt64 textID);

        #endregion
    }
}
