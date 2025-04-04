namespace GloryEngine
{
    /// <summary>
    /// Version structure
    /// </summary>
    public struct Version
    {
        #region Props

        /// <summary>
        /// Major version
        /// </summary>
        public int Major => _major;
        /// <summary>
        /// Minor version
        /// </summary>
        public int Minor => _minor;
        /// <summary>
        /// Sub minor version or patch
        /// </summary>
        public int SubMinor => _subMinor;
        /// <summary>
        /// Release candidate
        /// </summary>
        public int RC => _rc;

        #endregion

        #region Fields

        private int _major;
        private int _minor;
        private int _subMinor;
        private int _rc;

        #endregion

        #region Constructor

        /// <summary>
        /// Constrcutor
        /// </summary>
        /// <param name="major">Major version</param>
        /// <param name="minor">Minor version</param>
        /// <param name="subMinor">Sub minor version or patch</param>
        /// <param name="rc">Release candidate</param>
        public Version(int major, int minor, int subMinor, int rc)
        {
            _major = major;
            _minor = minor;
            _subMinor = subMinor;
            _rc = rc;
        }

        #endregion

        #region Methods

        /// <summary>
        /// Compare 2 versions
        /// </summary>
        /// <param name="v1">Version 1</param>
        /// <param name="v2">Version 2</param>
        /// <param name="ignoreRC"></param>
        /// <returns>-1 if v1 is less than v2, +1 for the opposite, 0 if they are identical</returns>
        public static int Compare(Version v1, Version v2, bool ignoreRC = false)
        {
            if (v1.Major != v2.Major)
            {
                if (v1.Major < v2.Major) return -1;
                if (v1.Major > v2.Major) return 1;
            }

            if (v1.Minor != v2.Minor)
            {
                if (v1.Minor < v2.Minor) return -1;
                if (v1.Minor > v2.Minor) return 1;
            }

            if (v1.SubMinor != v2.SubMinor)
            {
                if (v1.SubMinor < v2.SubMinor) return -1;
                if (v1.SubMinor > v2.SubMinor) return 1;
            }

            if (ignoreRC) return 0;

            /* RC 0 is always considered latest! */
            if (v1.RC == v2.RC) return 0;
            if (v1.RC == 0) return 1;
            if (v2.RC == 0) return -1;
            return v1.RC > v2.RC ? 1 : -1;
        }

        #endregion
    }
}
