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
#include <stdbool.h>

#include "include/sock.h"
#include "include/api.h"

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
   bool listen = true;
   char *recStr = "received";
   while (listen) {
      Message send_msg;
      send_msg.status = DB_OK;
      send_msg.length = strlen(recStr);
      strncpy(send_msg.payload, recStr, send_msg.length);
      /*
       * BUG: make sure send_msg.length is less than MAX_BUF_SZ.
       * This code will be changed, so keep in mind.
       */
      send_msg.payload[send_msg.length] = '\0';

      Message rec_msg; 
      if (recv(socket, &rec_msg, sizeof rec_msg - 1, 0) == -1) {
         fprintf(stdout, "%s:%d: failed to receive message from client.\n",
                 __FUNCTION__, __LINE__);
         return;
      }

      /*
       * Check the status from the client.
       */ 
      switch (rec_msg.status) {
         case DB_OK:
            /*
             * Parse request and execute request.
             * Send back response.
             */
            if (recv(socket, (void *) rec_msg.payload, rec_msg.length, 0) == -1) {
               fprintf(stdout, "%s:%d: failed to receive message from client.\n",
                       __FUNCTION__, __LINE__);
               return;
            }
            fprintf(stdout, "%s:%d: message from client: %s\n",
                    __FUNCTION__, __LINE__, rec_msg.payload);
            goto send_response_to_client;
             
         case DB_CONTINUE:
            /*
             * Receiving long message (like file),
             * so continue to receive msg, no need
             * to parse command.
             */
            goto send_response_to_client;
         case DB_ERROR:
            /*
             * Shutdown. Make sure to save db on server
             * side, so client data persists
             */
            goto send_response_to_client;
      }

   
send_response_to_client:

      if (send(socket, (void *) &send_msg, sizeof(send_msg) - 1, 0) == -1) {
         fprintf(stdout, "%s:%d: failed to send response to client.\n",
                 __FUNCTION__, __LINE__);
         return;
      }
      if (send(socket, (void *) send_msg.payload, send_msg.length, 0) == -1) {
         fprintf(stdout, "%s:%d: failed to send response to client.\n",
                 __FUNCTION__, __LINE__);
         return;
      }
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

   client_socket = accept(server_socket, (struct sockaddr *) &remote, &remote_len);
   if (client_socket == -1) {
      fprintf(stdout, "%s:%d: error accepting client request.\n",
              __FUNCTION__, __LINE__);
      exit(1);
   }
   DBServer_HandleClient(client_socket);
}
