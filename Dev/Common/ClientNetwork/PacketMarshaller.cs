using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ClientNetwork
{
    class CPacketMarshaller
    {
        public int UnMarshall(CClientPeer peer, byte[] buf, int total)
        {
            int handled = 0;

            do
            {
                // 헤더 넘게 왔나?
                if (total < CPacketBase.HeaderLength)
                    break;

                // 패킷 Id
                short id = 0;
                unsafe
                {
                    fixed (byte* p = &buf[0])
                    {
                        id = *(short*)p;
                    }
                }

                // 패킷 size
                short size = 0;
                unsafe
                {
                    fixed (byte* p = &buf[2])
                    {
                        size = *(short*)p;
                    }
                }

                // 사이즈 만큼 왔나?
                if (total < size)
                    break;

                // 패킷 처리

                

            } while (false);


            return handled;
        }

        public void Marshall(byte[] byf, int offset, int len)
        {

        }
    }
}
