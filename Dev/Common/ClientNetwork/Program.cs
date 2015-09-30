using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ClientNetwork
{
   
    class Program
    {
        // 서버와 통신하는 테스트 프로그램을 만들자.
        static void Main(string[] args)
        {
            CClientPeer peer = new CClientPeer();

            peer.Connect("127.0.0.1", 6010);

            if (peer.IsConnected() && peer.GetErrorCount() == 0)
            {
                
            }
        }
    }
}
