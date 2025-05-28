using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Localize
{
    /// <summary>
    /// String table resource
    /// </summary>
    public class StringTable : Resource
    {
        #region Constructor

        public StringTable() : base() { }
        public StringTable(AssetManager manager, UInt64 id) : base(manager, id) { }

        #endregion

        /// <summary>
        /// Find keys belonging to a group at a specific path
        /// </summary>
        /// <param name="path">Path to group</param>
        /// <returns>Array with all keys in the group</returns>
        public string[] FindKeys(string path) => StringTable_FindKeys(_objectID, path);
        /// <summary>
        /// Find keys belonging to a group at a specific path, without allocating an array
        /// </summary>
        /// <param name="path">Path to group</param>
        /// <param name="array">Array to populate with result</param>
        /// <param name="maxKeys">Maximum number of keys to search for</param>
        /// <returns>Number of keys found</returns>
        public UInt32 FindKeys(string path, string[] array, UInt32 maxKeys) =>
            StringTable_FindKeysNoAlloc(_objectID, path, array, maxKeys);
        /// <summary>
        /// Find keys belonging to a group and all subgroups of that group recursively
        /// </summary>
        /// <param name="path">Path to group</param>
        /// <returns>Array with all keys in the group</returns>
        public string[] FindKeysRecursive(string path) => StringTable_FindKeysRecursive(_objectID, path);
        /// <summary>
        /// Find keys belonging to a group and all subgroups of that group recursively, without allocating an array
        /// </summary>
        /// <param name="path">Path to group</param>
        /// <param name="array">Array to populate with result</param>
        /// <param name="maxKeys">Maximum number of keys to search for</param>
        /// <returns>Number of keys found</returns>
        public UInt32 FindKeysRecursive(string path, string[] array, UInt32 maxKeys) =>
            StringTable_FindKeysRecursiveNoAlloc(_objectID, path, array, maxKeys);
        /// <summary>
        /// Find subgroup names belonging to a group at a specific path
        /// </summary>
        /// <param name="path">Path to group</param>
        /// <returns>Array with all subgroup names in the group</returns>
        public string[] FindSubgroups(string path) => StringTable_FindSubgroups(_objectID, path);
        /// <summary>
        /// Find subgroup names belonging to a group at a specific path, without allocating an array
        /// </summary>
        /// <param name="path">Path to group</param>
        /// <param name="array">Array to populate with result</param>
        /// <param name="maxGroups">Maximum number of group names to search for</param>
        /// <returns></returns>
        public UInt32 FindSubgroups(string path, string[] array, UInt32 maxGroups) =>
            StringTable_FindSubgroupsNoAlloc(_objectID, path, array, maxGroups);

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string[] StringTable_FindKeys(UInt64 tableID, string path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt32 StringTable_FindKeysNoAlloc(UInt64 tableID, string path, string[] array, UInt32 maxKeys);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string[] StringTable_FindKeysRecursive(UInt64 tableID, string path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt32 StringTable_FindKeysRecursiveNoAlloc(UInt64 tableID, string path, string[] array, UInt32 maxKeys);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string[] StringTable_FindSubgroups(UInt64 tableID, string path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt32 StringTable_FindSubgroupsNoAlloc(UInt64 tableID, string path, string[] array, UInt32 maxKeys);

        #endregion
    }
}
