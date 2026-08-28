// ---------------------------------------------------------------------------------------------- //

#ifndef server_included

// ---------------------------------------------------------------------------------------------- //

#define server_included

// ---------------------------------------------------------------------------------------------- //

#include <winsock2.h>

#include <stdio.h>

// ---------------------------------------------------------------------------------------------- //

#include "events.c"

// ---------------------------------------------------------------------------------------------- //

typedef struct server_s {

    char server_ip[32];
    
    int server_port;
    
    SOCKET server_socket;
    
    SOCKET client_socket;

    char server_buffer[1024];
    
    int buffer_len;
    
} server_t;

// ---------------------------------------------------------------------------------------------- //

server_t server = {

    .server_ip = "127.0.0.1",

    .server_port = 8084,

    .server_socket = INVALID_SOCKET,

    .client_socket = INVALID_SOCKET,

    .server_buffer = { 0 },
    
    .buffer_len = 0,

};

// ---------------------------------------------------------------------------------------------- //

void initialize_server(server_t *server) {

    int is_initialized_successfully = 1;

    if (WSAStartup(MAKEWORD(2, 2), &(WSADATA) { 0 }) != 0) is_initialized_successfully = 0;

    if (is_initialized_successfully) {
        
        server -> server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        
        if (server -> server_socket == INVALID_SOCKET) is_initialized_successfully = 0;
        
    }

    if (is_initialized_successfully) {

        setsockopt(server -> server_socket, SOL_SOCKET, SO_REUSEADDR, (const char *) &(int) { 1 }, sizeof(int));

        ioctlsocket(server -> server_socket, FIONBIO, &(u_long) { 1 });

        if (bind(server -> server_socket, (struct sockaddr *) &(struct sockaddr_in) { .sin_family = AF_INET, .sin_port = htons((u_short) server -> server_port), .sin_addr.s_addr = inet_addr(server -> server_ip) }, sizeof(struct sockaddr_in)) == SOCKET_ERROR) is_initialized_successfully = 0;

    }

    if (is_initialized_successfully) {
        
        if (listen(server -> server_socket, SOMAXCONN) == SOCKET_ERROR) is_initialized_successfully = 0;
        
    }

    if (is_initialized_successfully) {

        printf("\n[ INFO ] | server.c | initialize_server() | %s | %d\n", server -> server_ip, server -> server_port);

    } else {

        printf("\n[ ERROR ] | server.c | initialize_server() | an error occured while initializing the server\n");
        
        exit(1);

    }

}

// ---------------------------------------------------------------------------------------------- //

void destroy_server(server_t *server) {

    if (server -> client_socket != INVALID_SOCKET) {
        
        shutdown(server -> client_socket, SD_SEND);
        
        closesocket(server -> client_socket);
        
        server -> client_socket = INVALID_SOCKET;
        
    }
    
    if (server -> server_socket != INVALID_SOCKET) {
        
        closesocket(server -> server_socket);
        
        server -> server_socket = INVALID_SOCKET;
        
    }
    
    WSACleanup();
    
}

// ---------------------------------------------------------------------------------------------- //

void update_server(server_t *server) {

    if (server -> server_socket == INVALID_SOCKET) return;

    if (server -> client_socket == INVALID_SOCKET) {
        
        server -> client_socket = accept(server -> server_socket, NULL, NULL);
        
        if (server -> client_socket != INVALID_SOCKET) {
            
            ioctlsocket(server -> client_socket, FIONBIO, &(u_long) { 1 });
            
            send(server -> client_socket, "connected to dp-cvep-speller\n", 29, 0);
            
        }
        
    }

    if (server -> client_socket != INVALID_SOCKET) {

        int bytes = recv(server -> client_socket, server -> server_buffer + server -> buffer_len, sizeof(server -> server_buffer) - server -> buffer_len - 1, 0);
        
        if (bytes > 0) {
            
            server -> buffer_len += bytes;
            
            server -> server_buffer[server -> buffer_len] = '\0';
            
            int start_idx = 0;
            
            for (int i = 0; i < server -> buffer_len; i++) {
                
                if (server -> server_buffer[i] == '\r' || server -> server_buffer[i] == '\n' || server -> server_buffer[i] == ';' || server -> server_buffer[i] == '|') {
                    
                    server -> server_buffer[i] = '\0';
                    
                    char *cmd = server -> server_buffer + start_idx;
                    
                    if (strcmp(cmd, "TRAINING") == 0) push_event_training();

                    else if (strcmp(cmd, "ONLINE") == 0) push_event_online();

                    else if (strcmp(cmd, "STOP") == 0) push_event_idle();

                    else if (strcmp(cmd, "CLOSE") == 0) push_event_close();

                    else if (strcmp(cmd, "GET_PCOMMS") == 0) send(server -> client_socket, "TRAINING|ONLINE|STOP|CLOSE|GET_PCOMMS|UP", 40, 0);

                    else if (strcmp(cmd, "UP") == 0) send(server -> client_socket, "1", 1, 0);
                    
                    start_idx = i + 1;
                    
                }
                
            }
            
            if (start_idx > 0) {
                
                int remaining = server -> buffer_len - start_idx;
                
                if (remaining > 0) memmove(server -> server_buffer, server -> server_buffer + start_idx, remaining);
                
                server -> buffer_len = remaining;
                
                server -> server_buffer[server -> buffer_len] = '\0';
                
            }
            
            if (server -> buffer_len >= sizeof(server -> server_buffer) - 1) {
                
                server -> buffer_len = 0;
                
                server -> server_buffer[0] = '\0';
                
            }
            
        } else if (bytes == 0 || (bytes == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)) {

            closesocket(server -> client_socket);
            
            server -> client_socket = INVALID_SOCKET;
            
            server -> buffer_len = 0;

        }
        
    }

}

// ---------------------------------------------------------------------------------------------- //

#endif

// ---------------------------------------------------------------------------------------------- //