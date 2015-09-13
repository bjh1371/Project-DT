////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file PacketBase.h
/// \author bjh1371
/// \date 2014.5.24
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <tchar.h>

#include <type_traits>

template <typename T>
struct IsPod { enum { Value = std::is_pod<T>::value }; };

template <typename T>
struct MaxBytes { enum { Value = sizeof(T) }; };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CSizeCounter
/// \brief 패킷 사이즈를 계산하기 위한 클래스
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSizeCounter
{
public:
	template <typename T>
	static int Bytes(const T& value)
	{
		SizeImpl< IsPod<T>::Value > impl;
		return impl(value);
	}


private:
	template <bool IsPod>
	struct SizeImpl {};

	template <>
	struct SizeImpl < true >
	{
		template <typename T>
		int operator() (const T& value) const
		{
			return sizeof(T);
		}
	};

	template <>
	struct SizeImpl < false >
	{
		template <typename T>
		int operator() (const T& value) const
		{
			return value.Bytes();
		}
	};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CPacketRecvBuffer
/// \brief 패킷 리시브 버퍼
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CPacketReadBuffer
{
private:
	char* m_Buffer;    ///< 버퍼
	int   m_Offset;    ///< 현재 위치
	int   m_MaxLength; ///< 최대 길이


public:
	CPacketReadBuffer(char* buffer, int len)
	:
	m_Buffer(buffer),
	m_MaxLength(len),
	m_Offset(0)
	{

	}


public:
	/// \brief Read 함수
	template <typename T>
	int Read(T& value)
	{
		ReadImpl< IsPod<T>::Value > impl;
		return impl(*this, value);
	}

	/// \brief Pod를 통째로 읽는 Read함수
	template <typename T>
	int ReadArray(T* value, int length)
	{
		int read = 0;
		if (m_Offset + length <= m_MaxLength)
		{
			memcpy(value, m_Buffer + m_Offset, length);
			m_Offset += length;
			read = length;
		}
		return read;
	}


private:
	/// \brief Read 구현부분
	template <bool IsPod>
	struct ReadImpl {};

	/// \brief POD라면
	template <>
	struct ReadImpl < true >
	{
		template <typename T>
		int operator() (CPacketReadBuffer& buffer, T& value)
		{
			return buffer.ReadArray(&value, sizeof(value));
		}
	};

	/// \brief POD가 아니라면
	template <>
	struct ReadImpl < false >
	{
		template <typename T>
		int operator() (CPacketReadBuffer& buffer, T& value)
		{
			return value.Read(buffer);
		}
	};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CPacketWriteBuffer
/// \brief 패킷 라이트 버퍼
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CPacketWriteBuffer
{
private:
	char* m_Buffer;    ///< 버퍼
	int   m_Offset;    ///< 현재 위치
	int   m_MaxLength; ///< 최대 길이


public:
	CPacketWriteBuffer(char* buffer, int len)
	:
	m_Buffer(buffer),
	m_MaxLength(len),
	m_Offset(0)
	{

	}


public:
	/// \brief Write 함수
	template <typename T>
	int Write(const T& value)
	{
		WriteImpl< IsPod<T>::Value > impl;
		return impl(*this, value);

	}

	/// \brief Pod를 통째로 쓰는 Write함수
	template <typename T>
	int WriteArray(const T* value, int length)
	{
		int write = 0;
		if (m_Offset + length <= m_MaxLength)
		{
			memcpy(m_Buffer + m_Offset, value, length);
			m_Offset += length;
			write = length;
		}
		return write;
	}


private:
	/// \brief Write 구현부분
	template <bool IsPod>
	struct WriteImpl {};

	/// \brief POD라면
	template <>
	struct WriteImpl < true >
	{
		template <typename T>
		int operator() (CPacketWriteBuffer& buffer, const T& value)
		{
			return buffer.WriteArray(&value, sizeof(value));
		}
	};

	/// \brief POD가 아니라면
	template <>
	struct WriteImpl < false >
	{
		template <typename T>
		int operator() (CPacketWriteBuffer& buffer, const T& value)
		{
			return value.Write(buffer);
		}
	};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CString
/// \brief 패킷에 사용할 스트링 클래스
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(push, 1)

template <int MaxLength>
class CString
{
private:
	TCHAR m_String[MaxLength + 1];


public:
	/// \brief 생성자
	CString()
	{
		m_String[0] = 0;
	}

	/// \brief 소멸자
	~CString()
	{

	}
	/// \brief 바이트를 반환
	int Bytes() const
	{
		int size = sizeof(short) + static_cast<int>((_tcsnlen(m_String, MaxLength + 1)) * sizeof(TCHAR));
		return size;
	}

	/// \brief 문자열을 복사
	void operator= (const TCHAR* string)
	{
		_tcsncpy_s(m_String, MaxLength + 1, string, _TRUNCATE);
	}

	// 형변환 연산자
	operator const TCHAR* () const
	{
		return m_String;
	}


public:
	/// \brief 버퍼에서 읽어 온다.
	int Read(CPacketReadBuffer& buffer)
	{
		int read = 0;
		short length = 0;
		read += buffer.Read(length);
		read += buffer.ReadArray(m_String, length * sizeof(TCHAR));
		m_String[length] = 0;

		return read;
	}

	/// \brief 버퍼에 쓴다.
	int Write(CPacketWriteBuffer& buffer) const
	{
		int written = 0;
		short length = static_cast<short>(_tcsnlen(m_String, MaxLength + 1));
		written += buffer.Write(length);
		written += buffer.WriteArray(m_String, length * sizeof(TCHAR));

		return written;
	}
};

#pragma pack(pop)

template <int MaxLength>
struct IsPod < CString<MaxLength> > { enum { Value = false }; };

template <int MaxLength>
struct MaxBytes< CString<MaxLength> > {enum { Value = sizeof(TCHAR) * (MaxLength + 1) }; };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CList
/// \brief 패킷에 사용할 리스트 클래스
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(push, 1)

template <typename T, int MaxLength>
class CList
{
private:
	short m_Length;
	T     m_List[MaxLength];


public:
	/// \brief 생성자
	CList()
	:
	m_Length(0)
	{

	}

	/// \brief 소멸자
	~CList()
	{

	}


public:
	/// \brief 리스트에 추가한다.
	T& Add()
	{
		assert(m_Length + 1 <= MaxLength && "list overflow");

		T& data = m_List[m_Length++];
		return data;

	}

	/// \brief 최대 길이를 반환한다.
	short GetLength() const { return m_Length; }

	/// \brief 바이트를 반환한다.
	int Bytes() const
	{
		ByteImpl<IsPod<T>::Value> impl;
		return impl(m_List, m_Length);
	}

	/// \brief 버퍼에서 읽어온다.
	int Read(CPacketReadBuffer& buffer)
	{
		ReadImpl< IsPod<T>::Value > impl;
		return impl(buffer, m_List, m_Length);
	}

	/// \brief 버퍼에 쓴다.
	int Write(CPacketWriteBuffer& buffer) const
	{
		WriteImpl< IsPod<T>::Value > impl;
		return impl(buffer, m_List, m_Length);
	}

	/// \brief 인덱스에 있는 값을 반환한다.
	const T& At(int idx) const { return m_List[idx]; }


private:
	/// \brief 사이즈 구현부분
	template <bool IsPod>
	struct ByteImpl
	{};

	/// \brief POD라면
	template <>
	struct ByteImpl < true >
	{
		template <typename T>
		int operator()(const T* list, const short length) const
		{
			return length * sizeof(T);
		}
	};

	/// \brief POD가 아니라면
	template <>
	struct ByteImpl < false >
	{
		template <typename T>
		int operator()(const T* list, const short length) const
		{
			int size = 0;
			for (int i = 0; i < length; ++i)
			{
				size += list[i].Bytes();
			}
			return size;
		}
	};


private:
	/// \brief Read 구현부분
	template <bool IsPod>
	struct ReadImpl {};

	/// \brief POD라면
	template <>
	struct ReadImpl < true >
	{
		template <typename T>
		int operator()(CPacketReadBuffer& buffer, T* list, short& length)
		{
			int size = 0;
			size += buffer.Read(length);
			size += buffer.ReadArray(list[0], length * sizeof(T));
			return size;
		}
	};

	/// \brief POD가 아니라면
	template <>
	struct ReadImpl < false >
	{
		template <typename T>
		int operator()(CPacketReadBuffer& buffer, T* list, short& length)
		{
			int size = 0;
			size += buffer.Read(length);
			for (int i = 0; i < length; ++i)
			{
				size += list[i].Read(buffer);
			}
			return size;
		}
	};


private:
	/// \brief Write 구현부분
	template <bool IsPod>
	struct WriteImpl {};

	/// \brief POD라면
	template <>
	struct WriteImpl < true >
	{
		template <typename T>
		int operator() (CPacketWriteBuffer& buffer, const T* list, const short length) const
		{
			int size = 0;
			size += buffer.Write(length);
			size += buffer.WriteArray(list[0], length * sizeof(T));
			return size;
		}
	};

	/// \brief POD가 아니라면
	template <>
	struct WriteImpl < false >
	{
		template <typename T>
		int operator() (CPacketWriteBuffer& buffer, const T* list, const short length) const
		{
			int size = 0;
			size += buffer.Write(length);
			for (int i = 0; i < length; ++i)
			{
				size += list[i].Write(buffer);
			}
			return size;
		}
	};
};

#pragma pack(pop)

template <typename T, int MaxLength>
struct IsPod < CList<T, MaxLength> > { enum { Value = false }; };

template <typename T, int MaxLength>
struct MaxBytes < CList<T, MaxLength> > { enum { Value = MaxBytes<T>::Value * MaxLength }; };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CArray
/// \brief 패킷에서 사용할 배열
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(push, 1)

template <typename T, int MaxLength>
class CArray
{
private:
	T m_Array[MaxLength];


public:
	/// \brief 객체 반환
	const T& operator[] (int index) const
	{
		return m_Array[index];
	}

	/// \brief 객체 반환
	T& operator[] (int index)
	{
		return m_Array[index];
	}


public:
	/// \brief 사이즈 반환
	int Bytes() const
	{
		CByteImpl<IsPod<T>::Value> impl;
		return impl(m_Array, MaxLength);
	}

	/// \brief 버퍼에서 읽어온다.
	int Read(CPacketReadBuffer& buffer)
	{
		CReadImpl<IsPod<T>::Value> impl;
		return impl(buffer, m_Array, MaxLength);
	}

	/// \brief 버퍼에 쓴다.
	int Write(CPacketWriteBuffer& buffer) const
	{
		CWriteImpl<IsPod<T>::Value> impl;
		return impl(buffer, m_Array, MaxLength);
	}


private:
	/// \brief Bytes 구현부분
	template <bool IsPod>
	struct CByteImpl
	{};

	/// \brief POD라면
	template <>
	struct CByteImpl < true >
	{
		template <typename T>
		int operator()(const T* array, const int length) const
		{
			return sizeof(T) * length;
		}
	};

	/// \brief POD가 아니라면
	template<>
	struct CByteImpl < false >
	{
		template <typename T>
		int operator()(const T* array, const int length) const
		{
			int size = 0;
			for (int i = 0; i < length; ++i)
			{
				size += array[i].Bytes();
			}
			return size;
		}
	};


private:
	/// \brief Read 구현부분
	template <bool IsPod>
	struct CReadImpl
	{};

	/// \brief POD 인경우
	template <>
	struct CReadImpl < true >
	{
		template <typename T>
		int operator() (CPacketReadBuffer& buffer, T* array, const int length)
		{
			int read = 0;
			read += buffer.ReadArray(array[0], sizeof(T) * length);
			return read;
		}
	};

	/// \brief POD가 아닌경우
	template <>
	struct CReadImpl < false >
	{
		template <typename T>
		int operator() (CPacketReadBuffer& buffer, T* array, const int length)
		{
			int read = 0;
			for (int i = 0; i < length; ++i)
			{
				read += array[i].Read(buffer);
			}
			return read;
		}
	};


private:
	/// \brief Write 구현부분
	template <bool IsPod>
	struct CWriteImpl
	{};

	/// \brief POD인 경우
	template <>
	struct CWriteImpl < true >
	{
		int operator() (CPacketWriteBuffer& buffer, const T* array, const int length) const
		{
			int write = 0;
			write += buffer.WriteArray(array[0], sizeof(T) * length);
			return write;
		}
	};

	/// \brief POD가 아닌 경우
	template <>
	struct CWriteImpl < false >
	{
		int operator() (CPacketWriteBuffer& buffer, const T* array, const int length) const
		{
			int write = 0;
			for (int i = 0; i < length; ++i)
			{
				write += buffer.Write(array[i]);
			}
			return write;
		}
	};

};

#pragma pack (pop)

template <typename T, int MaxLength>
struct IsPod < CArray<T, MaxLength> > { enum { Value = IsPod<T>::Value }; };

template <typename T, int MaxLength>
struct MaxBytes < CArray<T, MaxLength> > { enum { Value = MaxBytes<T>::Value * MaxLength }; };

/// 패킷 그룹
enum PacketGroup
{
	PACKET_GROUP_SHARED, ///< 클라 <-> 서버
	PACKET_GROUP_SERVER, ///< 서버 <-> 서버
	PACKET_GROUP_MAX,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CPacket
/// \brief 패킷
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef unsigned short PacketSize_t;
typedef unsigned short PacketId_t;
typedef unsigned short Checksum_t;

#pragma pack (push, 1)

class CPacket
{
public:
	enum
	{
		// 헤더 길이
		PACKET_HEADER_LENGTH = sizeof(PacketSize_t) + sizeof(PacketId_t),

		// 풀 헤더 길이
		PACKET_HEADER_FULL_LENGTH = sizeof(PacketSize_t) + sizeof(PacketId_t) + sizeof(Checksum_t),
	};


public:
	Checksum_t m_Checksum; ///< 체크섬


public:
	/// \brief 생성자
	CPacket()
	:
	m_Checksum(0)
	{}
	/// \brief 소멸자
	virtual ~CPacket() {}


public:
	/// \brief 패킷 ID 반환한다.
	virtual PacketId_t GetId() const = 0;

	/// \brief 패킷 본문을 읽는다.
	virtual int ReadPayload(CPacketReadBuffer& buffer) = 0;

	/// \brief 패킷 본문에 쓴다.
	virtual int WritePayload(CPacketWriteBuffer& buffer) const = 0;

	/// \brief 패킷 본문의 사이즈를 반환한다.
	virtual int BytesPayload() const = 0;

	/// \brief 패킷을 읽는다.
	inline int Read(CPacketReadBuffer& buffer)
	{
		int read = 0;
		read += buffer.Read(m_Checksum);
		read += ReadPayload(buffer);

		return read;
	}

	/// \brief 버퍼에 쓴다.
	inline int Write(CPacketWriteBuffer& buffer)
	{
		int write = 0;
		write += buffer.Write(GetId());
		write += buffer.Write(static_cast<PacketSize_t>(PACKET_HEADER_FULL_LENGTH + BytesPayload()));
		write += buffer.Write(m_Checksum);
		write += WritePayload(buffer);

		return write;
	}


public:
	// \brief POD 형식의 읽기/쓰기를 위한 주소 반환
	inline char* GetPodPayloadAddr() { return reinterpret_cast<char*>(this) + sizeof(void*) + sizeof(m_Checksum); }
	inline const char* GetPodPayloadAddr() const { return reinterpret_cast<const char*>(this) + sizeof(void*) + sizeof(m_Checksum); }

	/// \brief POD 형식의 읽기/쓰기를 위한 길이 반환
	inline int GetPodPayloadLength(size_t sz) { return static_cast<int>(sz - sizeof(void*) - sizeof(m_Checksum)); }
	inline int GetPodPayloadLength(size_t sz) const { return static_cast<int>(sz - sizeof(void*) - sizeof(m_Checksum)); }


public:
	/// \brief 패킷 ID 주소를 반환한다.
	static inline PacketId_t* GetPacketIdPtr(char* buffer) { return reinterpret_cast<PacketId_t*>(buffer); }

	/// \brief 패킷 사이즈 주소를 반환한다.
	static inline PacketSize_t* GetPacketSizePtr(char* buffer) { return reinterpret_cast<PacketSize_t*>(buffer + sizeof(PacketId_t)); }

	/// \brief 패킷 체크섬 주소를 반환한다
	static inline PacketSize_t* GetPacketChecksumPtr(char* buffer) { return reinterpret_cast<Checksum_t*>(buffer + sizeof(PacketId_t) + sizeof(PacketSize_t)); }
};

#pragma pack (pop)