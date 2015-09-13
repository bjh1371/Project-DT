////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Platform.h
/// \author 
/// \date 2014.10.24
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// 4127 조건식이 상수입니다.
// 4100 참조되지 않는 매개변수입니다.
#pragma warning( disable : 4100 4127)

#define NOMINMAX

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <math.h>
#include <memory>
#include <random>
#include <regex>
#include <tchar.h>
#include <thread>
#include <iostream>

#include <bitset>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <tbb/concurrent_priority_queue.h>
#include <tbb/concurrent_queue.h>
#include <tbb/scalable_allocator.h>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_vector.h>

#include <sstream>
#include <fstream>

extern  const TCHAR* LOCAL_IP_ADDRESS;

extern const short   LOGIN_SERVER_PORT;
extern const short   ENTRY_SERVER_PORT;
extern const short   GAME_SERVER_PORT;
extern const short   WORLD_SERVER_PORT;

extern const short   ENTRY_SERVER_LINK_PORT;
extern const short   GAME_SERVER_LINK_PORT;
extern const short   WORLD_SERVER_LINK_PORT;
extern const short   QUERY_SERVER_LINK_PORT;

namespace stx
{
	using wstring  = std::basic_string< wchar_t, std::char_traits<wchar_t>, tbb::scalable_allocator<wchar_t> >;

	using string  = std::basic_string< char, std::char_traits<char>, tbb::scalable_allocator<char> >;

	template <typename T>
	using vector = std::vector < T, tbb::scalable_allocator<T> >;

	template <typename T>
	using list = std::list<T, tbb::scalable_allocator<T>>;

	template < class Key, class T, class Compare = less<Key> >
	using map = std::map< Key, T, Compare, tbb::scalable_allocator< std::pair< const Key, T > > >;

	template<class Key, class T, class Hash = std::hash< Key >, class KeyEqual = std::equal_to< Key > >
	using unordered_map = std::unordered_map< Key, T, Hash, KeyEqual, tbb::scalable_allocator< std::pair< const Key, T > > >;

	template < class T, class Compare = less< T > >
	using set = std::set< T, Compare, tbb::scalable_allocator< T > >;

	template <class Key, class Hash = std::hash< Key >, class Pred = std::equal_to< Key > >
	using unordered_set = std::unordered_set< Key, Hash, Pred, tbb::scalable_allocator< Key > >;
}

// 스레드 카운트
#define MAX_THREAD_COUNT 30

#if defined(UNICODE) || defined(_UNICODE)
typedef wchar_t TCHAR;
typedef stx::wstring tstring;
typedef std::wstringstream tstringstream;
typedef std::wregex tregex;
typedef std::wcmatch tcmatch;
typedef std::match_results<std::wstring::const_iterator> tsmatch;
typedef std::wcsub_match tcsub_match;
typedef std::sub_match<std::wstring::const_iterator> tssub_match;
typedef std::wcregex_iterator tcregex_iterator;
typedef std::regex_iterator<std::wstring::const_iterator> tsregex_iterator;
typedef std::wcregex_token_iterator tcregex_token_iterator;
typedef std::regex_token_iterator<std::wstring::const_iterator> tsregex_token_iterator;
typedef std::wfstream tfstream;
typedef std::wofstream tofstream;
typedef std::wifstream tifstream;
#define tcout std::wcout
#else
typedef char TCHAR;
typedef stx::string tstring;
typedef std::stringstream tstringstream;
typedef std::regex tregex;
typedef std::cmatch tcmatch;
typedef std::match_results<std::string::const_iterator> tsmatch;
typedef std::csub_match tcsub_match;
typedef sub_match<std::string::const_iterator> tssub_match;
typedef std::cregex_iterator tcregex_iterator;
typedef std::regex_iterator<std::string::const_iterator> tsregex_iterator;
typedef std::cregex_token_iterator tcregex_token_iterator;
typedef std::regex_token_iterator<std::string::const_iterator> tsregex_token_iterator;
typedef std::fstream tfstream;
typedef std::ofstream tofstream;
typedef std::ifstream tifstream;
#define tcout std::cout
#endif