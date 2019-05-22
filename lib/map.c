#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "map.h"
#include "ship.h"
#include "util.h"


int check_map(Map* m)
{
    int i, j;

    for(i=0; i<m->width; i++)
        for(j=0; j<m->height; j++)
            if (m->map[i][j][0] == SHIP)
                return 1;

    return 0;
}
int getType(Map *m, int x, int y){
    return m->map[y][x][1];
}
int getIndex(Map *m, int x, int y){
    return m->map[y][x][2];
}
void show_ships(Map *m)
{
    int i;

    printf("Size:\t\t\t  1  2  3  4\n");

    printf("0) Aircraft carier (%i)\t", m->ships[0]);
    for(i=0;i<AIRCRAFT_CARRIER_SIZE; i++) {printf("  "); PRINT_GREEN(SHIP_SYMBOL);}

    printf("\n1) Battleship (%i)\t", m->ships[1]);
    for(i=0;i<BATTLESHIP_SIZE; i++) {printf("  "); PRINT_GREEN(SHIP_SYMBOL);}
    for(i=0;i<AIRCRAFT_CARRIER_SIZE-BATTLESHIP_SIZE; i++) {printf("  "); PRINT_BLUE(WATER_SYMBOL);}

    printf("\n2) Submarine (%i)\t", m->ships[2]);
    for(i=0;i<SUBMARINE_SIZE; i++) {printf("  "); PRINT_GREEN(SHIP_SYMBOL);}
    for(i=0;i<AIRCRAFT_CARRIER_SIZE-SUBMARINE_SIZE; i++) {printf("  "); PRINT_BLUE(WATER_SYMBOL);}

    printf("\n3) Patrol boat (%i)\t", m->ships[3]);
    for(i=0;i<PATROL_BOAT_SIZE; i++) {printf("  "); PRINT_GREEN(SHIP_SYMBOL);}
    for(i=0;i<AIRCRAFT_CARRIER_SIZE-PATROL_BOAT_SIZE; i++) {printf("  "); PRINT_BLUE(WATER_SYMBOL);}

    printf("\n\n");
}

void show_ships_left(Map *m)
{
    int i;

    printf("Size:\t\t\t  1  2  3  4\n");

    printf("Aircraft carier (%i)\t", m->ships[0]);
    for(i=0;i<AIRCRAFT_CARRIER_SIZE; i++) {printf("  "); PRINT_GREEN(SHIP_SYMBOL);}

    printf("\nBattleship (%i)\t\t", m->ships[1]);
    for(i=0;i<BATTLESHIP_SIZE; i++) {printf("  "); PRINT_GREEN(SHIP_SYMBOL);}
    for(i=0;i<AIRCRAFT_CARRIER_SIZE-BATTLESHIP_SIZE; i++) {printf("  "); PRINT_BLUE(WATER_SYMBOL);}

    printf("\nSubmarine (%i)\t\t", m->ships[2]);
    for(i=0;i<SUBMARINE_SIZE; i++) {printf("  "); PRINT_GREEN(SHIP_SYMBOL);}
    for(i=0;i<AIRCRAFT_CARRIER_SIZE-SUBMARINE_SIZE; i++) {printf("  "); PRINT_BLUE(WATER_SYMBOL);}

    printf("\nPatrol boat (%i)\t\t", m->ships[3]);
    for(i=0;i<PATROL_BOAT_SIZE; i++) {printf("  "); PRINT_GREEN(SHIP_SYMBOL);}
    for(i=0;i<AIRCRAFT_CARRIER_SIZE-PATROL_BOAT_SIZE; i++) {printf("  "); PRINT_BLUE(WATER_SYMBOL);}

    printf("\n\n");
}

void show_map(Map *m)
{
    int i, j;

    // Print first line
    printf(" y\\x ");
    for (i=0; i<m->height; i++)
        if (i<10)
            printf("%i  ", i);
        else
            printf("%i ", i);

    printf("\n");

    // Print letters line
    for (i=0; i<m->width; i++)
    {
        // Print letter
        if (i<10)
            printf("  %i", i);
        else
            printf(" %i", i);

        for (j=0; j<m->height; j++)
        {
            switch(m->map[i][j][0])
            {
                case WATER:
                    printf("  ");
                    PRINT_BLUE(WATER_SYMBOL);
                    break;
                case MISSED:
                    printf("  ");
                    PRINT_RED(MISS_SYMBOL);
                    break;
                case SHIP:
                    printf("  ");
                    PRINT_GREEN(SHIP_SYMBOL);
                    break;
                case DESTROYED:
                    printf("  ");
                    PRINT_RED(HIT_SYMBOL);
                    break;
                default:
                    printf("  ?");
                    break;
            }
            // printf("  %i", m->map[i][j]);

        }
        printf("\n");
    }
    printf("\n");
}
void show_maps(Map *m1, Map *m2)
{
    int i, j;

    // Print first line
    printf(" y\\x ");
    for (i=0; i<m1->height; i++)
        if (i<10)
            printf("%i  ", i);
        else
            printf("%i ", i);

    printf("    ");

    printf(" y\\x ");
    for (i=0; i<m2->height; i++)
        if (i<10)
            printf("%i  ", i);
        else
            printf("%i ", i);

    printf("\n");

    // Print letters line
    for (i=0; i<m1->width; i++)
    {
        // Print letter
        if (i<10)
            printf("  %i", i);
        else
            printf(" %i", i);

        for (j=0; j<m1->height; j++)
        {
            switch(m1->map[i][j][0])
            {
                case WATER:
                    printf("  ");
                    PRINT_BLUE(WATER_SYMBOL);
                    break;
                case MISSED:
                    printf("  ");
                    PRINT_RED(MISS_SYMBOL);
                    break;
                case SHIP:
                    printf("  ");
                    PRINT_GREEN(SHIP_SYMBOL);
                    break;
                case DESTROYED:
                    printf("  ");
                    PRINT_RED(HIT_SYMBOL);
                    break;
                default:
                    printf("  ?");
                    break;
            }
            // printf("  %i", m->map[i][j]);

        }
        printf("      ");

        // Print letter
        if (i<10)
            printf("  %i", i);
        else
            printf(" %i", i);

        for (j=0; j<m2->height; j++)
        {
            switch(m2->map[i][j][0])
            {
                case WATER:
                    printf("  ");
                    PRINT_BLUE(WATER_SYMBOL);
                    break;
                case MISSED:
                    printf("  ");
                    PRINT_RED(MISS_SYMBOL);
                    break;
                case SHIP:
                    printf("  ");
                    PRINT_GREEN(SHIP_SYMBOL);
                    break;
                case DESTROYED:
                    printf("  ");
                    PRINT_RED(HIT_SYMBOL);
                    break;
                default:
                    printf("  ?");
                    break;
            }
            // printf("  %i", m->map[i][j]);

        }
        printf("\n");
    }
    printf("\n");
}

Map *init_map_matrix(int width, int height)
{
    Map *m = malloc(sizeof(Map));
    int i, j;

    m->width = width;
    m->height = height;

    m->map = malloc(m->width * sizeof(int **));

    for(i=0; i<m->width; i++)
    {
        m->map[i] = malloc(m->height * sizeof(int *));
        for(j=0; j<m->height; j++){
            m->map[i][j] = malloc(3 * sizeof(int));
            
            m->map[i][j][0] = 0;
            m->map[i][j][1] = -1;
            m->map[i][j][2] = -1;
        }
    }

    m->ships[0] = AIRCRAFT_CARRIER_COUNT;
    m->ships[1] = BATTLESHIP_COUNT;
    m->ships[2] = SUBMARINE_COUNT;
    m->ships[3] = PATROL_BOAT_COUNT;

    m->ships_staus = malloc(4 * sizeof(int *));
    m->ships_staus[0] = malloc(AIRCRAFT_CARRIER_COUNT * sizeof(int));
    m->ships_staus[1] = malloc(BATTLESHIP_COUNT * sizeof(int));
    m->ships_staus[2] = malloc(SUBMARINE_COUNT * sizeof(int));
    m->ships_staus[3] = malloc(PATROL_BOAT_COUNT * sizeof(int));

    // set all to zero
    for(i=0; i<AIRCRAFT_CARRIER_COUNT; i++)
        m->ships_staus[0][i] = -1;
    for(i=0; i<BATTLESHIP_COUNT; i++)
        m->ships_staus[1][i] = -1;
    for(i=0; i<SUBMARINE_COUNT; i++)
        m->ships_staus[2][i] = -1;
    for(i=0; i<PATROL_BOAT_COUNT; i++)
        m->ships_staus[3][i] = -1;

    /*  STATUS OF SHIP NR:
            0   1   2   3
    cztero  4   *   *   *
    trzy    3   3   *   *
    twu     2   2   2   *
    jedno   1   1   1   1

    */
    return m;
}

