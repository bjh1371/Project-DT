using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ClientNetwork
{
    class CClientPeer
    {
        private CClientStreamBuffer m_SendBuffer;
        private CClientStreamBuffer m_RecvBuffer;
        private CPacketMarshaller m_Marshaller;
        private CClientSocket m_Socket;


        public CClientPeer()
        {
            m_SendBuffer = new CClientStreamBuffer(65535);
            m_RecvBuffer = new CClientStreamBuffer(65535);
            m_Marshaller = new CPacketMarshaller();
            m_Socket = new CClientSocket();
        }

        ~CClientPeer()
        {
            m_Socket.Close();
        }

        public void FillFromSocket()
        {
            m_RecvBuffer.FillFromoSocket(m_Socket);
        }

        public int Flush()
        {
            return m_SendBuffer.FlushToSocket(m_Socket);
        }

        public void Execute()
        {
            int handled = 0;
            while (m_RecvBuffer.GetLength() > CPacketBase.HeaderFullLength)
            {
                int result = m_Marshaller.UnMarshall(this, m_RecvBuffer.GetStream(), m_RecvBuffer.GetLength());
                if (result == 0)
                    break;
                else
                {
                    handled += result;
                    m_RecvBuffer.Skip(result);
                }
            }

            m_RecvBuffer.Adjust(handled);
        }


        public void Connect(string ipAddress, short port)
        {
            m_Socket.Open();
            m_Socket.Connect(ipAddress, port);
        }

        public void Send(IPacket packet)
        {
            short tag = 1;
            short test = 0;
            byte[] buf = new byte[100];
            CWriteBuffer writeBuf = new CWriteBuffer(buf, 100);

            int written = writeBuf.Write(packet.GetId());
            written += writeBuf.Write(test);
            written += writeBuf.Write(tag);
            written += packet.Write(writeBuf);
            writeBuf.WritePacketSize((short)written);

            m_Marshaller.Marshall(buf, CPacketBase.HeaderLength, written - CPacketBase.HeaderLength);
            m_SendBuffer.Write(buf, written);
        }


        public bool IsConnected()
        {
            return m_Socket.IsConnected();
        }

        public int GetErrorCount()
        {
            return m_Socket.GetErrorCount();
        }

        public void Reset()
        {
            m_Socket.Close();
            m_RecvBuffer.Reset();
            m_SendBuffer.Reset();
        }
    }
}
