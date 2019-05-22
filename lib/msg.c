#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "msg.h"
#include "map.h"

int send_id_message(int sockfd, int id){
	id_message msg;
	msg.type = ID_MSG_TYPE;
	msg.id = id;
	if(send(sockfd, &msg, sizeof(id_message), 0) < 0)
		return -1;
	else
		return 1;
}
int send_hold_message(int sockfd){
    hold_message msg;
	msg.type = HOLD_MSG_TYPE;
	if(send(sockfd, &msg, sizeof(hold_message), 0) < 0)
		return -1;
	else
		return 1;
}
int send_start_message(int sockfd){
    start_message msg;
	msg.type = START_MSG_TYPE;
	if(send(sockfd, &msg, sizeof(start_message), 0) < 0)
		return -1;
	else{
		return 1;
	}
}
int send_insert_message(int sockfd, int id, int ship, int x, int y, int orientation){
	insert_message msg;
	msg.type = INSERT_MSG_TYPE;
	msg.id = id;
	msg.ship = ship;
	msg.x = x;
	msg.y = y;
	msg.orientation = orientation;
	if(send(sockfd, &msg, sizeof(insert_message), 0) < 0)
		return -1;
	else{
		return 1;
	}

}
int send_begin_message(int sockfd, int id){
    begin_message msg;
	msg.type = BEGIN_MSG_TYPE;
	msg.id = id;
	if(send(sockfd, &msg, sizeof(begin_message), 0) < 0)
		return -1;
	else{
		return 1;
	}
}
int send_attack_message(int sockfd, int id, int x, int y){
	attack_message msg;
	msg.type = ATTACK_MSG_TYPE;
	msg.id = id;
	msg.x = x;
	msg.y = y;
	if(send(sockfd, &msg, sizeof(attack_message), 0) < 0)
		return -1;
	else{
		return 1;
	}
}
int send_status_message(int sockfd, int id, int x, int y, int response){
	status_message msg;
	msg.type = STATUS_MSG_TYPE;
	msg.id = id;
	msg.x = x;
	msg.y = y;
	msg.response = response;
	if(send(sockfd, &msg, sizeof(status_message), 0) < 0)
		return -1;
	else{
		return 1;
	}
}
int receive_message(int sockfd, void** msg){
	if(*msg != NULL)
		free(*msg);
	int n, type = 0;
	if( (n = recv(sockfd, &type, sizeof(int), MSG_PEEK) ) < 0 )
		return -1;

    #ifdef DEBUG
    printf("[DEBUG] type value: %d\n", type);
    #endif

	switch(type)
        {
            case ID_MSG_TYPE:
            	*msg = malloc(sizeof(id_message));
                if( (n = recv(sockfd, *msg, sizeof(id_message), 0) ) < 0)
                	return -2;
                else{
                	return ID_MSG_TYPE;
                }
                break;
            case HOLD_MSG_TYPE:
            	*msg = malloc(sizeof(hold_message));
                if( (n = recv(sockfd, *msg, sizeof(hold_message), 0) ) < 0)
                	return -2;
                else
                	return HOLD_MSG_TYPE;
                break;
            case START_MSG_TYPE:
            	*msg = malloc(sizeof(start_message));
                if( (n = recv(sockfd, *msg, sizeof(start_message), 0) ) < 0)
                	return -2;
                else
                	return START_MSG_TYPE;
                break;
            case INSERT_MSG_TYPE:
            	*msg = malloc(sizeof(insert_message));
                if( (n = recv(sockfd, *msg, sizeof(insert_message), 0) ) < 0)
                	return -2;
                else
                	return INSERT_MSG_TYPE;
                break;
            case BEGIN_MSG_TYPE:
            	*msg = malloc(sizeof(begin_message));
                if( (n = recv(sockfd, *msg, sizeof(begin_message), 0) ) < 0)
                	return -2;
                else
                	return BEGIN_MSG_TYPE;
                break;
            case ATTACK_MSG_TYPE:
            	*msg = malloc(sizeof(attack_message));
                if( (n = recv(sockfd, *msg, sizeof(attack_message), 0) ) < 0)
                	return -2;
                else
                	return ATTACK_MSG_TYPE;
                break;
            case STATUS_MSG_TYPE:
            	*msg = malloc(sizeof(status_message));
                if( (n = recv(sockfd, *msg, sizeof(status_message), 0) ) < 0)
                	return -2;
                else
                	return STATUS_MSG_TYPE;
                break;
            default:
                return -3;
                break;
        }
}