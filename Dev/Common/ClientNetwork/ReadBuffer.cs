using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace ClientNetwork
{
    class CReadBuffer
    {
        private byte[] m_Array;
        private int m_offset;
        private int m_MaxSize;

        public CReadBuffer(byte[] array, int maxSize)
        {
            m_Array = array;
            m_offset = 0;
            m_MaxSize = maxSize;
        }

        public int Read<T>(ref T value)
        {
            int read = 0;

            if (m_offset <= m_MaxSize)
            {
                int len = Marshal.SizeOf(value);

                GCHandle pinnedArray = GCHandle.Alloc(m_Array, GCHandleType.Pinned);
                long memPtr = (long)pinnedArray.AddrOfPinnedObject() + m_offset;
                value = (T)Marshal.PtrToStructure(new IntPtr(memPtr), typeof(T));

                m_offset += len;
                read = len;
            }

            return read;
        }

        // char는 2바이트인데 위 마샬로 하면 1바이트로 인식해서 따로 오버로드해준다.
        public int Read(ref char value)
        {
            int read = 0;
            int len = sizeof(char);

            if (m_offset + len <= m_MaxSize)
            {
                GCHandle pinnedArray = GCHandle.Alloc(m_Array, GCHandleType.Pinned);
                long memPtr = (long)pinnedArray.AddrOfPinnedObject() + m_offset;
                value = (char)Marshal.PtrToStructure(new IntPtr(memPtr), typeof(char));

                m_offset += len;
                read = len;
            }

            return read;
        }

        public int ReadArray<T>(T[] arr, int len)
        {
            int read = 0;

            if (m_offset + len <= m_MaxSize)
            {
                System.Buffer.BlockCopy(m_Array, m_offset, arr, 0, len);
                m_offset += len;
            }

            return read;
        }

    }
}
