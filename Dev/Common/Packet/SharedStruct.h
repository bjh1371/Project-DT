////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 자동으로 생성된 파일입니다. 손으로 수정하지마세요
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "PacketBase.h"
#include "PacketElement.h"


//----------------------------------------------------------------------------------------------------------------------

// 월드 정보
struct CWorldData
{
	WorldId_t                      m_Id;      ///< 월드 ID
	CString<MAX_WORLD_NAME_LENGTH> m_Name;    ///< 월드 이름
	char                           m_PCCount; ///< 계정에 있는 PC의 갯수

	WorldId_t GetId() const { return m_Id; }
	void SetId(WorldId_t value) { m_Id = value; }

	const TCHAR* GetName() { return m_Name; }
	void SetName(const TCHAR* value) { m_Name = value; }

	char GetPCCount() const { return m_PCCount; }
	void SetPCCount(char value) { m_PCCount = value; }

	//-----------------------------------------------------------------------------------

	int Read(CPacketReadBuffer& buffer)
	{
		int size = 0;
		size += buffer.Read(m_Id);
		size += buffer.Read(m_Name);
		size += buffer.Read(m_PCCount);
		return size;
	}

	int Write(CPacketWriteBuffer& buffer) const
	{
		int size = 0;
		size += buffer.Write(m_Id);
		size += buffer.Write(m_Name);
		size += buffer.Write(m_PCCount);
		return size;
	}

	int Bytes() const
	{
		int size = 0;
		size += CSizeCounter::Bytes(m_Id);
		size += CSizeCounter::Bytes(m_Name);
		size += CSizeCounter::Bytes(m_PCCount);
		return size;
	}
};

template <> struct IsPod < CWorldData > { enum { Value = false }; };
