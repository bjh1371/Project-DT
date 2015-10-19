using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace ClientNetwork
{
   class CPacketBase
   {
       private static int PACKET_HEADER_LENGTH = sizeof(short) + sizeof(short);
       private static int PACKET_HEADER_FULL_LENGTH = sizeof(short) + sizeof(short) + sizeof(short);

       public static int HeaderLength
       {
           get { return PACKET_HEADER_LENGTH; }
       }

       public static int HeaderFullLength
       {
           get { return PACKET_HEADER_FULL_LENGTH; }
       }

   }
    
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
                read = len;
            }

            return read;
        }

        public void Skip(int len)
        {
            if (m_offset + len <= m_MaxSize)
            {
                m_offset += len;
            }
        }

    }

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

        public void WritePacketSize(short size)
        {
            // packet Id 다음에 사이즈를 복사 시킴
            BitConverter.GetBytes(size).CopyTo(m_Array, sizeof(short));
        }

        public int WriteArray<T>(T[] array, int len)
        {
            int written = 0;

            if (m_offset + len <= m_MaxSize)
            {
                System.Buffer.BlockCopy(array, 0, m_Array, m_offset, len);
                m_offset += len;
                written = len;
            }

            return written;
        }

        public int Write(int data)
        {
            int written = 0;
            BitConverter.GetBytes(data).CopyTo(m_Array, m_offset);
            m_offset += sizeof(int);
            written = sizeof(int);
            return written;
        }

        public int Write(short data)
        {
            int written = 0;
            BitConverter.GetBytes(data).CopyTo(m_Array, m_offset);
            m_offset += sizeof(short);
            written = sizeof(short);
            return written;
        }

    }

    interface IPacket
    {
        int Read(CReadBuffer buffer);

        int Write(CWriteBuffer buffer);

        short GetId();
    }

    struct CString : IPacket
    {
        private char[] arr;

        public CString(string str)
        {
            arr = new char[str.Length];
            System.Buffer.BlockCopy(str.ToArray(), 0, arr, 0, str.Length * sizeof(char));
        }

        public static implicit operator CString(string str)
        {
            return new CString(str);
        }

        public string ToStr()
        {
            return new string(arr);
        }

        public short GetId() { return -1; }

        public int Read(CReadBuffer buf)
        {
            int read = 0;

            short size = 0;
            read += buf.Read(ref size);

            // 사이즈 할당
            arr = new char[size];
            read += buf.ReadArray(arr, sizeof(char) * size);

            return read;
        }

        public int Write(CWriteBuffer buf)
        {
            int written = 0;

            short size = (short)arr.Length;
            written += buf.Write(size);
            written += buf.WriteArray(arr, sizeof(char) * size);

            return written;
        }
    }

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

        public short GetId() { return -1; }

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
