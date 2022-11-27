namespace GloryEngine
{
    public struct Layer
    {
        public Layer(string name, LayerMask mask)
        {
            Name = name;
            Mask = mask;
        }

        public LayerMask Mask;
        public string Name;
    }
}
