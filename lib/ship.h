#ifndef _SHIP_H_
#define _SHIP_H_


#define AIRCRAFT_CARRIER 0
#define BATTLESHIP 1
#define SUBMARINE 2
#define PATROL_BOAT 3

#define AIRCRAFT_CARRIER_SIZE 4
#define BATTLESHIP_SIZE 3
#define SUBMARINE_SIZE 2
#define PATROL_BOAT_SIZE 1

#define AIRCRAFT_CARRIER_COUNT 1
#define BATTLESHIP_COUNT 2
#define SUBMARINE_COUNT 3
#define PATROL_BOAT_COUNT 4

#define VERTICAL 1
#define HORIZONTAL 2


int attack_ship(Map *m, int x, int y);
int insert_ship(Map *m, int ship, int x, int y, int orientation);
int check_used_ships(Map *m);

#endif