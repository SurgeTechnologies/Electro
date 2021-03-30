using System;
using System.Runtime.CompilerServices;

namespace Electro
{
    public class ElectroAPI
    {
        public ulong ID { get; private set; }

        protected ElectroAPI() { ID = 0; }

        internal ElectroAPI(ulong id)
        {
            ID = id;
        }

        ~ElectroAPI() {}
    }
}
