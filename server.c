#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

#include "lib/msg.h"
#include "lib/map.h"
#include "lib/ship.h"
#include "lib/util.h"
#include "lib/config.h"
#include "lib/multicast.h"

#define DEBUG

int num_of_games = 0;

void sig_chld(int signo)
{
    pid_t   pid;
    int     stat;

    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("child %d terminated\n", pid);
    return;
}

void error(const char *msg)
{
    syslog (LOG_ERR, msg);
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

    syslog (LOG_INFO,"Listener set.");

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
    
    syslog (LOG_INFO,"Waiting for clients...");

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

        syslog (LOG_INFO,"Accepted connection from client %d", num_conn);
        
        /* Send the client it's ID. */
        if(send_id_message(cli_sockfd[num_conn], num_conn) < 0)
            error("ERROR sending ID to client");

        syslog (LOG_INFO,"Sent client %d it's ID.", num_conn);
        
        /* Increment the player count. */

        if (num_conn == 0) {
            /* Send "HLD" to first client to let the user know the server is waiting on a second client. */
            if(send_hold_message(cli_sockfd[0]) < 0)
                error("ERROR sending HOLD to client");
            
            syslog (LOG_INFO,"Told client 0 to hold.");
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

    syslog (LOG_INFO,"Waiting for players' ship coordinates");

    do
    {
        syslog (LOG_INFO,"Waiting for INSERT messages from clients...");
        FD_SET(cli_sockfd[0], &rset);
        FD_SET(cli_sockfd[1], &rset);
        if ( ( n = select(maxfd+1, &rset, NULL, NULL, NULL) ) < 0){
            return -4;
        }
        syslog (LOG_INFO,"%d sockets ready for read.\n", n);
        /* TO MOZE BYC BLAD GNIAZDA NIE KONIECZNIE READY READ*/
        if (FD_ISSET(cli_sockfd[0], &rset)) {
            syslog (LOG_INFO,"Socket 0 is ready");
            if( (msg_type = receive_message(cli_sockfd[0], &message) ) < 0 )
                error("ERROR receiving message form client");
        }
        if (FD_ISSET(cli_sockfd[1], &rset)) {
            syslog (LOG_INFO,"Socket 1 is ready");
            if( (msg_type = receive_message(cli_sockfd[1], &message) ) < 0 )
                error("ERROR receiving message form client");
        }
        if(msg_type == INSERT_MSG_TYPE){
            syslog (LOG_INFO,"recived INSERT message");
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

            syslog (LOG_INFO,"Ships left to assign: Player0=%d, Player1=%d\n", ships_left_p0, ships_left_p1);

            if(ships_left_p0 == 0){
                syslog (LOG_INFO,"Recived all player0's coordinates");
                if(!hold_sent){
                    if(send_hold_message(cli_sockfd[0]) < 0)
                        error("ERROR sending HOLD to client");
                    hold_sent = 1;
                    syslog (LOG_INFO,"sent HOLD to player0");
                }
            }
            if(ships_left_p1 == 0){
                syslog (LOG_INFO,"Recived all player1's coordinates");
                if(!hold_sent){
                    if(send_hold_message(cli_sockfd[1]) < 0)
                        error("ERROR sending HOLD to client");
                    hold_sent = 1;
                    syslog (LOG_INFO,"sent HOLD to player1");
                }
            }
            
        }else
            error("ERROR wrong message type, expected INSERT message");
    }while(ships_left_p0 > 0 || ships_left_p1 > 0);

    syslog (LOG_INFO,"Both players' ship coordinates recived. Starting the game...");
    return 1;
}

/* Runs a game between two clients. */
void run_game(int *cli_sockfd /*void *thread_data*/)
{
    //int *cli_sockfd = (int*)thread_data; /* Client sockets. */
    syslog (LOG_INFO,"GAME ON!");

    /* Create maps for player 1 and 2*/
    Map *map_p0 = init_map_matrix(MAP_WIDTH, MAP_HEIGH);
    Map *map_p1 = init_map_matrix(MAP_WIDTH, MAP_HEIGH);
    
    /* Send the start message. */
    if(send_start_message(cli_sockfd[0]) < 0)
        error("ERROR sending START to client1");
    if(send_start_message(cli_sockfd[1]) < 0)
        error("ERROR sending START to client2");

    syslog (LOG_INFO,"Sent start message.");

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
        syslog (LOG_INFO,"Waiting for Player%d's move\n", player_turn);
        if( (msg_type = receive_message(cli_sockfd[player_turn], &message) ) < 0 )
                error("ERROR receiving message form client");
        if(msg_type == ATTACK_MSG_TYPE){
            syslog (LOG_INFO,"Recived ATTACK message");
            Map *map = ((attack_message*)message)->id ? map_p0 : map_p1;
            if ( ( code = attack_ship(map, ((attack_message*)message)->x, ((attack_message*)message)->y) ) ){

                if (check_map(map) == 0)
                {
                    syslog (LOG_INFO,"GAME OVER!");
                    if(send_status_message(cli_sockfd[0], ((attack_message*)message)->id, ((attack_message*)message)->x, ((attack_message*)message)->y, GAMEOVER, 0) < 0)
                        error("ERROR sending STATUS to client");
                    if(send_status_message(cli_sockfd[1], ((attack_message*)message)->id, ((attack_message*)message)->x, ((attack_message*)message)->y, GAMEOVER, 0) < 0)
                        error("ERROR sending STATUS to client");
                    syslog (LOG_INFO,"Sent STATUS message");
                    break;
                }
                else
                {
                    if(code == 1){
                        syslog (LOG_INFO,"HIT");
                        if(send_status_message(cli_sockfd[0], ((attack_message*)message)->id, ((attack_message*)message)->x, ((attack_message*)message)->y, HIT, 0) < 0)
                            error("ERROR sending STATUS to client");
                        if(send_status_message(cli_sockfd[1], ((attack_message*)message)->id, ((attack_message*)message)->x, ((attack_message*)message)->y, HIT, 0) < 0)
                            error("ERROR sending STATUS to client");
                        syslog (LOG_INFO,"Sent STATUS message");
                    }else if(code == 2){
                        syslog (LOG_INFO,"SUNK");
                        int ship_type = getType(map, ((attack_message*)message)->x, ((attack_message*)message)->y );
                        if(send_status_message(cli_sockfd[0], ((attack_message*)message)->id, ((attack_message*)message)->x, ((attack_message*)message)->y, SUNK, ship_type) < 0)
                            error("ERROR sending STATUS to client");
                        if(send_status_message(cli_sockfd[1], ((attack_message*)message)->id, ((attack_message*)message)->x, ((attack_message*)message)->y, SUNK, ship_type) < 0)
                            error("ERROR sending STATUS to client");
                        syslog (LOG_INFO,"Sent STATUS message");
                    }
                }
            }else{
                syslog (LOG_INFO,"MISS");
                if(send_status_message(cli_sockfd[0], ((attack_message*)message)->id, ((attack_message*)message)->x, ((attack_message*)message)->y, MISS, 0) < 0)
                    error("ERROR sending STATUS to client");
                if(send_status_message(cli_sockfd[1], ((attack_message*)message)->id, ((attack_message*)message)->x, ((attack_message*)message)->y, MISS, 0) < 0)
                    error("ERROR sending STATUS to client");
                syslog (LOG_INFO,"Sent STATUS message");
                player_turn = !player_turn; //change player turn
            }
            show_maps(map_p0, map_p1);  
        }
        else
            error("ERROR wrong message type, expected ATTACK message");
    }

    syslog (LOG_INFO,"QUITTING...");

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

#define MAXFD   64

int daemon_init(const char *pname, int facility, uid_t uid, int socket){
    int     i, p;
    pid_t   pid;

    if ( (pid = fork()) < 0)
        return (-1);
    else if (pid)
        exit(0);            /* parent terminates */

    /* child 1 continues... */

    if (setsid() < 0)           /* become session leader */
        return (-1);

    signal(SIGHUP, SIG_IGN);
    if ( (pid = fork()) < 0)
        return (-1);
    else if (pid)
        exit(0);            /* child 1 terminates */

    /* child 2 continues... */

    chdir("/tmp");              /* change working directory  or chroot()*/
    //chroot("/tmp");

    /* close off file descriptors */
    for (i = 0; i < MAXFD; i++){
        if(socket != i )
            close(i);
    }

    /* redirect stdin, stdout, and stderr to /dev/null */
    p= open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);

    openlog(pname, LOG_PID, facility);
    
        syslog(LOG_ERR," STDIN =   %i\n", p);
    setuid(uid); /* change user */
    
    return (0);             /* success */
}

int main(int argc, char *argv[])
{   
    void                sig_chld(int);
    struct sigaction new_action, old_action;

  /* Set up the structure to specify the new action. */
    new_action.sa_handler = sig_chld;
    //new_action.sa_handler = SIG_IGN;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;

    if( sigaction (SIGCHLD, &new_action, &old_action) < 0 ){
          fprintf(stderr,"sigaction error : %s\n", strerror(errno));
          return 1;
    }
 

    /* Start program as deamon */

    daemon_init(argv[0], LOG_USER, 1000, MAXFD);
    syslog (LOG_NOTICE, "Program started by User %d", getuid ());

    /* Multicast part*/
    if ( fork() == 0) {
        service_discovery();
        exit(0);
    }
    
    /* Unicast part */
    //int lis_sockfd = setup_listener(atoi(argv[1])); /* Listener socket. */
    syslog (LOG_INFO,"Setting up listener...");

    int lis_sockfd = setup_listener(SERVICE_PORT);

    while (1) {
        if( (num_of_games++) < 10) { /* maximum number of hosted games */
            int *cli_sockfd = (int*)malloc(2*sizeof(int)); /* Client sockets */
            memset(cli_sockfd, 0, 2*sizeof(int));
            
            /* Get two clients connected. */
            get_clients(lis_sockfd, cli_sockfd);
            
            syslog (LOG_INFO,"Starting new game thread... ");
            
            if ( fork() == 0) {    /* child process */
                close(lis_sockfd);    /* close listening socket */
                run_game(cli_sockfd);   /* process the request */
                exit(0);
            }
            close(cli_sockfd[0]);
            close(cli_sockfd[1]);
            
            syslog (LOG_INFO,"New game thread started.");
        }
    }

    close(lis_sockfd);
}
