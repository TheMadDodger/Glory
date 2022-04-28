using GloryEngine;

namespace Glory
{
    public class GloryCSMain
    {
        public static void main()
        {
            Debug.Log("Test Warning", LogLevel.Warning);
            Vector2 vecA = new Vector2(0.0f, 25.0f);
            Vector2 vecB = new Vector2(25.0f, -25.0f);
            Vector2 vecC = vecA + vecB;
            Vector2 vecD = vecA - vecB;
            Vector2 vecE = vecA * vecB;
            Vector2 vecF = vecA / vecB;
            Vector2 vecG = vecA * 10.0f;
            Vector2 vecH = vecA / 10.0f;
            Debug.Log(vecC.ToString(), LogLevel.Info);
            Debug.Log(vecD.ToString(), LogLevel.Info);
            Debug.Log(vecE.ToString(), LogLevel.Info);
            Debug.Log(vecF.ToString(), LogLevel.Info);
            Debug.Log(vecG.ToString(), LogLevel.Info);
            Debug.Log(vecH.ToString(), LogLevel.Info);
        }
    }
}
