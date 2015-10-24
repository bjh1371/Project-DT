using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ClientNetwork
{
    class CPacketMarshaller
    {
        /// <summary>
        /// 패킷 헤더 크기만큼 왔는지 검사하고 실행된 함수
        /// </summary>
        public int UnMarshall(CClientPeer peer, byte[] buf, int total)
        {
            int handled = 0;

            do
            {
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
