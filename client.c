#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "lib/msg.h"
#include "lib/map.h"
#include "lib/ship.h"
#include "lib/util.h"
#include "lib/config.h"

#define DEBUG
#define RANDOMIZE

void error(const char *msg)
{
    #ifdef DEBUG
    perror(msg);
    #else
    printf("Either the server shut down or the other player disconnected.\nGame over.\n");
    #endif 

    exit(0);
}

/* Sets up the connection to the server. */
int connect_to_server(char * hostname, int portno)
{
    struct sockaddr_in serv_addr;
    struct hostent *server;
 
    /* Get a socket. */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
    if (sockfd < 0) 
        error("ERROR opening socket for server.");
	
    /* Get the address of the server. */
    server = gethostbyname(hostname);
	
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
	
	/* Zero out memory for server info. */
	memset(&serv_addr, 0, sizeof(serv_addr));

	/* Set up the server info. */
    serv_addr.sin_family = AF_INET;
    memmove(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno); 

	/* Make the connection. */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting to server");

    #ifdef DEBUG
    printf("[DEBUG] Connected to server.\n");
    #endif 
    
    return sockfd;
}

int attack(int sockfd, int id){
    int x, y;

    printf("Your turn to attack\n");
    printf("x: ");
    scanf("%i", &x);
    while(x > MAP_WIDTH || x < 0){
        printf("Invalid choice.");
        scanf("%i", &x);
    }
    printf("y: ");
    scanf("%i", &y);
    while(y > MAP_HEIGH || y < 0){
        printf("Invalid choice.");
        scanf("%i", &x);
    }
    if(send_attack_message(sockfd, id, x, y) < 0)
        error("ERROR sending ATTACK message");
    #ifdef DEBUG
    printf("[DEBUG] sent ATTACK message\n");
    #endif
}

int main(int argc, char *argv[])
{
    /* Make sure host and port are specified. */
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    /* Connect to the server. */
    int sockfd = connect_to_server(argv[1], atoi(argv[2]));
    printf("Connected.\n");

    /* The client ID is the first thing we receive after connecting. */    
    int id;
    void* message = NULL;
    int msg_type;
    if( (msg_type = receive_message(sockfd, &message) ) < 0 )
        error("ERROR receiving ID form server");
    else
        if(msg_type == ID_MSG_TYPE){
            id = ((id_message*)message)->id;
            #ifdef DEBUG
            printf("[DEBUG] received ID message\n");
            #endif
        }else
            error("ERROR wrong message type, expected id_message");
    

    #ifdef DEBUG
    printf("[DEBUG] Client ID: %d\n", id);
    #endif 

    /* Wait for the game to start. */
    do {
        if( (msg_type = receive_message(sockfd, &message) ) < 0 ){
            error("ERROR receiving message form server");
        }
        else
            if(msg_type == HOLD_MSG_TYPE){
                #ifdef DEBUG
                printf("[DEBUG] received HOLD message\n");
                #endif
                printf("Waiting for a second player...\n");
                }         
    } while ( msg_type != START_MSG_TYPE );

    #ifdef DEBUG
    printf("[DEBUG] received START message\n");
    #endif

    /* --------------------- The game has begun. ------------------------------ */

    printf("Game on!\n");
    sleep(1);

    Map *my_map = init_map_matrix(MAP_WIDTH, MAP_HEIGH);
    Map *opponent_map = init_map_matrix(MAP_WIDTH, MAP_HEIGH);
    int i, ship, x, y, orientation; // variables used for ship insertion;

    #ifdef RANDOMIZE
    srand(time(NULL));
    #endif
    /*Insert ships*/
    while((i = check_used_ships(my_map)) > 0)
    {
        show_map(my_map);
        show_ships(my_map);

        printf("You still have %i ship(s) to organize\n", i);
        printf("Choose one to put in the map: ");
        #ifdef RANDOMIZE
        ship = rand() % 4;
        #else
        scanf("%i", &ship);
        #endif
        while( (ship > 3 || ship < 0) || my_map->ships[ship] <= 0)
        {
            printf("Invalid choice\n");
            printf("Please, choose another one: ");
            #ifdef RANDOMIZE
            ship = rand() % 4;
            #else
            scanf("%i", &ship);
            #endif
        }

        printf("Orientation (1-vert/2-hori): ");
        #ifdef RANDOMIZE
        orientation = 1 + rand() % 2;
        #else
        scanf("%i", &orientation);
        #endif
        while(orientation != 1 && orientation != 2)
        {
            printf("Invalid choice\n");
            printf("Please, choose another option: ");
            #ifdef RANDOMIZE
            orientation = 1 + rand() % 2;
            #else
            scanf("%i", &orientation);
            #endif
        }

        printf("Ship head position\n");
        printf("x: ");
        #ifdef RANDOMIZE
        x = rand() % 10;
        #else
        scanf("%i", &x);
        #endif
        while(x > my_map->width || x < 0)
        {
            printf("Invalid choice\n");
            printf("Please, choose another x: ");
            #ifdef RANDOMIZE
            x = rand() % 10;
            #else
            scanf("%i", &x);
            #endif
        }

        printf("y: ");
        #ifdef RANDOMIZE
        y = rand() % 10;
        #else
        scanf("%i", &y);
        #endif
        while(y > my_map->height || y < 0)
        {
            printf("Invalid choice\n");
            printf("Please, choose another y: ");
            #ifdef RANDOMIZE
            y = rand() % 10;
            #else
            scanf("%i", &y);
            #endif
        }


        if (insert_ship(my_map, ship, x, y, orientation) == -1)
            printf("\nOut of limit!\nChoose again...\n\n");
        else
            if(send_insert_message(sockfd, id, ship, x, y, orientation) < 0)
                error("ERROR sending INSERT to server");
            else{
                #ifdef DEBUG
                printf("[DEBUG] sent INSERT message\n");
                #endif
            }
    }

    //system("clear");

    printf("\nShips ready!\n");

    show_maps(my_map, opponent_map);
    show_ships_left(opponent_map);

    do {
        if( (msg_type = receive_message(sockfd, &message) ) < 0 ){
            error("ERROR receiving message form server");
        }
        else
            if(msg_type == HOLD_MSG_TYPE){
                #ifdef DEBUG
                printf("[DEBUG] received HOLD message\n");
                #endif
                printf("Waiting for a second player...\n");
                }         
    } while ( msg_type != BEGIN_MSG_TYPE );
    #ifdef DEBUG
        printf("[DEBUG] recived BEGIN message\n");
    #endif

    /*First move*/
    if (id == ((begin_message*)message)->id)
        attack(sockfd, id);
    else
        printf("Oponent's move. Waiting...\n");
    
    int ship_type;

    while(1) {
        if( (msg_type = receive_message(sockfd, &message) ) < 0 ){
            error("ERROR receiving message form server");
        }
        else
            if(msg_type == STATUS_MSG_TYPE){
                #ifdef DEBUG
                printf("[DEBUG] received STATUS message\n");
                #endif
                /* response on my atttack msg */
                if( id == ((status_message*)message)->id ){
                    switch (((status_message*)message)->response)
                    {
                        case MISS:
                            opponent_map->map[((status_message*)message)->y][((status_message*)message)->x][0] = MISSED;
                            show_maps(my_map, opponent_map);
                            show_ships_left(opponent_map);
                            printf("Oponent's move. Waiting...\n");
                            break;
                        case HIT:
                            opponent_map->map[((status_message*)message)->y][((status_message*)message)->x][0] = DESTROYED;
                            show_maps(my_map, opponent_map);
                            show_ships_left(opponent_map);
                            printf("You hit.\n");
                            attack(sockfd, id);
                            break;
                        case SUNK:
                            opponent_map->map[((status_message*)message)->y][((status_message*)message)->x][0] = DESTROYED;
                            ship_type = ((status_message*)message)->options;
                            opponent_map->ships[ship_type]--;
                            show_maps(my_map, opponent_map);
                            show_ships_left(opponent_map);
                            printf("Hit and sunk!\n");
                            attack(sockfd, id);
                            break;
                        case GAMEOVER:
                            opponent_map->map[((status_message*)message)->y][((status_message*)message)->x][0] = DESTROYED;
                            ship_type = ((status_message*)message)->options;
                            opponent_map->ships[ship_type]--;
                            show_maps(my_map, opponent_map);
                            show_ships_left(opponent_map);
                            PRINT_BLUE("\nYOU WON!!!\n\n");
                            close(sockfd);
                            return 0;
                        default:
                            break;
                    }
                /* response on opponent's attack msg */
                }else{
                    switch (((status_message*)message)->response)
                    {
                        case MISS:
                            my_map->map[((status_message*)message)->y][((status_message*)message)->x][0] = MISSED;
                            show_maps(my_map, opponent_map);
                            show_ships_left(opponent_map);
                            printf("Oponent missed.\n");
                            attack(sockfd, id);
                            break;
                        case HIT:
                            my_map->map[((status_message*)message)->y][((status_message*)message)->x][0] = DESTROYED;
                            show_maps(my_map, opponent_map);
                            show_ships_left(opponent_map);
                            printf("Oponent hit. Waiting for his next move...\n");
                            break;
                        case SUNK:
                            my_map->map[((status_message*)message)->y][((status_message*)message)->x][0] = DESTROYED;
                            // more code...
                            show_maps(my_map, opponent_map);
                            show_ships_left(opponent_map);
                            printf("Oponent has hit and sunk your ship!\n");
                            break;
                        case GAMEOVER:
                            my_map->map[((status_message*)message)->y][((status_message*)message)->x][0] = DESTROYED;
                            show_maps(my_map, opponent_map);
                            show_ships_left(opponent_map);
                            PRINT_RED("\nYOU LOST!!!\n\n");
                            close(sockfd);
                            return 0;
                        default:
                            break;
                    }
                }
            }else
                error("ERROR wrong message type, expected ATTACK message");
    } 
    printf("Game over.\n");

    /* Close server socket and exit. */
    close(sockfd);
    return 0;
}
