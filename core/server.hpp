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
		void	configure(const char* configPath);

		int		run(void);

	private:

		void	loadConfig(const char* configPath);
		void	initializeConnections(void);
		void	initializeSockets(void);
		bool	isListeningSocket(int socket) const;
		int		acceptClient(int listeningSocket) const;
		void	handleClient(int clientSocket, fd_set& writableSockets);
		void	disconnectClient(int clientSocket);
		int		receiveData(int socket);
		int		processData(std::string message, int client);
		void	sendData(int socket, const char* message, int messageSize) const;
		void	closeSockets() const;

	private:
		const Config  config_;

		struct ConnectionInfo {
			std::string ipAddress;
			std::string port;

			ConnectionInfo() {}

			ConnectionInfo(const std::string& ipAddress, const std::string& port)	: ipAddress(ipAddress), port(port){}

			bool operator<(const struct ConnectionInfo& second) const {
				if (this->ipAddress == second.ipAddress)
				return (this->port < second.port);
				return (this->ipAddress < second.ipAddress);
			}
		};

		fd_set										masterSockets_;
		std::set<struct ConnectionInfo>				connections_;
		std::vector<int>							listeningSockets_;
		std::map<int, const struct ConnectionInfo*>	socketToConnection_;

};

