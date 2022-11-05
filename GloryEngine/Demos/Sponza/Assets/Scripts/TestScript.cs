using System;
using GloryEngine;
using GloryEngine.Entities;

namespace Sponza
{
    public class Test : EntityBehaviour
    {
        private int _updateCounter = 0;
        private int _drawCounter = 0;

        public void Update()
        {
            Debug.LogInfo(string.Format("Update() has been called {0} times!", _updateCounter));
            ++_updateCounter;
        }

        public void Draw()
        {
            //Debug.LogInfo(string.Format("Draw() has been called {0} times!", _drawCounter));
            Debug.LogInfo(string.Format("Hello World! Draw() has been called {0} times!", _drawCounter));
            ++_drawCounter;
        }
    }
}