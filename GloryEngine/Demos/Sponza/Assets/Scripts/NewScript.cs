using System;
using GloryEngine;
using GloryEngine.Entities;

namespace Sponza.Testing
{
    public class NewScript : EntityBehaviour
    {
        bool called = false;

        public void Update()
        {
            if (called) return;
            Debug.LogInfo("Hello World!");
            called = true;

            Entity.IsValid();

            //Entity
        }
    }
}
