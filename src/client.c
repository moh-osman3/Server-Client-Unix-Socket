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
   char buf[100];
   
   client_socket = DBClient_LoadClient();
   if (client_socket == -1) {
      fprintf(stdout, "%s:%d: failed to load client.\n",
              __FUNCTION__, __LINE__);
      exit(1);
   }
   
   while(printf("> "), fgets(buf, 100, stdin), !feof(stdin)) {
      if (send(client_socket, (void *) buf, strlen(buf), 0) == -1) {
         fprintf(stdout, "%s:%d: failed to send message.\n",
                 __FUNCTION__, __LINE__);
         exit(1);
      }
      
      if (recv(client_socket, (void *) buf, sizeof buf, 0) == -1) {
         fprintf(stdout, "%s:%d: failed to receive message from server.\n",
                 __FUNCTION__, __LINE__);
         exit(1);
      }
      fprintf(stdout, "buf: %s\n", (char *) buf);
   }
   
}
