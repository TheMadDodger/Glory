namespace GloryEngine
{
    /// <summary>
    /// Layer objects can render to
    /// </summary>
    public struct Layer
    {
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
