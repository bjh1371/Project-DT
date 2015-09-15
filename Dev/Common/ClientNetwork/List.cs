using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace ClientNetwork
{
    struct CList<T> where T : IPacket
    {
        private T[] list;
        private short count;

        public short Count { get { return count; } set { count = value; } }

        public CList(int length)
        {
            list = new T[length];
            count = 0;
        }

        public void Append(ref T value)
        {
            list[count] = value;
            ++count;
        }

        public T At(int index)
        {
            return list[index];
        }

        public int Read(CReadBuffer buf)
        {
            int read = 0;
            short size = 0;
            read += buf.Read(ref size);
            list = new T[size];

            for (int i = 0; i < size; ++i)
            {
                list[i].Read(buf);
            }

            return read;
        }

        public int ReadPod(CReadBuffer buf)
        {
            int read = 0;
            short size = 0;
            read += buf.Read(ref size);
            list = new T[size];

            int len = Marshal.SizeOf(typeof(T));
            buf.ReadArray(list, size * len);

            return read;
        }

        public int Write(CWriteBuffer buf)
        {
            int written = 0;
            buf.Write(count);

            for (int i = 0; i < count; ++i)
            {
                list[i].Write(buf);
            }

            return written;
        }

        public int WritePod(CWriteBuffer buf)
        {
            int written = 0;
            buf.Write(count);

            int len = Marshal.SizeOf(typeof(T));
            buf.WriteArray(list, len * count);

            return written;
        }
    }
}
