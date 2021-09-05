/** server.c
 *
 * This file provides a basic unix socket implementation for a server
 * used in an interactive client-server database.
 * The client should be able to send messages containing queries to the
 * server.  When the server receives a message, it must:
 * 1. Respond with a status based on the query (OK, UNKNOWN_QUERY, etc.)
 * 2. Process any appropriate queries, if applicable.
 * 3. Return the query response to the client.
 **/

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


/*
 *****************************************************************************
 * DBServer_HandleClient --
 *
 *  Continually receive messages from client and execute queries.
 *  1. Parse the command
 *  2. Handle request if appropriate
 *  3. Send status of the received message (OK, UNKNOWN_QUERY, etc)
 *  4. Send response to the request.
 *
 * PARAMS: client socket [IN]
 * RETURNS: true  on success
 *          false on failure
 *****************************************************************************
 */

void
DBServer_HandleClient(int socket)
{
   char buf[100];
   char *rec = "received!";
   
   if (recv(socket, (void *) buf, sizeof buf, 0) == -1) {
      fprintf(stdout, "%s:%d: failed to receive message from client.\n",
              __FUNCTION__, __LINE__);
      return;
   }

   fprintf(stdout, "%s:%d: message from client: %s\n",
           __FUNCTION__, __LINE__, (char *) buf);
   
   if (send(socket, (void *) rec, strlen(rec), 0) == -1) {
      fprintf(stdout, "%s:%d: failed to send response to client.\n",
              __FUNCTION__, __LINE__);
      return;
   }
}

/*
 *****************************************************************************
 * DBServer_LoadServer --
 *
 * Establishes unix socket for server and listens for client
 * connections.
 *
 * PARAMS: none
 * RETURNS: fd of socket on success
 *          -1 on failure
 *****************************************************************************
 */

int 
DBServer_LoadServer()
{
   int sock;
   size_t len;
   struct sockaddr_un addr;

   sock = socket(AF_UNIX, SOCK_STREAM, 0);

   if (sock == -1) {
      fprintf(stdout, "%s:%d: failed to create unix socket.\n",
             __FUNCTION__, __LINE__);
      return -1;
   }
   addr.sun_family = AF_UNIX;
   strncpy(addr.sun_path, SOCK_PATH, strlen(SOCK_PATH) + 1);
   /*
    * unlink sun_path to remove the socket if it already exists.
    * This must be done before calling bind()
    */
   unlink(addr.sun_path);
   len = strlen(addr.sun_path) + sizeof(addr.sun_family) + 1;
   if (bind(sock, (struct sockaddr *)&addr, len) == -1) {
      fprintf(stdout, "%s:%d: Socket failed to bind.\n",
              __FUNCTION__, __LINE__);
      return -1;
   }

   if (listen(sock, 5) == -1) {
      fprintf(stdout, "%s:%d: Failed to listen on socket.\n",
              __FUNCTION__, __LINE__);
      return -1;
   }

   return sock;
}


int
main(void) 
{
   int client_socket;
   struct sockaddr_un remote;
   uint remote_len = sizeof(remote);
   int server_socket = DBServer_LoadServer();
   
   if (server_socket == -1) {
      fprintf(stdout, "%s:%d: error loading server.\n",
              __FUNCTION__, __LINE__);
      exit(1);
   }

   for (;;) {
      client_socket = accept(server_socket, (struct sockaddr *) &remote, &remote_len);
      if (client_socket == -1) {
         fprintf(stdout, "%s:%d: error accepting client request.\n",
                 __FUNCTION__, __LINE__);
         exit(1);
      }

      DBServer_HandleClient(client_socket);
   }
}
