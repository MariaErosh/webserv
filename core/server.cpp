#include "./server.hpp"
#include "../utils/logger.hpp"
#include "../utils/containers.hpp"
#include <csignal>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <errno.h>
#include <algorithm>
#include <stdlib.h>

	Server  Server::instance_;

	void	Server::configure(const char* configPath) {
		this->loadConfig(configPath);
		this->initializeConnections();
		this->initializeSockets();
	}


	void	Server::loadConfig(const char* configPath) {
			ParserConfig::parseFile(configPath, const_cast<Config&>(this->config_));
	}


	void	Server::initializeConnections(void) {
		for (size_t i = 0; i < this->config_.serverConfigurations.size(); ++i) {
		this->connections_.insert(Server::ConnectionInfo(
										this->config_.serverConfigurations[i].ipAddress,
										this->config_.serverConfigurations[i].port)
										);
		}
	}


	void	Server::initializeSockets(void) {
		FD_ZERO(&masterSockets_);

		this->listeningSockets_.reserve(config_.serverConfigurations.size());

		std::set<Server::ConnectionInfo>::const_iterator connectionIterator;
		for (connectionIterator = this->connections_.begin(); connectionIterator != this->connections_.end(); ++connectionIterator)	{
		this->listeningSockets_.push_back(socket(AF_INET, SOCK_STREAM, 0));
			if ((this->listeningSockets_.back()) == -1)
				throw std::runtime_error("Failed to create a listening socket: " + std::string(strerror(errno)));

			int reuseOption = 1;
			if (setsockopt(
				this->listeningSockets_.back(),
				SOL_SOCKET,
				SO_REUSEADDR,
				&reuseOption,
				sizeof(int)) == -1
				)
				throw std::runtime_error("Can't setsockopt() at listening socket: " + std::string(strerror(errno)));

			sockaddr_in socketAddress;
			std::memset(&socketAddress, 0, sizeof(socketAddress));

			socketAddress.sin_family = AF_INET;

			uint16_t portNumber = atoi(connectionIterator->port.c_str());
			socketAddress.sin_port = htons(portNumber);

			if ((socketAddress.sin_addr.s_addr = inet_addr(connectionIterator->ipAddress.c_str())) == INADDR_NONE)
				throw std::runtime_error("Non valid ipAddress: " + std::string(strerror(errno)));

			if (bind(this->listeningSockets_.back(), (sockaddr *)&socketAddress, sizeof(socketAddress)) == -1)
			{
				throw std::runtime_error("Can't bind() the socket: " + std::string(strerror(errno)));
			}

			if (listen(this->listeningSockets_.back(), SOMAXCONN) == -1)
				throw std::runtime_error("Can't listen() the socket: " + std::string(strerror(errno)));

			FD_SET(this->listeningSockets_.back(), &masterSockets_);

			socketToConnection_.insert(std::make_pair(
				this->listeningSockets_.back(),
				&(*connectionIterator)));
		}
	}

	void	Server::closeSockets() const {
		Logger::instance_.debug(" closeSockets !");
		for (size_t i = 0; i < config_.serverConfigurations.size() ; i++) {
			if (close(listeningSockets_[i]) == -1)
				throw std::runtime_error("Can't close a listening socket: " + std::string(strerror(errno)));
		}
	}	

	void Server::signalHandler(int signum) {
		std::cout << "Received signal " << signum << ", closing sockets..." << std::endl;
		Server::instance_.closeSockets();
		exit(signum);
	}

	int Server::run() {
		fd_set				activeReadSockets;
		fd_set				activeWriteSockets;
		int					clientSocket;

		signal(SIGINT, signalHandler);
		while (true) {
			
			activeReadSockets = masterSockets_;
			activeWriteSockets = masterSockets_;

			if (select(FD_SETSIZE, &activeReadSockets, &activeWriteSockets, NULL, NULL) == -1)
				throw std::runtime_error("Can't select() fdsets: " + std::string(strerror(errno)));

			for (int socket = 0; socket < FD_SETSIZE; socket++) {
				if (FD_ISSET(socket, &activeReadSockets)) {
					if (isListeningSocket(socket)) {  //for listening sockets
						clientSocket = acceptClient(socket);
						socketToConnection_[clientSocket] = socketToConnection_[socket];
						FD_SET(clientSocket, &masterSockets_);
					} else { //for client sockets
						handleClient(socket, activeWriteSockets);
					}
				}
			}
		}
		closeSockets();
	}


	bool  Server::isListeningSocket(int socket) const {
		return Containers::contains(this->listeningSockets_, socket);
	}


	int  Server::acceptClient(int listeningSocket) const {
		std::stringstream	logMessage;
		std::stringstream	debugMessage;
		sockaddr_in			clientAddress;
		socklen_t			clientAddressSize = sizeof(clientAddress);
		int					clientSocket;

		if ((clientSocket = accept(listeningSocket, (sockaddr *)&clientAddress, &clientAddressSize)) == -1)
			throw std::runtime_error("Can't accept() the client: " + std::string(strerror(errno)));
		logMessage << "Client #" << clientSocket << " has been accepted";
		Logger::instance_.logInfo(logMessage.str());

		debugMessage << "Client #" << clientSocket << ": " << inet_ntoa(clientAddress.sin_addr) << ":" << htons(clientAddress.sin_port);
		Logger::instance_.debug(debugMessage.str());

		return clientSocket;
	}

	void  Server::handleClient(int clientSocket, fd_set& writableSockets) {
			if (receiveData(clientSocket) != CLIENT_DISCONNECTED && FD_ISSET(clientSocket, &writableSockets)) {
				sendData(clientSocket, "Message has been recieved!\n", sizeof("Message has been recieved!\n"));
			}
	}

	void	Server::disconnectClient(int clientSocket) {
			std::stringstream logMessage;

			if (close(clientSocket) == -1)
				throw std::runtime_error("Can't close() the client's connection: " + std::string(strerror(errno)));
			FD_CLR(clientSocket, &masterSockets_);
			logMessage << "Client #" << clientSocket << " has disconnected";
			Logger::instance_.logInfo(logMessage.str());
	}


	int	Server::processData(std::string message, int client) {
			static std::map<int, std::string>	socket_to_pending_request; // stors content of unfinished request(e.g. for telnet)

			Logger::debug("#" + String::convertToString(client) + "Recieved: " + message);

			// get connection info
			const Server::ConnectionInfo* info = socketToConnection_.at(client);

			// make response
			int connection_status = 1; //unexisting status for valgrind
			//std::cout << "sock:" << client << std::endl;

			//Check that msg is is finished(has \r\n\r\n). If not-> continue
			if (message.find("\r\n\r\n") == std::string::npos) {
				socket_to_pending_request[client] += message;
				if (socket_to_pending_request[client].find("\r\n\r\n") != std::string::npos ) {
					message = socket_to_pending_request[client];
					socket_to_pending_request[client].clear();
				}
				else
					return CONNECTION_CONTINUE;
			}
			else {
				if (!socket_to_pending_request[client].empty()) {
					message = socket_to_pending_request[client] + message;
					socket_to_pending_request[client].clear();
				}
			}

			std::string response = HttpRequestProcessor::handleClientInput (
				message,
				info->ipAddress,
				info->port,
				this->config_,
				connection_status
			);

			Logger::instance_.debug("SENDING RESPONSE");
			Logger::instance_.debug("{" + response + "}");

			sendData(client, response.c_str(), response.size());

			Logger::instance_.debug("RESPONSE HAS BEEN SENDED");

			if (connection_status == CONNECTION_TERMINATE){
				disconnectClient(client);
				return CLIENT_DISCONNECTED;
			}
			return CONNECTION_CONTINUE;
	}

	int	Server::receiveData(int socket) {
			std::string			receivedData;
			char				dataBuffer[4096];
			int					bytesRead;

			while ((bytesRead = recv(socket, dataBuffer, sizeof(dataBuffer) - 1, MSG_DONTWAIT)) > 0) {
				receivedData.append(dataBuffer, bytesRead);
			}
			if (bytesRead <= 0) {
				disconnectClient(socket);
				return CLIENT_DISCONNECTED;
			}
			else {
				return processData(receivedData, socket);
			}
	}

	void	Server::sendData(int socket, const char* message, int messageSize) const {
			int result;
			if ((result = send(socket, message, messageSize, 0)) == -1)
				throw std::runtime_error("Can't send() message to the client: " + std::string(strerror(errno)));
	}

	
