////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 자동으로 생성된 파일입니다. 손으로 수정하지마세요
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SharedStruct.h"

enum
{
	ePKTEcho = 0,                                                  ///< 에코 패킷
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------------------------------------------------------

// 에코 패킷
struct PKTEcho : public CPacket
{
	enum
	{
		ID = ePKTEcho,
	};

	CString<100> m_Str; ///< 스트링

	LPCTSTR GetStr() const { return m_Str; }
	void SetStr(LPCTSTR value) { m_Str = value; }

	//-----------------------------------------------------------------------------------

	virtual PacketId_t GetId() const override { return ID; } 

	virtual int ReadPayload(CPacketReadBuffer& buffer) override 
	{
		int read = 0;
		read += buffer.Read(m_Str);
		return read;
	}

	virtual int WritePayload(CPacketWriteBuffer& buffer) const override 
	{
		int written = 0;
		written += buffer.Write(m_Str);
		return written;
	}

	virtual int BytesPayload() const override 
	{
		 int size = 0;
		 size += CSizeCounter::Bytes(m_Str);
		 return size;
	}
};

template <> struct IsPod<PKTEcho> { enum { Value = false }; }; 
template <> struct MaxBytes<PKTEcho> { enum { Value = CPacket::PACKET_HEADER_FULL_LENGTH + MaxBytes<CString<100>>::Value }; }; 
