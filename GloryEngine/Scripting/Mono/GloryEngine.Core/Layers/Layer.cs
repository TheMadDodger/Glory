namespace GloryEngine
{
    /// <summary>
    /// Layer objects can render to
    /// </summary>
    public struct Layer
    {
        /// <summary>
        /// Construct a layer
        /// </summary>
        /// <param name="name">Name</param>
        /// <param name="mask">Mask</param>
        public Layer(string name, LayerMask mask)
        {
            Name = name;
            Mask = mask;
        }

        /// <summary>
        /// Mask of the layer
        /// </summary>
        public LayerMask Mask;
        /// <summary>
        /// Name of the layer
        /// </summary>
        public string Name;
    }
}
