#ifndef API_H
#define API_H

#define MAX_BUF_SZ 4096
 
typedef enum Status {
   DB_OK,
   DB_CONTINUE,
   DB_ERROR,
} Status;

typedef struct Message {
   Status status;
   size_t length;
   char payload[MAX_BUF_SZ];   
} Message;

#endif // API_H
