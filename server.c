#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include "lib/msg.h"
#include "lib/map.h"
#include "lib/ship.h"
#include "lib/util.h"
#include "lib/config.h"
#include "lib/multicast.h"

#define DEBUG

int num_of_games = 0;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

/* Sets up the listener socket. */
int setup_listener(int portno)
{
    int sockfd;
    struct sockaddr_in serv_addr;

    /* Get a socket to listen on */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening listener socket.");

    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        error("ERROR setsockopt() error");
    
    /* Zero out the memory for the server information */
    memset(&serv_addr, 0, sizeof(serv_addr));
    
	/* set up the server info */
    serv_addr.sin_family = AF_INET;	
    serv_addr.sin_addr.s_addr = INADDR_ANY;	
    serv_addr.sin_port = htons(portno);		

    /* Bind the server info to the listener socket. */
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR binding listener socket.");

    /* Listen for clients. */
    listen(sockfd, /*253 - player_count*/ 2);

    #ifdef DEBUG
    printf("[DEBUG] Listener set.\n");    
    #endif 

    /* Return the socket number. */
    return sockfd;
}

/* Sets up the client sockets and client connections. */
void get_clients(int lis_sockfd, int * cli_sockfd)
{
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    id_message id_msg;
    hold_message hold_msg;
    
    #ifdef DEBUG
    printf("[DEBUG] Listening for clients...\n");
    #endif 

    /* Listen for two clients. */
    int num_conn = 0;
    while(num_conn < 2)
    {
        /* Zero out memory for the client information. */
        memset(&cli_addr, 0, sizeof(cli_addr));

        clilen = sizeof(cli_addr);
	
	    /* Accept the connection from the client. */
        cli_sockfd[num_conn] = accept(lis_sockfd, (struct sockaddr *) &cli_addr, &clilen);
    
        if (cli_sockfd[num_conn] < 0)
            /* Horrible things have happened. */
            error("ERROR accepting a connection from a client.");

        #ifdef DEBUG
        printf("[DEBUG] Accepted connection from client %d\n", num_conn);
        #endif 
        
        /* Send the client it's ID. */
        if(send_id_message(cli_sockfd[num_conn], num_conn) < 0)
            error("ERROR sending ID to client");

        #ifdef DEBUG
        printf("[DEBUG] Sent client %d it's ID.\n", num_conn); 
        #endif 
        
        /* Increment the player count. */

        if (num_conn == 0) {
            /* Send "HLD" to first client to let the user know the server is waiting on a second client. */
            if(send_hold_message(cli_sockfd[0]) < 0)
                error("ERROR sending HOLD to client");
            
            #ifdef DEBUG
            printf("[DEBUG] Told client 0 to hold.\n");
            #endif 
        }

        num_conn++;
    }
}

int get_clients_coords(int *cli_sockfd, Map* map_p0, Map* map_p1){
    void* message = NULL;
    int msg_type;
    int ships_left_p0 = 0;
    int ships_left_p1 = 0;
    int hold_sent = 0;

    int maxfd, n;
    fd_set rset;

    FD_ZERO(&rset);

    if(cli_sockfd[0]>cli_sockfd[1])
        maxfd = cli_sockfd[0];
    else
        maxfd = cli_sockfd[1];

    printf("Waiting for players' ship coordinates\n");

    do
    {
        #ifdef DEBUG
        printf("[DEBUG] Waiting for INSERT messages from clients...\n");
        #endif
        FD_SET(cli_sockfd[0], &rset);
        FD_SET(cli_sockfd[1], &rset);
        if ( ( n = select(maxfd+1, &rset, NULL, NULL, NULL) ) < 0){
            return -4;
        }
        #ifdef DEBUG
        printf("[DEBUG] %d sockets ready for read.\n", n);
        #endif
        /* TO MOZE BYC BLAD GNIAZDA NIE KONIECZNIE READY READ*/
        if (FD_ISSET(cli_sockfd[0], &rset)) {
            #ifdef DEBUG
            printf("[DEBUG] socket 0 is ready");
            #endif
            if( (msg_type = receive_message(cli_sockfd[0], &message) ) < 0 )
                error("ERROR receiving message form client");
        }
        if (FD_ISSET(cli_sockfd[1], &rset)) {
            #ifdef DEBUG
            printf("[DEBUG] socket 1 is ready");
            #endif
            if( (msg_type = receive_message(cli_sockfd[1], &message) ) < 0 )
                error("ERROR receiving message form client");
        }
        if(msg_type == INSERT_MSG_TYPE){
            #ifdef DEBUG
            printf("[DEBUG] recived INSERT message.\n");
            #endif
            if (insert_ship(
                ((insert_message*)message)->id ? map_p1 : map_p0,
                ((insert_message*)message)->ship,
                ((insert_message*)message)->x,
                ((insert_message*)message)->y,
                ((insert_message*)message)->orientation
                )== -1)
                error("ERROR inserting ship based on recived client data");

            ships_left_p0 = check_used_ships(map_p0);
            ships_left_p1 = check_used_ships(map_p1);
            #ifdef DEBUG
            printf("[DEBUG] Ships left to assign: Player0=%d, Player1=%d\n", ships_left_p0, ships_left_p1);
            #endif

            if(ships_left_p0 == 0){
                #ifdef DEBUG
                printf("[DEBUG] Recived all player0's coordinates\n");
                #endif
                if(!hold_sent){
                    if(send_hold_message(cli_sockfd[0]) < 0)
                        error("ERROR sending HOLD to client");
                    hold_sent = 1;
                    #ifdef DEBUG
                    printf("[DEBUG] sent HOLD to player0\n");
                    #endif
                }
            }
            if(ships_left_p1 == 0){
                #ifdef DEBUG
                printf("[DEBUG] Recived all player1's coordinates\n");
                #endif
                if(!hold_sent){
                    if(send_hold_message(cli_sockfd[1]) < 0)
                        error("ERROR sending HOLD to client");
                    hold_sent = 1;
                    #ifdef DEBUG
                    printf("[DEBUG] sent HOLD to player1\n");
                    #endif
                }
            }
            
        }else
            error("ERROR wrong message type, expected INSERT message");
    }while(ships_left_p0 > 0 || ships_left_p1 > 0);

    #ifdef DEBUG
    printf("[DEBUG] Both players' ship coordinates recived. Starting the game...\n");
    #endif
    return 1;
}

/* Runs a game between two clients. */
void run_game(int *cli_sockfd /*void *thread_data*/)
{
    //int *cli_sockfd = (int*)thread_data; /* Client sockets. */
    printf("Game on!\n");

    /* Create maps for player 1 and 2*/
    Map *map_p0 = init_map_matrix(MAP_WIDTH, MAP_HEIGH);
    Map *map_p1 = init_map_matrix(MAP_WIDTH, MAP_HEIGH);
    
    /* Send the start message. */
    if(send_start_message(cli_sockfd[0]) < 0)
        error("ERROR sending START to client1");
    if(send_start_message(cli_sockfd[1]) < 0)
        error("ERROR sending START to client2");

    #ifdef DEBUG
    printf("[DEBUG] Sent start message.\n");
    #endif

    /* Initialize map for both players*/
    get_clients_coords( cli_sockfd, map_p0, map_p1);

    /* Pick random player who starts*/
    srand(time(NULL));
    int starting_id = rand() % 2;

    /* Send begin to both*/
    if(send_begin_message(cli_sockfd[0], starting_id) < 0)
        error("ERROR sending BEGIN to client1");
    if(send_begin_message(cli_sockfd[1], starting_id) < 0)
        error("ERROR sending BEGIN to client2");

    show_maps(map_p0, map_p1);
    
    int game_over = 0;
    int player_turn = starting_id;
    int msg_type, code;
    void* message = NULL;
    while(!game_over) {
        #ifdef DEBUG
        printf("[DEBUG] Waiting for Player%d's move\n", player_turn);
        #endif
        if( (msg_type = receive_message(cli_sockfd[player_turn], &message) ) < 0 )
                error("ERROR receiving message form client");
        if(msg_type == ATTACK_MSG_TYPE){
            #ifdef DEBUG
            printf("[DEBUG] recived ATTACK message\n");
            #endif
            Map *map = ((attack_message*)message)->id ? map_p0 : map_p1;
            if ( ( code = attack_ship(map, ((attack_message*)message)->x, ((attack_message*)message)->y) ) ){

                if (check_map(map) == 0)
                {
                    #ifdef DEBUG
                    printf("[DEBUG] GAME OVER\n");
                    #endif
                    if(send_status_message(cli_sockfd[0], ((attack_message*)message)->id, ((attack_message*)message)->x, ((attack_message*)message)->y, GAMEOVER, 0) < 0)
                        error("ERROR sending STATUS to client");
                    if(send_status_message(cli_sockfd[1], ((attack_message*)message)->id, ((attack_message*)message)->x, ((attack_message*)message)->y, GAMEOVER, 0) < 0)
                        error("ERROR sending STATUS to client");
                    #ifdef DEBUG
                    printf("[DEBUG] sent STATUS message\n");
                    #endif
                    break;
                }
                else
                {
                    if(code == 1){
                        #ifdef DEBUG
                        printf("[DEBUG] HIT\n");
                        #endif
                        if(send_status_message(cli_sockfd[0], ((attack_message*)message)->id, ((attack_message*)message)->x, ((attack_message*)message)->y, HIT, 0) < 0)
                            error("ERROR sending STATUS to client");
                        if(send_status_message(cli_sockfd[1], ((attack_message*)message)->id, ((attack_message*)message)->x, ((attack_message*)message)->y, HIT, 0) < 0)
                            error("ERROR sending STATUS to client");
                        #ifdef DEBUG
                        printf("[DEBUG] sent STATUS message\n");
                        #endif
                    }else if(code == 2){
                        #ifdef DEBUG
                        printf("[DEBUG] SUNK\n");
                        #endif
                        int ship_type = getType(map, ((attack_message*)message)->x, ((attack_message*)message)->y );
                        if(send_status_message(cli_sockfd[0], ((attack_message*)message)->id, ((attack_message*)message)->x, ((attack_message*)message)->y, SUNK, ship_type) < 0)
                            error("ERROR sending STATUS to client");
                        if(send_status_message(cli_sockfd[1], ((attack_message*)message)->id, ((attack_message*)message)->x, ((attack_message*)message)->y, SUNK, ship_type) < 0)
                            error("ERROR sending STATUS to client");
                        #ifdef DEBUG
                        printf("[DEBUG] sent STATUS message\n");
                        #endif
                    }
                }
            }else{
                #ifdef DEBUG
                printf("[DEBUG] MISS\n");
                #endif
                if(send_status_message(cli_sockfd[0], ((attack_message*)message)->id, ((attack_message*)message)->x, ((attack_message*)message)->y, MISS, 0) < 0)
                    error("ERROR sending STATUS to client");
                if(send_status_message(cli_sockfd[1], ((attack_message*)message)->id, ((attack_message*)message)->x, ((attack_message*)message)->y, MISS, 0) < 0)
                    error("ERROR sending STATUS to client");
                #ifdef DEBUG
                printf("[DEBUG] sent STATUS message\n");
                #endif
                player_turn = !player_turn; //change player turn
            }
            show_maps(map_p0, map_p1);  
        }
        else
            error("ERROR wrong message type, expected ATTACK message");
    }

    printf("Game over.\n");

	/* Close client sockets and decrement player counter. */
    close(cli_sockfd[0]);
    close(cli_sockfd[1]);
    
    free(cli_sockfd);
}

/* 
 * Main Program
 */
void service_discovery()
{
    int sendfd, recvfd;
    const int on = 1;
    socklen_t salen;
    struct sockaddr *sasend, *sarecv;
    struct sockaddr_in6 *ipv6addr;
    struct sockaddr_in *ipv4addr;
    char   *addr_str;

    sendfd = snd_udp_socket(SERVICE_MULTICAST_ADDR, SERVICE_PORT, &sasend, &salen);

    if ( (recvfd = socket(sasend->sa_family, SOCK_DGRAM, 0)) < 0){
        error("ERROR: socket error");
    }

    if (setsockopt(recvfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0){
        error("ERROR: setsockopt error");
    }

    sarecv = malloc(salen);
    memcpy(sarecv, sasend, salen);
    
    if(sarecv->sa_family == AF_INET6){
      ipv6addr = (struct sockaddr_in6 *) sarecv;
      ipv6addr->sin6_addr =  in6addr_any;
    }

    if(sarecv->sa_family == AF_INET){
      ipv4addr = (struct sockaddr_in *) sarecv;
      ipv4addr->sin_addr.s_addr =  htonl(INADDR_ANY);
    }
    
    if( bind(recvfd, sarecv, salen) < 0 ){
        error("ERROR: bind error");
    }
    
    if( mcast_join(recvfd, sasend, salen, NULL, 0) < 0 ){
        error("ERROR: mcast_join() error");
    }
      
    mcast_set_loop(sendfd, 1);

    while(1)
        recv_multicast(recvfd, salen);
}

int main(int argc, char *argv[])
{   
    /* Make sure a port was specified. */
    /*if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }*/
    /* Multicast part*/
    if ( fork() == 0) {
        service_discovery();
    }
    
    /* Unicast part */
    //int lis_sockfd = setup_listener(atoi(argv[1])); /* Listener socket. */
    int lis_sockfd = setup_listener(SERVICE_PORT);

    while (1) {
        if( (num_of_games++) < 10) { /* maximum number of hosted games */
            int *cli_sockfd = (int*)malloc(2*sizeof(int)); /* Client sockets */
            memset(cli_sockfd, 0, 2*sizeof(int));
            
            /* Get two clients connected. */
            get_clients(lis_sockfd, cli_sockfd);
            
            #ifdef DEBUG
            printf("[DEBUG] Starting new game thread...\n");
            #endif
            
            if ( fork() == 0) {    /* child process */
                close(lis_sockfd);    /* close listening socket */
                run_game(cli_sockfd);   /* process the request */
                exit(0);
            }
            close(cli_sockfd[0]);
            close(cli_sockfd[1]);
            
            #ifdef DEBUG
            printf("[DEBUG] New game thread started.\n");
            #endif
        }
    }

    close(lis_sockfd);
}
