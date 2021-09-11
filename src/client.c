#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#include "include/sock.h"
#include "include/api.h"

/*
 *****************************************************************************
 * DBClient_LoadClient --
 *
 * Establishes unix socket for client and connects to server.
 *
 * PARAMS: none
 * RETURNS: fd of socket on success
 *          -1 on failure
 *****************************************************************************
 */

int
DBClient_LoadClient()
{
   int sock;
   size_t len;
   struct sockaddr_un addr;
   int connected;

   sock = socket(AF_UNIX, SOCK_STREAM, 0);
   if (sock == -1) {
      fprintf(stdout, "%s:%d: failed to create unix socket.\n",
              __FUNCTION__, __LINE__);
      return -1;
   }

   addr.sun_family = AF_UNIX;
   strncpy(addr.sun_path, SOCK_PATH, strlen(SOCK_PATH) + 1);
   len = strlen(addr.sun_path) + sizeof(addr.sun_family) + 1;

   connected = connect(sock, (struct sockaddr *) &addr, len);
   if (connected == -1) {
      fprintf(stdout, "%s:%d: failed to connect to server.\n",
              __FUNCTION__, __LINE__);
      return -1;
   }
   
   return sock;
}


int
main(void)
{
   int client_socket;
   char *dummy = "hello";
   char buf[MAX_BUF_SZ];
   
   client_socket = DBClient_LoadClient();
   if (client_socket == -1) {
      fprintf(stdout, "%s:%d: failed to load client.\n",
              __FUNCTION__, __LINE__);
      exit(1);
   }
   
   while(printf("> "), fgets(buf, MAX_BUF_SZ, stdin), !feof(stdin)) {
      Message send_msg;
      Message rec_msg;
      send_msg.status = DB_OK;
      send_msg.length = strlen(buf);
      strncpy(send_msg.payload, buf, send_msg.length);
      send_msg.payload[send_msg.length - 1] = '\0';

      if (send(client_socket, &send_msg, sizeof(send_msg) - 1, 0) == -1) {
         fprintf(stdout, "%s:%d: failed to send message.\n",
                 __FUNCTION__, __LINE__);
         exit(1);
      }
      printf("client sent mess\n");
      if (send(client_socket, send_msg.payload, send_msg.length, 0) == -1) {
         fprintf(stdout, "%s:%d: failed to send message.\n",
                 __FUNCTION__, __LINE__);
         exit(1);
      }
      
      if (recv(client_socket, (void *) &rec_msg, sizeof(rec_msg) - 1, 0) == -1) {
         fprintf(stdout, "%s:%d: failed to receive message from server.\n",
                 __FUNCTION__, __LINE__);
         exit(1);
      }
 
      switch(rec_msg.status) {
         case DB_OK:
         case DB_CONTINUE:
            if (recv(client_socket, (void *) rec_msg.payload, rec_msg.length, 0) == -1) {
               fprintf(stdout, "%s:%d: failed to receive message from server.\n",
                       __FUNCTION__, __LINE__);
               exit(1);
            }
            break;
         case DB_ERROR:
            /*
             * TODO: Cleanup socket.
             */
            break;
      }
      fprintf(stdout, "Message from server: %s\n", (char *) rec_msg.payload);
      buf[0] = '\0';
   }
   
}
