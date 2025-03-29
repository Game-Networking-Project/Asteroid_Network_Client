#pragma once

#include <map>

int parse_scoreboard_buffer(const uint8_t* buffer, size_t buffer_size, Player* players, int maxPlayers);