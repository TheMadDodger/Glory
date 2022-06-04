using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GloryEngine
{
    public class Behaviour : Object
    {
        public void Update()
        {
            Debug.LogInfo("Update was called!");
        }
    }
}
