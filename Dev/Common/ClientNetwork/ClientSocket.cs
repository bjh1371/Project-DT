using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;

namespace ClientNetwork
{
    class CClientSocket
    {
        private Socket m_Socket;
        private IPAddress m_IpAddress;
        private int m_ErrorCount;
        private string m_ErrorMessage;

        public void Open()
        {
            if (m_Socket != null)
            {
                Close();
            }

            m_Socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            m_Socket.Blocking = false;
        }

        public void Close()
        {
            m_Socket.Close();
            m_Socket = null;
            m_IpAddress = null;
            m_ErrorCount = 0;
        }

        public void Connect(string addr, int port)
        {
            try
            {
                m_IpAddress = IPAddress.Parse(addr);
                IPEndPoint serverEndPoint = new IPEndPoint(m_IpAddress, port);

                m_Socket.Connect(serverEndPoint);
            }
            catch (SocketException e)
            {
                if (e.ErrorCode != 10035)
                {
                    ++m_ErrorCount;
                    m_ErrorMessage = e.Message;
                    // connect error
                }
            }
        }

        public bool IsConnected()
        {
            bool connected = m_Socket.Poll(10, SelectMode.SelectWrite);

            if (!connected)
            {
                // 접속 실패
                if (m_Socket.Poll(10, SelectMode.SelectError))
                {
                    ++m_ErrorCount;
                }
            }

            return connected;
        }

        public int GetErrorCount()
        {
            return m_ErrorCount;
        }

        public int Send(byte[] bytes, int offset, int len)
        {
            int sentBytes = 0;
            try
            {
                sentBytes = m_Socket.Send(bytes, offset, len, SocketFlags.None);
            }
            catch (SocketException e)
            {
                if (e.ErrorCode != 10035)
                {
                    m_ErrorMessage = e.Message;
                    ++m_ErrorCount;

                    // send error

                    sentBytes = -1;
                }
            }

            return sentBytes;
        }

        public int Recv(byte[] bytes, int offset, int len)
        {
            int recvBytes = 0;
            try
            {
                recvBytes = m_Socket.Receive(bytes, offset, len, SocketFlags.None);

                // graceful close
                if (recvBytes == 0)
                {
                    Close();
                }
            }
            catch (SocketException e)
            {
                if (e.ErrorCode != 10035)
                {
                    m_ErrorMessage = e.Message;
                    ++m_ErrorCount;

                    // recv error
                }
            }

            return recvBytes;
        }
    }
}
