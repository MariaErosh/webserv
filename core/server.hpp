#pragma once

#include "./HttpRequestProcessor.hpp"
#include "../config/config.hpp"
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <sys/select.h>

#define CLIENT_DISCONNECTED -1

  class Server  {
	public:
		static Server instance_;

	private:
		Server() {}
		Server(Server&) { }
		Server& operator=(const Server&) { return *this; }

	public:
		// Server initialization.
		void    init(const char* config_path);

		// Runs the server.
		int     run(void);

	private:
		// Init configuration file
		void    initConfig(const char* config_path);

		// Init connections (not servers) list with IP:Port pairs
		void    initConnectionsSet(void);

		// Init listening sockets
		void    initSockets(void);

		// Tells if the socket is a listening socket
		bool  isListening(int socket) const;

		/* Function for accepting client connection
		*  Throws exception  when accept() fails
		*/
		int   acceptConnection(int listening_socket) const;

		// Handler for client connections
		void  handleConnection(int client_socket, fd_set& writefds);

		// Handler for client disconnections
		void  handleDisconnection(int client_socket);

		// Recieve a message from a client
		int   recvMsg(int socket_recv_from);

		// Handle a message recieved from a client
		int   handleMsg(std::string msg, int msg_owner);

		// Send a message to a client
		void  sendMsg(int socket_to_send, const char* msg, int msg_size) const;

		// Close all listening sockets
		void  closeListeningSockets() const;

	private:
		const Config  conf_;


		// Data struct of info about one connection, that can be used by different servers
		struct ConnectionInfo {
			std::string ip_addr;
			std::string port;

			ConnectionInfo() {}

			ConnectionInfo(const std::string& ip_addr, const std::string& port)	: ip_addr(ip_addr), port(port){}

			bool operator<(const struct ConnectionInfo& second) const {
				if (this->ip_addr == second.ip_addr)
				return (this->port < second.port);
				return (this->ip_addr < second.ip_addr);
			}
		};

		fd_set                                      master_set_;        // master set of all sockets
		std::set<struct ConnectionInfo>             connections_set_;   // list of connection infos
		std::vector<int>                            listening_sockets_; // list of listening sockets
		std::map<int, const struct ConnectionInfo*> socket_infos_;      // < socket to ConnectionInfo map

  };

