#pragma once
#include "Main.h"


// ships data
struct ShipsState
{
	AEVec2 Position;
	AEVec2 Velocity;
	f32 CurrentDirection; // for rotation
	f32 lastFiredTime; // time stamp on when 
	uint8_t isActive;
};

// bullets data
struct BulletsState
{
	AEVec2 Position;
	AEVec2 Velocity;
	f32 CurrentDirection; // for rotation
	f32 spawnTime; // Timestamp of when the bullet was fired
};

// asteroid data
struct AsteroidData
{
	AEVec2 Position;
	AEVec2 Scale;
	AEVec2 Velocity;
};

// which player am I, and my state
struct ShipStateUpdateMessage 
{
	uint8_t playerID;  // Unique identifier for the player
	ShipsState state; // Current state of the ship
	float lastUpdatedTime;
};

// who and when client fired the bullet
struct FireEventMessage
{
	uint8_t playerID;  // Player who fired
	uint32_t timestamp; // Timestamp of when the firing occurred
};

// i am the bullet of who, and whats my state
struct BulletStateUpdateMessage 
{
	uint8_t playerID;        
	BulletsState bulletState;  // Bullet state (position, velocity, etc.)
	float lastUpdatedTime;
};

void clientJoinGame(SOCKET& clientSocket);
void clientResponseMessage(SOCKET& clientSocket);
void clientReceieveData(SOCKET& clientSocket);
// void clientSendMessage(SOCKET& clientSocket);

extern ShipStateUpdateMessage Myself;
extern std::vector <BulletStateUpdateMessage> myBullet;
extern std::map<int, ShipStateUpdateMessage> PlayerList;
extern std::map<int, BulletStateUpdateMessage> BulletList;
extern std::vector <AsteroidData> AsteroidList;