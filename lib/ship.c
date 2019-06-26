#include <stdio.h>

#include "map.h"
#include "util.h"
#include "ship.h"


int attack_ship(Map *m, int x, int y)
{
    if (x > m->width || x < 0 || y > m->width || y < 0)
        return -1;

    if (m->map[y][x][0] == SHIP || m->map[y][x][0] == DESTROYED)
    {
        m->map[y][x][0] = DESTROYED;
        printf("[DEBUG] ship type: %d\n", m->map[y][x][1]);
        printf("[DEBUG] ship index: %d\n", m->map[y][x][2]);
        printf("[DEBUG] status before %d\n", ( m->ships_staus[ m->map[y][x][1] ][ m->map[y][x][2] ]));
        if( --( m->ships_staus[ m->map[y][x][1] ][ m->map[y][x][2] ]) == 0)
            return 2;
        else
            return 1;
    }
    else {
        m->map[y][x][0] = MISSED;
        return 0;
    }
}

int insert_ship(Map *m, int ship_type, int x, int y, int orientation)
{
    int i, j, size, count, index;

    switch(ship_type)
    {
        case AIRCRAFT_CARRIER:
            size = AIRCRAFT_CARRIER_SIZE;
            count = AIRCRAFT_CARRIER_COUNT;
            break;
        case BATTLESHIP:
            size = BATTLESHIP_SIZE;
            count = BATTLESHIP_COUNT;
            break;
        case SUBMARINE:
            size = SUBMARINE_SIZE;
            count = SUBMARINE_COUNT;
            break;
        case PATROL_BOAT:
            size = PATROL_BOAT_SIZE;
            count = PATROL_BOAT_COUNT;
            break;
    }
    
    if (orientation == VERTICAL)
        if (x >= m->height || y+size-1 >= m->width)
            return -1;

    if (orientation == HORIZONTAL)
        if (y >= m->width || x+size-1 >= m->height)
            return -1;

    //printf("[DEBUG] placing ship (size %d) at x:%d y:%d\n", size, x, y);
    if (orientation == VERTICAL)
    {
        for (i=y; i<y+size; i++){
            if(i==y){
                //printf("[DEBUG] checking at x:%d y:%d\n", x, i-1);
                if(i-1 >= 0)
                    if (m->map[i-1][x][0] != 0) // check up, upleft, upright
                        return -1;
                //printf("[DEBUG] checking at x:%d y:%d\n", x-1, i-1);
                if(i-1 >= 0 && x-1 >= 0)
                    if(m->map[i-1][x-1][0] != 0)
                        return -1;
                //printf("[DEBUG] checking at x:%d y:%d\n", x+1, i-1);
                if(i-1 >= 0 && x+1 < m->width)
                    if(m->map[i-1][x+1][0] != 0)
                        return -1;
            }
            //printf("[DEBUG] checking at x:%d y:%d\n", x, i);
            if(1)
                if (m->map[i][x][0] != 0) // check directly at
                    return -1;
            //printf("[DEBUG] checking at x:%d y:%d\n", x+1, i);
            if(x+1 < m->width)
                if (m->map[i][x+1][0] != 0) // check right
                    return -1;
            //printf("[DEBUG] checking at x:%d y:%d\n", x-1, i);
            if(x-1 >= 0)
                if (m->map[i][x-1][0] != 0) // check left
                    return -1;
            if(i==y+size-1){
                //printf("[DEBUG] checking at x:%d y:%d\n", x, i+1);
                if(i+1 < m->height)
                    if (m->map[i+1][x][0] != 0) // check down, downleft, upright
                        return -1;
                //printf("[DEBUG] checking at x:%d y:%d\n", x-1, i+1);
                if(i+1 < m->height && x-1 >= 0)
                    if (m->map[i+1][x-1][0] != 0) // check down, downleft, upright
                        return -1;
                //printf("[DEBUG] checking at x:%d y:%d\n", x+1, i+1);
                if(i+1 < m->width && x+1 < m->width)
                    if (m->map[i+1][x+1][0] != 0) // check down, downleft, upright
                        return -1;
            }
        }
			
		for(j=0; j < count; j++){
			if( m->ships_staus[ship_type][j] == -1){
				m->ships_staus[ship_type][j] = size;
				index = j;
				break;
			}
		}
		
        for (i=y; i<y+size; i++){
                m->map[i][x][0] = SHIP; // this tells there is a ship
                m->map[i][x][1] = ship_type; // this tells what is the ship type
				m->map[i][x][2] = index; // this tells which one is it from all of the type above

        }
    }

    if (orientation == HORIZONTAL)
    {
        for (i=x; i<x+size; i++){
            if(i==x){
                //printf("[DEBUG] checking at x:%d y:%d\n", i-1, y);
                if(i-1 >= 0)
                    if (m->map[y][i-1][0] != 0) 
                        return -1;
                //printf("[DEBUG] checking at x:%d y:%d\n", i-1, y-1);
                if(i-1 >= 0 && y-1 >= 0)
                    if(m->map[y-1][i-1][0] != 0)
                        return -1;
                //printf("[DEBUG] checking at x:%d y:%d\n", i-1, y+1);
                if(i-1 >= 0 && y+1 < m->height)
                    if(m->map[y+1][i-1][0] != 0)
                        return -1;
            }
            //printf("[DEBUG] checking at x:%d y:%d\n", i, y);
            if(1)
                if (m->map[y][i][0] != 0) 
                    return -1;
            //printf("[DEBUG] checking at x:%d y:%d\n", i, y+1);
            if(y+1 < m->height)
                if (m->map[y+1][i][0] != 0) 
                    return -1;
            //printf("[DEBUG] checking at x:%d y:%d\n", i, y-1);
            if(y-1 >= 0)
                if (m->map[y-1][i][0] != 0) 
                    return -1;
            if(i==x+size-1){
                printf("[DEBUG] checking at x:%d y:%d\n", i+1, y);
                if(i+1 < m->width)
                    if (m->map[y][i+1][0] != 0) 
                        return -1;
                //printf("[DEBUG] checking at x:%d y:%d\n", i-1, y-1);
                if(i+1 < m->width && y-1 >= 0)
                    if (m->map[y-1][i+1][0] != 0) 
                        return -1;
                //printf("[DEBUG] checking at x:%d y:%d\n", i-1, y+1);
                if(i+1 < m->width && y+1 < m->height)
                    if (m->map[y+1][i+1][0] != 0)
                        return -1;
            }
        }

		for(j=0; j < count; j++){
			if( m->ships_staus[ship_type][j] == -1){
				m->ships_staus[ship_type][j] = size;
				index = j;
				break;
			}
		}
		
        for (i=x; i<x+size; i++){
            m->map[y][i][0] = SHIP;
            m->map[y][i][1] = ship_type;
			m->map[y][i][2] = index;
        }
    }

    m->ships[ship_type]--;

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