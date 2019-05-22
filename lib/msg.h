#ifndef _MSG_H_
#define _MSG_H_

#define MISS 0
#define HIT 1
#define SUNK 2
#define GAMEOVER 3

#define ID_MSG_TYPE 1
#define HOLD_MSG_TYPE 2
#define START_MSG_TYPE 3
#define INSERT_MSG_TYPE 4
#define BEGIN_MSG_TYPE 5
#define ATTACK_MSG_TYPE 6
#define STATUS_MSG_TYPE 7

typedef struct id_message
{
    int type;
    int id;

} id_message;

typedef struct hold_message
{
    int type;

} hold_message;

typedef struct start_message
{
    int type;

} start_message;

typedef struct insert_message
{
    int type;
    int id;
    int ship, x, y, orientation;

} insert_message;

typedef struct begin_message
{
    int type;
    int id;

} begin_message;

typedef struct attack_message
{
    int type;
    int id;
    int x, y;

} attack_message;

typedef struct status_message
{
    int type;
    int id;
    /* 0 = MISS
     * 1 = HIT
     * 2 = OK ??
     * 3 = FINISH
     */
    int x, y;
    int response;
    int options;

} status_message;

int send_id_message(int sockfd, int id);
int send_hold_message(int sockfd);
int send_start_message(int sockfd);
int send_begin_message(int sockfd, int id);
int send_attack_message(int sockfd, int id, int x, int y);
int send_status_message(int sockfd, int id, int x, int y, int response, int options);
int send_insert_message(int sockfd, int id, int ship, int x, int y, int orientation);
int receive_message(int sockfd, void** msg);
#endif