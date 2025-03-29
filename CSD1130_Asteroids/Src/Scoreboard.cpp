#include "Scoreboard.h"
#include <map>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <iostream>

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

// Packet types/command IDs
#define CMD_SCOREBOARD_UPDATE 0x01

// Player Structure
// Identified by a player id
// With their respective score
struct Player {
	int player_id;
	int score = 0;
};

// List of players
std::map<int, Player> list_of_players;

// Scoreboard packet
typedef struct {
	uint8_t cmd_id;
	uint16_t player_count;
} ScoreboardHeader;

typedef struct {
	int32_t player_id;
	int32_t score;
} ScoreboardPlayerEntry;

int parse_scoreboard_buffer(const uint8_t* buffer, size_t buffer_size, Player* players, int maxPlayers) {
	int offset = 0;

	// Check minimum buffer size
	// 3 = cmd_id + player_count
	if (buffer_size < 3) {
		return -1;
	}

	// Read CMDID
	uint8_t cmd_id = buffer[offset++];
	if (cmd_id != CMD_SCOREBOARD_UPDATE) {
		return -2;
	}

	uint16_t net_playercount;
	memcpy(&net_playercount, buffer + offset, sizeof(uint16_t));
	uint16_t playercount = ntohs(net_playercount);
	offset += sizeof(uint16_t);

	// Validate player count
	if (playercount > maxPlayers) {
		playercount = maxPlayers;
	}

	// Check buffer size with player count
	size_t required_size = 1 + 2 + (playercount * (4 + 4));
	if (buffer_size < required_size) {
		return -3;
	}

	for (int i = 0; i < playercount; i++) {
		// Read player ID
		int32_t net_playerID;
		memcpy(&net_playerID, buffer + offset, sizeof(int32_t));
		players[i].player_id = ntohl(net_playerID);
		offset += sizeof(int32_t);

		// Read player score
		int32_t net_score;
		memcpy(&net_score, buffer + offset, sizeof(int32_t));
		players[i].score = ntohl(net_score);
		offset += sizeof(int32_t);
	}

	return playercount;
}