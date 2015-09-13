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
/// \brief ��Ŷ ���縦 ���̱����ؼ� ���� Ŭ����
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class CPacketWriter
{
private:
	T*              m_PacketPtr; ///< ��Ŷ ������
	T               m_Packet;    ///< ��Ŷ ��ü
	CAsyncTcpEvent* m_Event;     ///< �̺�Ʈ ��ü


public:
	/// \brief ������
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

	/// \brief �Ҹ���
	~CPacketWriter()
	{
		if (m_Event && m_Event->DecreaseRefCount() == 0)
		{
			CAsyncTcpEvent::Dealloc(m_Event);
		}
	}


public:
	/// \brief ��Ŷ�� ����
	T* operator-> () { return m_PacketPtr; }

	/// \brief ����ȯ 
	operator CAsyncTcpEvent* ()
	{
		// m_Event�� nullptr�̶�°��� POD�� �ƴ϶�¶�
		// POD�� �ƴϸ� ��Ŷ������ ���۷� �����Ѵ�.
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
/// \brief ��Ŷ ���縦 ���̱����ؼ� ���� Ŭ����
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class CPacketReader
{
private:
	const T* m_PacketPtr; ///< ��Ŷ ������
	T        m_Packet;    ///< ��Ŷ
	int      m_Length;    ///< ����


public:
	/// \brief ������
	CPacketReader(char* buffer, int len)
	{
		// POD ��� �ٷ� �����ع�����.
		if (IsPod<T>::Value)
		{
			m_PacketPtr = reinterpret_cast<const T*>(buffer - sizeof(void*));
			m_Length = sizeof(T) - sizeof(void*);
		}
		// �ƴϸ� ��Ŷ���� �о�;���
		else
		{
			CPacketReadBuffer readProxy(buffer, len);
			m_Length = m_Packet.Read(readProxy);
			m_PacketPtr = &m_Packet;
		}
	}

	/// \brief �Ҹ���
	~CPacketReader() = default;


public:
	/// \brief ��Ŷ�� ����
	const T* operator-> () { return m_PacketPtr; }

	/// \brief ���� ����
	int GetLength() { return m_Length; }
};