////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file PacketBuffer.h
/// \author bjh1371
/// \date 2014/10/09
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Base/AsyncTcpEvent.h"
#include "Packet/PacketBase.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CPacketWriter
/// \brief 패킷 복사를 줄이기위해서 만든 클래스
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class CPacketWriter
{
private:
	T*              m_PacketPtr; ///< 패킷 포인터
	T               m_Packet;    ///< 패킷 객체
	CAsyncTcpEvent* m_Event;     ///< 이벤트 객체


public:
	/// \brief 생성자
	CPacketWriter()
	:
	m_Event(nullptr),
	m_PacketPtr(nullptr)
	{
		if (IsPod<T>::Value)
		{
			int size = CPacket::PACKET_HEADER_LENGTH + sizeof(T) - sizeof(void*);
			m_Event = CAsyncTcpEvent::Alloc(size);
			m_Event->IncreaseRefCount();
			m_Event->m_TotalBytes = size;

			*CPacket::GetPacketIdPtr(m_Event->m_Buf) = T::ID;
			*CPacket::GetPacketSizePtr(m_Event->m_Buf) = size;
			m_PacketPtr = reinterpret_cast<T*>(m_Event + CPacket::PACKET_HEADER_LENGTH - sizeof(void*));
		}
		else
		{
			m_PacketPtr = &m_Packet;
		}
	}

	/// \brief 소멸자
	~CPacketWriter()
	{
		if (m_Event && m_Event->DecreaseRefCount() == 0)
		{
			CAsyncTcpEvent::Dealloc(m_Event);
		}
	}


public:
	/// \brief 패킷에 접근
	T* operator-> () { return m_PacketPtr; }

	/// \brief 형변환 
	operator CAsyncTcpEvent* ()
	{
		// m_Event가 nullptr이라는것은 POD가 아니라는뜻
		// POD가 아니면 패킷내용을 버퍼로 복사한다.
		if (m_Event == nullptr)
		{
			int size = CPacket::PACKET_HEADER_FULL_LENGTH + m_Packet.BytesPayload();
			m_Event = CAsyncTcpEvent::Alloc(size);
			m_Event->IncreaseRefCount();
			m_Event->m_TotalBytes = size;

			memset(m_Event->m_Buf, 0, size);
			CPacketWriteBuffer buffer(m_Event->m_Buf, size);
			m_Packet.Write(buffer);
		}

		return m_Event;
	}

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CPacketReader
/// \brief 패킷 복사를 줄이기위해서 만든 클래스
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class CPacketReader
{
private:
	const T* m_PacketPtr; ///< 패킷 포인터
	T        m_Packet;    ///< 패킷
	int      m_Length;    ///< 길이


public:
	/// \brief 생성자
	CPacketReader(char* buffer, int len)
	{
		// POD 라면 바로 접근해버린다.
		if (IsPod<T>::Value)
		{
			m_PacketPtr = reinterpret_cast<const T*>(buffer - sizeof(void*));
			m_Length = sizeof(T) - sizeof(void*);
		}
		// 아니면 패킷으로 읽어와야지
		else
		{
			CPacketReadBuffer readProxy(buffer, len);
			m_Length = m_Packet.Read(readProxy);
			m_PacketPtr = &m_Packet;
		}
	}

	/// \brief 소멸자
	~CPacketReader() = default;


public:
	/// \brief 패킷에 접근
	const T* operator-> () { return m_PacketPtr; }

	/// \brief 읽은 길이
	int GetLength() { return m_Length; }
};