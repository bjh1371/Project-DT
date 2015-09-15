using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ClientNetwork
{
    struct CString
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

        public int Read(CReadBuffer buf)
        {
            int read = 0;

            short size = 0;
            read += buf.Read(ref size);

            // 사이즈 할당
            arr = new char[size];

            read += buf.ReadArray(arr, sizeof(char) * size);

            // c++과 호환을 위해 null 문자열
            char temp = ' ';
            read += buf.Read(ref temp);

            return read;
        }

        public int Write(CWriteBuffer buf)
        {
            int written = 0;

            short size = (short)arr.Length;
            written += buf.Write(size);
            written += buf.WriteArray(arr, sizeof(char) * size);

            // c++과 호환을 위해 null 문자열
            written += buf.Write((char)0);
            return written;
        }
    }
}
