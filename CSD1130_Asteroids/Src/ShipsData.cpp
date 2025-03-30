#include "Main.h"

#define MAX_STR_LEN         5000
// Link against the Winsock library
#pragma comment(lib, "Ws2_32.lib")

// Define the external variables
ShipStateUpdateMessage Myself;
std::vector<BulletStateUpdateMessage> myBullet;
std::map<int, ShipStateUpdateMessage> PlayerList;
std::map<int, BulletStateUpdateMessage> BulletList;
std::vector <AsteroidData> AsteroidList;

void clientJoinGame(SOCKET& clientSocket) // client request to join game
{
	while (true)
	{
		char SendmsgBuffer[MAX_STR_LEN];
		SendmsgBuffer[0] = REQ_JOINGAMESESSION; //req join
		int bytes_received = 0;
		bytes_received = recv(clientSocket, SendmsgBuffer, MAX_STR_LEN, 0);
		if (bytes_received <= 0)
		{
			int error = WSAGetLastError();
			if (error == WSAEWOULDBLOCK) // this is to check if it is because of closing then receive nothing or not
			{
				continue;
			}
			std::cout << "disconnecting" << std::endl; // server closed
		}
		else
		{
			if (SendmsgBuffer[0] == RSP_JOINGAMESESSION)
			{
				if (SendmsgBuffer[1] == false)
				{
					break;
				}
				else
				{
					uint8_t PlayerID;
					memcpy(&PlayerID, &SendmsgBuffer[2], sizeof(uint8_t));
					Myself.playerID = PlayerID;
				}
			}
		}
	}
}

// when server need data from client
void clientResponseMessage(SOCKET& clientSocket)
{
	while (true)
	{
		char receiveBuffer[MAX_STR_LEN]; // check what client receive
		int bytes_received = 0;
		bytes_received = recv(clientSocket, receiveBuffer, MAX_STR_LEN, 0);
		if (bytes_received <= 0)
		{
			int error = WSAGetLastError();
			if (error == WSAEWOULDBLOCK) // this is to check if it is because of closing then receive nothing or not
			{
				continue;
			}
			std::cout << "disconnecting" << std::endl; // server closed
		}
		else
		{
			if (receiveBuffer[0] == REQ_PLAYERDATA) // server want player data
			{
				ShipStateUpdateMessage playerdata;
				receiveBuffer[0] = RSP_PLAYERDATA;
				playerdata = Myself;
				// convert floats to uint32_t in network byte order
				uint32_t netPosX = htonf(playerdata.state.Position.x);
				uint32_t netPosY = htonf(playerdata.state.Position.y);
				uint32_t netVelX = htonf(playerdata.state.Velocity.x);
				uint32_t netVelY = htonf(playerdata.state.Velocity.y);
				uint32_t netDir = htonf(playerdata.state.CurrentDirection);
				uint32_t netLastFired = htonf(playerdata.state.lastFiredTime);
				// copy data to buffer
				memcpy(&receiveBuffer[1], &playerdata.playerID, sizeof(uint8_t));
				memcpy(&receiveBuffer[2], &netPosX, sizeof(uint32_t));
				memcpy(&receiveBuffer[6], &netPosY, sizeof(uint32_t));
				memcpy(&receiveBuffer[10], &netVelX, sizeof(uint32_t));
				memcpy(&receiveBuffer[14], &netVelY, sizeof(uint32_t));
				memcpy(&receiveBuffer[18], &netDir, sizeof(uint32_t));
				memcpy(&receiveBuffer[22], &netLastFired, sizeof(uint32_t));
				memcpy(&receiveBuffer[26], &playerdata.state.isActive, sizeof(uint8_t));

				send(clientSocket, receiveBuffer, MAX_STR_LEN, 0);
			}
			else if (receiveBuffer[0] == REQ_PLAYERBULLETDATA)
			{
				std::vector<BulletStateUpdateMessage> bulletData;
				receiveBuffer[0] = RSP_PLAYERBULLETDATA;
				bulletData = myBullet;
				int counterforLoop = 1;
				for (const BulletStateUpdateMessage& eachbullet : bulletData)
				{
					BulletStateUpdateMessage eachBulletData = eachbullet;
					// convert float data values to network byte order
					uint32_t netPosX = htonf(eachBulletData.bulletState.Position.x);
					uint32_t netPosY = htonf(eachBulletData.bulletState.Position.y);
					uint32_t netVelX = htonf(eachBulletData.bulletState.Velocity.x);
					uint32_t netVelY = htonf(eachBulletData.bulletState.Velocity.y);
					uint32_t netDir = htonf(eachBulletData.bulletState.CurrentDirection);
					uint32_t netSpawnTime = htonf(eachBulletData.bulletState.spawnTime);

					// Prepare all the bullet data
					memcpy(&receiveBuffer[counterforLoop], &eachBulletData.playerID, sizeof(uint8_t));
					counterforLoop += 1; 
					memcpy(&receiveBuffer[counterforLoop], &netPosX, sizeof(uint32_t));
					counterforLoop += 4;
					memcpy(&receiveBuffer[counterforLoop], &netPosY, sizeof(uint32_t));
					counterforLoop += 4;
					memcpy(&receiveBuffer[counterforLoop], &netVelX, sizeof(uint32_t));
					counterforLoop += 4;
					memcpy(&receiveBuffer[counterforLoop], &netVelY, sizeof(uint32_t));
					counterforLoop += 4;
					memcpy(&receiveBuffer[counterforLoop], &netDir, sizeof(uint32_t));
					counterforLoop += 4;
					memcpy(&receiveBuffer[counterforLoop], &netSpawnTime, sizeof(uint32_t));
					counterforLoop += 4;
				}
				send(clientSocket, receiveBuffer, MAX_STR_LEN, 0);
			}
		}
	}
	
}

// client receive data
void clientReceieveData(SOCKET& clientSocket)
{
	while (true)
	{
		char receiveBuffer[MAX_STR_LEN]; // check what client receive
		int bytes_received = 0;
		bytes_received = recv(clientSocket, receiveBuffer, MAX_STR_LEN, 0);
		if (bytes_received <= 0)
		{
			int error = WSAGetLastError();
			if (error == WSAEWOULDBLOCK) // this is to check if it is because of closing then receive nothing or not
			{
				continue;
			}
			std::cout << "disconnecting" << std::endl; // server closed
		}
		else
		{
			if (receiveBuffer[0] == RSP_SHIPSDATA)
			{
				uint32_t numofPlayer;
				memcpy(&numofPlayer, &receiveBuffer[1], sizeof(uint32_t));
				numofPlayer = ntohl(numofPlayer);
				int initialoffset = 4;
				int eachPlayerOffset = 26;
				for (unsigned int i = 0; i < numofPlayer; i++)
				{
					int counterforLoop = 1;
					uint8_t playerID;
					uint32_t netPosX, netPosY, netVelX, netVelY, netDir, netLastFired;
					memcpy(&playerID, &receiveBuffer[initialoffset + ((i * eachPlayerOffset) + counterforLoop)], sizeof(uint8_t));
					counterforLoop += 1;
					f32 positionX;
					memcpy(&netPosX, &receiveBuffer[initialoffset + ((i * eachPlayerOffset) + counterforLoop)], sizeof(uint32_t));
					positionX = ntohf(netPosX);
					counterforLoop += 4;
					f32 positionY;
					memcpy(&netPosY, &receiveBuffer[initialoffset + ((i * eachPlayerOffset) + counterforLoop)], sizeof(uint32_t));
					positionY = ntohf(netPosY);
					counterforLoop += 4;
					f32 velocityX;
					memcpy(&netVelX, &receiveBuffer[initialoffset + ((i * eachPlayerOffset) + counterforLoop)], sizeof(uint32_t));
					velocityX = ntohf(netVelX);
					counterforLoop += 4;
					f32 velocityY;
					memcpy(&netVelY, &receiveBuffer[initialoffset + ((i * eachPlayerOffset) + counterforLoop)], sizeof(uint32_t));
					velocityY = ntohf(netVelY);
					counterforLoop += 4;
					f32 direction;
					memcpy(&netDir, &receiveBuffer[initialoffset + ((i * eachPlayerOffset) + counterforLoop)], sizeof(uint32_t));
					direction = ntohf(netDir);
					counterforLoop += 4;
					f32 lastFiretime;
					memcpy(&netLastFired, &receiveBuffer[initialoffset + ((i * eachPlayerOffset) + counterforLoop)], sizeof(uint32_t));
					lastFiretime = ntohf(netLastFired);
					counterforLoop += 4;
					uint8_t isActive;
					memcpy(&isActive, &receiveBuffer[initialoffset + ((i * eachPlayerOffset) + counterforLoop)], sizeof(uint8_t));

					if (playerID == Myself.playerID)
					{
						continue;
					}
					else
					{
						ShipStateUpdateMessage otherplayerData;
						otherplayerData.playerID = playerID;
						otherplayerData.state.Position = { positionX, positionY };
						otherplayerData.state.Velocity = { velocityX, velocityY };
						otherplayerData.state.CurrentDirection = direction;
						otherplayerData.state.lastFiredTime = lastFiretime;
						otherplayerData.state.isActive = isActive;
						static f32 currentTime = 0.0f;
						currentTime = (f32)AEGetTime(nullptr);
						otherplayerData.lastUpdatedTime = currentTime;
						PlayerList[playerID] = otherplayerData;
					}
				}
			}
			else if (receiveBuffer[0] == RSP_BULLETSDATA)
			{
				uint32_t numofPlayer;
				memcpy(&numofPlayer, &receiveBuffer[1], sizeof(uint32_t));
				numofPlayer = ntohl(numofPlayer);
				int initialoffset = 4;
				int eachPlayerOffset = 25;
				for (unsigned int i = 0; i < numofPlayer; i++)
				{
					int counterforLoop = 1;
					uint8_t playerID;
					uint32_t netPosX, netPosY, netVelX, netVelY, netDir, netSpawnTime;
					memcpy(&playerID, &receiveBuffer[initialoffset + ((i * eachPlayerOffset) + counterforLoop)], sizeof(uint8_t));
					counterforLoop += 1;
					f32 positionX;
					memcpy(&netPosX, &receiveBuffer[initialoffset + ((i * eachPlayerOffset) + counterforLoop)], sizeof(uint32_t));
					positionX = ntohf(netPosX);
					counterforLoop += 4;
					f32 positionY;
					memcpy(&netPosY, &receiveBuffer[initialoffset + ((i * eachPlayerOffset) + counterforLoop)], sizeof(uint32_t));
					positionY = ntohf(netPosY);
					counterforLoop += 4;
					f32 velocityX;
					memcpy(&netVelX, &receiveBuffer[initialoffset + ((i * eachPlayerOffset) + counterforLoop)], sizeof(uint32_t));
					velocityX = ntohf(netVelX);
					counterforLoop += 4;
					f32 velocityY;
					memcpy(&netVelY, &receiveBuffer[initialoffset + ((i * eachPlayerOffset) + counterforLoop)], sizeof(uint32_t));
					velocityY = ntohf(netVelY);
					counterforLoop += 4;
					f32 direction;
					memcpy(&netDir, &receiveBuffer[initialoffset + ((i * eachPlayerOffset) + counterforLoop)], sizeof(uint32_t));
					direction = ntohf(netDir);
					counterforLoop += 4;
					f32 spawnTime;
					memcpy(&netSpawnTime, &receiveBuffer[initialoffset + ((i * eachPlayerOffset) + counterforLoop)], sizeof(uint32_t));
					spawnTime = ntohf(netSpawnTime);

					if (playerID == Myself.playerID)
					{
						continue;
					}
					else
					{
						BulletStateUpdateMessage otherplayerbullet;
						otherplayerbullet.playerID = playerID;
						otherplayerbullet.bulletState.Position = { positionX, positionY };
						otherplayerbullet.bulletState.Velocity = { velocityX, velocityY };
						otherplayerbullet.bulletState.CurrentDirection = direction;
						otherplayerbullet.bulletState.spawnTime = spawnTime;
						static f32 currentTime = 0.0f;
						currentTime = (f32)AEGetTime(nullptr);
						otherplayerbullet.lastUpdatedTime = currentTime;
						BulletList[playerID] = otherplayerbullet;
					}
				}
			}
			else if (receiveBuffer[0] == RSP_ASTEROIDSDATA)
			{
				uint32_t numofAsteroid;
				memcpy(&numofAsteroid, &receiveBuffer[1], sizeof(uint32_t));
				numofAsteroid = ntohl(numofAsteroid);
				int initialoffset = 4;
				int eachAsteroidOffset = 24;
				for (unsigned int i = 0; i < numofAsteroid; i++)
				{
					int counterforLoop = 1;
					uint32_t netPosX, netPosY, netScaleX, netScaleY, netVelX, netVelY;
					f32 positionX;
					memcpy(&netPosX, &receiveBuffer[initialoffset + ((i * eachAsteroidOffset) + counterforLoop)], sizeof(uint32_t));
					positionX = ntohf(netPosX);
					counterforLoop += 4;
					f32 positionY;
					memcpy(&netPosY, &receiveBuffer[initialoffset + ((i * eachAsteroidOffset) + counterforLoop)], sizeof(uint32_t));
					positionY = ntohf(netPosY);
					counterforLoop += 4;
					f32 scaleX;
					memcpy(&netScaleX, &receiveBuffer[initialoffset + ((i * eachAsteroidOffset) + counterforLoop)], sizeof(uint32_t));
					scaleX = ntohf(netScaleX);
					counterforLoop += 4;
					f32 scaleY;
					memcpy(&netScaleY, &receiveBuffer[initialoffset + ((i * eachAsteroidOffset) + counterforLoop)], sizeof(uint32_t));
					scaleY = ntohf(netScaleY);
					f32 velocityX;
					memcpy(&netVelX, &receiveBuffer[initialoffset + ((i * eachAsteroidOffset) + counterforLoop)], sizeof(uint32_t));
					velocityX = ntohf(netVelX);
					counterforLoop += 4;
					f32 velocityY;
					memcpy(&netVelY, &receiveBuffer[initialoffset + ((i * eachAsteroidOffset) + counterforLoop)], sizeof(uint32_t));
					velocityY = ntohf(netVelY);
					AsteroidData asteroidData;
					asteroidData.Position = { positionX, positionY };
					asteroidData.Scale = { scaleX, scaleY };
					asteroidData.Velocity = { velocityX, velocityY };
					AsteroidList.push_back(asteroidData);
				}
			}
		}
	}
}

//// client request from server
//void clientSendMessage()
//{
//	char SendmsgBuffer[MAX_STR_LEN];
//	while (true)
//	{
//		SendmsgBuffer[0] = REQ_SHIPSDATA;
//		send(, SendmsgBuffer, MAX_STR_LEN, 0);
//		SendmsgBuffer[0] = REQ_BULLETSDATA;
//		send(, SendmsgBuffer, MAX_STR_LEN, 0);
//		SendmsgBuffer[0] = REQ_SCORE;
//		send(, SendmsgBuffer, MAX_STR_LEN, 0);
//		SendmsgBuffer[0] = REQ_ASTEROIDSDATA;
//		send(, SendmsgBuffer, MAX_STR_LEN, 0);
//	}
//}
