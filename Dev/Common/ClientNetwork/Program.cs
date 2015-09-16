using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ClientNetwork
{
    interface IPacket
    {
        int Read(CReadBuffer buffer);

        int Write(CWriteBuffer buffer);
    }
    class Program
    {
        static void Main(string[] args)
        {
            
        }
    }
}
