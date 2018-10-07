// Include all of the headers required 
#include "echo_s.h"
// simple logger
#include "log.h"

// ***
// Class ClientConnection
// - created to help simply methods and functions used to create TCP connection
// 	for this program to work
class ClientConnection {
	public:
		void createSocket();
		void setupAddress();
		void bindSocket();
		void listenSocket();
		int  waitingConnection();
		int  processConnection(int connection);
	private:
		int sock = -1;						// file descriptor for the server
		int lst  = -1;						// file descriptor for the listener
		struct sockaddr_in
		{
			short     	family;				// address family
			u_short       	port;				// port number
			struct in_addr  sin_addr;			// internet address
			char 		sin_zero[8];
		};
		struct sockaddr_in servAddr;				// struct needed for the server addr
		struct sockaddr_in clientAddr;				// struct needed for the listener addr
		char buffer[256];	
		int portNum = -1;
		int n = 0;
};

Log* logger = new Log();

// ***
// main ()
// - sets up the socket and accepts new connection until CLOSE or QUIT
int main (int argc, char *argv[])
{
	// Process the command arguments 
	int opt = 0;
	bool reading = true;
	ClientConnection client;
	
	while ((opt = getopt(argc, argv, "v")) !=  -1) 
	{
		switch (opt) 
		{
			case 'v':
				logger->setLogger(true);
				logger->printLog("Verbose Action captured");
				break;
			case ':':
			case '?':
			default:
				std::cout<< "Invalid option: " << argv[0] << std::endl;
				exit(-1);
		}
	}

	// create the socket
	client.createSocket();
	// set up socket address
	client.setupAddress(); 
	// bind socket
	client.bindSocket();
	// listen socket
	client.listenSocket();
	// Wait for the connection with the accept call 
	int returnCode = client.waitingConnection();
	logger->printLog("exit: " + std::to_string(returnCode));
	
}

void ClientConnection::createSocket () 
{
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
		logger->printLog ("Error opening socket");
		exit (-1);
	}
	else 
	{
		logger->printLog("Socket was created");
		logger->printLog("Socket info: " + std::to_string(sock)); 
	}
}

void ClientConnection::setupAddress ()
{
	// define the struct
	srand(time(NULL));
	portNum = (rand() % 10000 + 1024);
	
	// zero the whole struct
	bzero((char *)&servAddr, sizeof(servAddr));

	// Fill in the struct with the information need for the address of the host
	servAddr.family = AF_INET;	
	servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.port = htons(portNum);
	logger->printLog("Address has been created for socket");
}

void ClientConnection::bindSocket ()
{
	int bindSuccess = 0;
	int attempts = 0;
	std::string errorString;
	if (bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
	{
		errorString = strerror(errno);
		logger->printLog("bind() failed: " + errorString);
		exit(-1);
	}
	else
	{
		logger->printLog("bind() successful");
		logger->printLog("Bind() function returned: " + std::to_string(sock));
		std::cout << "Port number: " << portNum << std::endl;
	}
}

void ClientConnection::listenSocket ()
{
	int listenSocket = 1;
	std::string errorString;
	if (listen(sock, listenSocket) < 0) 
	{
		errorString = strerror(errno);
		logger->printLog("listen() failed: " + errorString);
		exit(-1);
	}
	else
	{
		logger->printLog("listen() successful");
		logger->printLog("listen() fucntion returns: " + std::to_string(sock));
	}
}

int ClientConnection::waitingConnection ()
{
	logger->printLog("Attempting accept()");
	socklen_t sizeClient = sizeof(clientAddr);
	
	lst = accept(sock, (struct sockaddr *) NULL, NULL);
	if (lst < 0)
	{
		std::string errorString = strerror(errno);
		logger->printLog("Error with accept(): " + errorString);			
		exit(-1);
	}
	else
	{
		logger->printLog("accept() successful");
		return processConnection(lst);			// processes all of the strings read into the socket
	}		
}

int ClientConnection::processConnection(int connection)
{
	logger->printLog("Connection made");	
	while(1) {
		std::memset(buffer, 0, sizeof(buffer));
		n = 0;
		if ((n = read(connection, buffer, 255)) < 0) 
		{
			logger->printLog("Error reading data");
			exit(-1);
			return -2;				// error message that indicates read() did not work
		}		
		else
		{
			logger->printLog("read() successful");

			// double for loop created to detect the keyword: QUIT
			int foundQUIT = 0;
			for (int n = 0; n < sizeof(buffer); n++) 
			{
				for (int i = 0; i < std::strlen("QUIT"); i++)
				{
					if ( buffer[n] == "QUIT"[i])
					{
						foundQUIT++;
					}
				}
				// need to find all of the chars that make "QUIT"
				if (foundQUIT >= strlen("QUIT"))
				{
					// assignment says to return 1
					return 1;
				}
			}

			// double for loop created to detect the keyword: CLOSE
			int foundCLOSE = 0;
			for (int n =0; n < sizeof(buffer); n++)
			{
				for (int i = 0; i < std::strlen("CLOSE"); i++)
				{
					if (buffer[n] == "CLOSE"[i])
					{
						foundCLOSE++;
					}
				}
				// need to find all of the chars that make "CLOSE"
				if (foundCLOSE >= strlen("CLOSE"))
				{
					// assignment says to return 0
					return 0;
				}
			}
			n = write(connection, buffer, sizeof(buffer));
			logger->printLog("write() successful");
		}
	}
	// sucessful calls for read() and write(); no keywords detected
	return -1;
}
