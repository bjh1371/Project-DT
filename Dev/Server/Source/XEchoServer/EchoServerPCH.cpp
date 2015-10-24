////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file EchoServerPCH.cpp
/// \author bjh1371
/// \date 2015/08/03
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "EchoServerPCH.h"

#pragma comment(lib, "Base")
#pragma comment(lib, "Server")
#pragma comment(lib, "PacketProc")

#ifdef _DEBUG
#pragma comment(lib, "tbb_debug")
#pragma comment(lib, "libyaml-cppmdd")
#pragma comment(lib, "libcouchbase_d")
#else
#pragma comment(lib, "tbb")
#pragma comment(lib, "libyaml-cppmd")
#pragma comment(lib, "libcouchbase")
#endif

