using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace ClientNetwork
{
    class CWriteBuffer
    {
        private byte[] m_Array;
        private int m_offset;
        private int m_MaxSize;

        public CWriteBuffer(byte[] array, int maxSize)
        {
            m_Array = array;
            m_offset = 0;
            m_MaxSize = maxSize;
        }

        public int Write<T>(ref T value)
        {
            int written = 0;
            if (m_offset <= m_MaxSize)
            {
                int len = Marshal.SizeOf(value);

                GCHandle pinnedArray = GCHandle.Alloc(m_Array, GCHandleType.Pinned);
                long memPtr = (long)pinnedArray.AddrOfPinnedObject() + m_offset;
                Marshal.StructureToPtr(value, new IntPtr(memPtr), false);

                m_offset += len;
                written = len;
            }

            return written;
        }

        public int WriteArray<T>(T[] array, int len)
        {
            int written = 0;

            if (m_offset + len <= m_MaxSize)
            {
                System.Buffer.BlockCopy(array, 0, m_Array, m_offset, len);
                m_offset += len;
            }

            return written;
        }

        public int Write(int data)
        {
            int written = 0;
            BitConverter.GetBytes(data).CopyTo(m_Array, m_offset);
            m_offset += sizeof(int);
            return written;
        }

        public int Write(short data)
        {
            int written = 0;
            BitConverter.GetBytes(data).CopyTo(m_Array, m_offset);
            m_offset += sizeof(short);
            return written;
        }

    }

}
