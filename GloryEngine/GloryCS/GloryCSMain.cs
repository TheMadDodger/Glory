using System;
using System.Runtime.CompilerServices;

namespace Glory
{
    public class GloryCSMain
    {
        public static void main()
        {
            Console.WriteLine(RandomWord.GetWord());
        }
    }

    public class RandomWord
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static string GetWord();
    }
}
