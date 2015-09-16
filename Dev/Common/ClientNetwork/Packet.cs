using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace ClientNetwork
{
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    struct PingPacket
    {
        public int x;
        public int y;
        public int z;

        public int Read(CReadBuffer buffer)
        {
            return buffer.Read(ref this);

        }

        public int Write(CWriteBuffer buffer)
        {
            return buffer.Write(ref this);
        }
    }
}
