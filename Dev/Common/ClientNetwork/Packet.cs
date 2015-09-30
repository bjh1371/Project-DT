using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace ClientNetwork
{
    // [StructLayout(LayoutKind.Sequential, Pack = 1)]
    struct PingPacket : IPacket
    {
        public CString m_Str;

        public int GetId() { return 0; }

        public int Read(CReadBuffer buffer)
        {
            return m_Str.Read(buffer);
        }

        public int Write(CWriteBuffer buffer)
        {
            return m_Str.Write(buffer);
        }
    }
}
