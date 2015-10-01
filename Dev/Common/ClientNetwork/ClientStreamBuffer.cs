using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ClientNetwork
{
    class CClientStreamBuffer
    {
        private byte[] m_ByteArray;
        private int    m_Capacity;
        private int    m_Head;
        private int    m_Tail;


        public CClientStreamBuffer(int capacity)
        {
            m_ByteArray = new byte[capacity];
            m_Capacity = capacity;
            m_Head = 0;
            m_Tail = 0;
        }

        public void FillFromoSocket(CClientSocket socket)
        {
            if (m_Tail < m_Capacity)
            {
                int result = socket.Recv(m_ByteArray, m_Tail, m_Capacity - m_Tail);

                if (result > 0)
                {
                    m_Tail += result;
                }
            }
        }

        public int FlushToSocket(CClientSocket socket)
        {
            int result = 0;
            if (m_Head < m_Tail && m_Tail > 0)
            {
                int sent = 0;
                while (m_Head < m_Tail)
                {
                    sent = socket.Send(m_ByteArray, m_Head, m_Tail - m_Head);
                    if (sent == -1)
                    {
                        break;
                    }

                    result += sent;
                    m_Head += sent;
                }
            }

            if (result > 0)
            {
                Adjust(result);
            }

            return result;
        }

        public void Write(byte[] data, int length)
        {
            if (m_Tail + length <= m_Capacity)
            {
                System.Buffer.BlockCopy(data, 0, m_ByteArray, m_Tail, length);
                m_Tail += length;
            }
            else
            {
                // buffer overflow
            }
        }

        public void Skip(int handled)
        {
            if (m_Head + handled <= m_Tail)
            {
                m_Head += handled;
            }
            else
            {
                // buffer invalid;
            }
        }

        public void Adjust(int handled)
        {
            if (handled > 0)
            {
                System.Buffer.BlockCopy(m_ByteArray, handled, m_ByteArray, 0, m_Tail - handled);
            }

            m_Tail -= handled;
            m_Head = 0;
        }
        
        public void Reset()
        {
            m_Head = 0;
            m_Tail = 0;
        }

        public int GetLength() { return m_Tail - m_Head; }
        
        public byte[] GetStream() { return m_ByteArray; }
    }
    
  
}
