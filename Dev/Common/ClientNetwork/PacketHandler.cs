using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ClientNetwork
{
    class PacketHandler
    {
        public int Execute(CClientPeer peer, byte[] buf, int total)
        {
            CReadBuffer readBuf = new CReadBuffer(buf, total);
            buf.Skip((int)PacketHeader.PACKET_HEADER_FULL_LENGTH);

            PingPacket ping = new PingPacket();
            int read = ping.Read(readBuf);

            return read;
        }
    }
}
