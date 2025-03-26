#include "Main.h"

#define MAX_STR_LEN         1000
enum CMDID
{
	REQ_JOINGAMESESSION = (unsigned char) 0x0,
	RSP_JOINGAMESESSION = (unsigned char) 0x1,
	// for server to request data from individual client
	REQ_PLAYERDATA = (unsigned char) 0x2,
	RSP_PLAYERDATA = (unsigned char) 0x3,
	REQ_PLAYERBULLETDATA = (unsigned char) 0x4,
	RSP_PLAYERBULLETDATA = (unsigned char) 0x5,
	// for client to request all data 
	REQ_SHIPSDATA = (unsigned char) 0x6,
	RSP_SHIPSDATA = (unsigned char) 0x7,
	REQ_BULLETSDATA = (unsigned char) 0x8,
	RSP_BULLETSDATA = (unsigned char) 0x9,
	// for client to request score
	REQ_SCORE = (unsigned char) 0x10,
	RSP_SCORE = (unsigned char) 0x11,
	// for client to request asteroid data
	REQ_ASTEROIDSDATA = (unsigned char) 0x12,
	RSP_ASTEROIDSDATA = (unsigned char) 0x13,
	// other commands
	REQ_LISTUSER = (unsigned char) 0x14,
	RSP_LISTUSER = (unsigned char) 0x15,
	REQ_QUIT = (unsigned char) 0x16
};

void clientProcessMessage()
{
	char buffer[MAX_STR_LEN];
}