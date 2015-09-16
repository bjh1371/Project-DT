////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file SharedDef.h
/// \author bjh1371
/// \date 2014.5.31
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 로그인 타입
enum LoginType : BYTE
{
	LOGINTYPE_TEMP,
	LOGINTYPE_SELF,
	LOGINTYPE_TEST,
};

const int MAX_ACCOUNT_NAME_LENGTH = 100;

const int MAX_PASSWORD_LENGTH = 100;

const int MAX_WORLD_NAME_LENGTH = 100;


typedef unsigned int ClientVersion_t;
const ClientVersion_t INVALID_CLIENT_VERSION = 0;

typedef unsigned int ObjectId_t;
const ObjectId_t INVALID_OBJECT_ID = 0;

typedef unsigned int ZoneId_t;
const ZoneId_t INVALID_ZONE_ID = 0;

typedef unsigned int WorldId_t;
const WorldId_t INVALID_WORLD_ID = 0;

typedef int Level_t;
const Level_t INVALID_LEVEL = 0;
