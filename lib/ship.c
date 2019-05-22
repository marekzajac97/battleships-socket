#include <stdio.h>

#include "map.h"
#include "util.h"
#include "ship.h"


int attack_ship(Map *m, int x, int y)
{
    if (x > m->width || x < 0 || y > m->width || y < 0)
        return -1;

    if (m->map[y][x] == SHIP || m->map[y][x] == DESTROYED)
    {
        m->map[y][x] = DESTROYED;
        return 1;
    }
    else {
        m->map[y][x] = MISSED;
        return 0;
    }
}

int insert_ship(Map *m, int ship, int x, int y, int orientation)
{
    int i, size;

    switch(ship)
    {
        case AIRCRAFT_CARRIER:
            size = AIRCRAFT_CARRIER_SIZE;
            break;
        case BATTLESHIP:
            size = BATTLESHIP_SIZE;
            break;
        case SUBMARINE:
            size = SUBMARINE_SIZE;
            break;
        case PATROL_BOAT:
            size = PATROL_BOAT_SIZE;
            break;
    }

    if (orientation == VERTICAL)
        if (x >= m->height || y+size-1 >= m->width)
            return -1;

    if (orientation == HORIZONTAL)
        if (y >= m->width || x+size-1 >= m->height)
            return -1;

    if (orientation == VERTICAL)
    {
        for (i=y; i<y+size; i++)
            if (m->map[i][x] != 0)
                return -1;

        for (i=y; i<y+size; i++)
                m->map[i][x] = SHIP;
    }

    if (orientation == HORIZONTAL)
    {
        for (i=x; i<x+size; i++)
            if (m->map[y][i] != 0)
                return -1;

        for (i=x; i<x+size; i++)
            m->map[y][i] = SHIP;
    }

    m->ships[ship]--;

    return 0;
}

int check_used_ships(Map *m)
{
    int i, missing = 0;

    for (i=0; i<sizeof(m->ships)/sizeof(int); i++)
        if (m->ships[i] > 0)
            missing += m->ships[i];

    return missing;
}