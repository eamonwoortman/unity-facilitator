#include "Utility.h"
#include "Log.h"
#include "TableSerializer.h"
#include "BitStream.h"
#include "StringCompressor.h"
#include "DS_Table.h"
#include "RakPeerInterface.h"
#include "RakSleep.h"
#include "MessageIdentifiers.h"
#include "NatPunchthroughServer.h"
#include <signal.h>
#include "SocketLayer.h"
#include <stdarg.h>

#ifdef WIN32
#include <stdio.h>
#include <time.h>
#include <windows.h>
#else
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#endif

using namespace RakNet;

RakPeerInterface *rakPeer;
NatPunchthroughServer *natPunchthrough;
bool quit;
bool printStats = false;

char* logfile = "facilitator.log";
const int fileBufSize = 1024;
char pidFile[fileBufSize];


void shutdown(int sig)
{
	Log::print_log("Shutting down\n\n");
	quit = true;
}


void cleanup()
{
	if (pidFile)
	{
		if (remove(pidFile) != 0)
			fprintf(stderr, "Failed to remove PID file at %s\n", pidFile);
	}
	rakPeer->Shutdown(100, 0);

	if (natPunchthrough)
	{
		rakPeer->DetachPlugin(natPunchthrough);
		delete natPunchthrough;
	}
	natPunchthrough = NULL;

	RakPeerInterface::DestroyInstance(rakPeer);
	Log::print_log("Quitting\n");
}


void usage()
{
	printf("\nAccepted parameters are:\n\t"
		   "-p\tListen port (1-65535)\n\t"
		   "-d\tDaemon mode, run in the background\n\t"
		   "-l\tUse given log file\n\t"
		   "-e\tDebug level (0=OnlyErrors, 1=Warnings, 2=Informational(default), 2=FullDebug)\n\t"
		   "-c\tMax connection count. [1000]\n\t"
		   "-s\tStatistics print delay (in seconds). [0]\n\t"
		   "-h\tBind to listen address\n\t"
		   "-b\tBind a second (external) addresses. When using 2 IP addresses, NATPunchthroughServer supports port stride detection, improving success rate\n\n"
		   "If any parameter is omitted the default value is used.\n");
}

void ProcessPacket(Packet *packet)
{
	switch (packet->data[0])
	{
		case ID_DISCONNECTION_NOTIFICATION:
			Log::info_log("%s has diconnected\n", packet->systemAddress.ToString());
			break;
		case ID_CONNECTION_LOST:
			Log::warn_log("Connection to %s lost\n", packet->systemAddress.ToString());
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			Log::warn_log("No free incoming connection for %s\n", packet->systemAddress.ToString());
			break;
		case ID_NEW_INCOMING_CONNECTION:
			Log::info_log("New connection established to %s (%s)\n", packet->systemAddress.ToString(), packet->guid.ToString());
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			Log::info_log("Connection to %s accepted\n", packet->systemAddress.ToString());
			break;
		case ID_CONNECTION_ATTEMPT_FAILED:
			Log::warn_log("Connection attempt to %s failed\n", packet->systemAddress.ToString());
			break;
		case ID_NAT_TARGET_NOT_CONNECTED:
		{
			SystemAddress systemAddress;
			RakNet::BitStream b(packet->data, packet->length, false);
			b.IgnoreBits(8); // Ignore the ID_...
			b.Read(systemAddress);
			Log::warn_log("ID_NAT_TARGET_NOT_CONNECTED to %s\n", systemAddress.ToString());
			break;
		}
		case ID_NAT_CONNECTION_TO_TARGET_LOST:
		{
			SystemAddress systemAddress;
			RakNet::BitStream b(packet->data, packet->length, false);
			b.IgnoreBits(8); // Ignore the ID_...
			b.Read(systemAddress);
			Log::warn_log("ID_NAT_CONNECTION_TO_TARGET_LOST to %s\n", systemAddress.ToString());
			break;
		}
		default:
			Log::error_log("Unknown ID %d from %s\n", packet->data[0], packet->systemAddress.ToString());
	}
}

bool validate_ip(const char* ip)
{
	bool valid = true;
	int i = 0;
	int dotCount = 0;

	if (ip == NULL) {
		return false;
	}

	while (ip[i] != 0)
	{
		if (ip[i++] == '.')
			dotCount++;
	}
	if (dotCount != 3)
		valid = false;

#ifdef WIN32
	unsigned long ulAddr = INADDR_NONE;
	ulAddr = inet_addr(ip);
	if (valid && ulAddr != INADDR_NONE)
		return true;
#else
	struct in_addr ia;
	if (valid && inet_aton(ip, &ia) == 1)
		return true;
#endif
	printf("IP address '%s' is invalid\n", ip);
	return false;
}

int main(int argc, char *argv[])
{  
#ifndef WIN32
	setlinebuf(stdout);
#endif

	rakPeer = RakPeerInterface::GetInstance();	// The facilitator
	natPunchthrough = new NatPunchthroughServer;

	// Default values
	int facilitatorPort = 50005;
	int connectionCount = 1000;

	time_t timerInterval = 10;	// 10 seconds
	time_t rotateCheckTimer = time(0) + timerInterval;
	int rotateSizeLimit = 5000000;	// 5 MB
	bool useLogFile = false;
	int statDelay = 30;
	bool daemonMode = false;
	const char *bindToIP1 = NULL;
	const char *bindToIP2 = NULL;

	// Process command line arguments
	for (int i = 1; i < argc; i++)
	{
		if (strlen(argv[i]) == 2 && argc>=i+1)
		{
			switch (argv[i][1]) 
			{
				case 'd':
				{
					daemonMode = true;
					break;
				}
				case 'p':
				{
					facilitatorPort = atoi(argv[i+1]);
					i++;
					if (facilitatorPort < 1 || facilitatorPort > 65535)
					{
						fprintf(stderr, "Facilitator port is invalid, should be between 0 and 65535.\n");
						return 1;
					}
					break;
				}
				case 'c':
				{
					connectionCount = atoi(argv[i+1]);
					i++;
					if (connectionCount < 0)
					{
						fprintf(stderr, "Connection count must be higher than 0.\n");
						return 1;
					}
					break;
				}
				case 'l':
				{
					useLogFile = Log::EnableFileLogging(logfile);
					break;
				}
				case 'e':
				{
					int debugLevel = atoi(argv[i+1]);
					Log::sDebugLevel = debugLevel;
					i++;
					if (debugLevel < 0 || debugLevel > 9)
					{
						fprintf(stderr, "Log level can be 0(errors), 1(warnings), 2(informational), 9(debug)\n");
						return 1;
					}
					break;
				}
				case 's':
				{
					int statDelay =  atoi(argv[i+1]);
					i++;
					if (statDelay < 0)
					{
						fprintf(stderr, "Statistics print delay must not be lower than 0. Use 0 to disable.\n");
						return 1;
					}
					Log::print_log("NOT IMPLEMENTED");
					printStats = true;
					break;
				}
				case 'h':
				{
					bindToIP1 = argv[i + 1];
					i++;
					if (!validate_ip(bindToIP1))
					{
						fprintf(stderr, "Invalid IP address given in -h parameter.\n");
						return 1;
					}
					break;
				}
				case 'b':
				{
					if (argc - i < 2)
					{
						fprintf(stderr, "You must provide an IP address with -b parameter.\n");
						return 1;
					}

					bindToIP2 = argv[i + 1];
					
					if (!validate_ip(bindToIP2))
					{
						fprintf(stderr, "Invalid IP address with -b paramter. Must be in dot notation.\n");
						return 1;
					}
							
					i = i + 1;
					break;
				}
				case '?':
				{
					usage();
					return 0;
				}
				default:
					fprintf(stderr, "Parsing error, unknown parameter '-%c'\n\n", argv[i][1]);
					usage();
					return 1;
			}
		}
		else
		{
			printf("Parsing error, incorrect parameters\n\n");
			usage();
			return 1;
		}
	}
#ifndef WIN32
	if (daemonMode)
	{
		printf("Running in daemon mode, file logging enabled...\n");
		if (!useLogFile)
			useLogFile = Log::EnableFileLogging(logfile);
		// Don't change cwd to /
		// Beware that log/pid files are placed wherever this was launched from
		daemon(1, 0);
	}
	
	if (!WriteProcessID(argv[0], &pidFile[0], fileBufSize))
		perror("Warning, failed to write own PID value to PID file\n");
#endif

	rakPeer->SetMaximumIncomingConnections(connectionCount);

	SystemAddress ipList[MAXIMUM_NUMBER_OF_INTERNAL_IDS];
	unsigned int i;
	for (i = 0; i < MAXIMUM_NUMBER_OF_INTERNAL_IDS; i++)
	{
		const char* localIP = rakPeer->GetLocalIP(i);
		ipList[i] = localIP;
		ipList[i] = rakPeer->GetLocalIP(i);
		if (strcmp(localIP, UNASSIGNED_SYSTEM_ADDRESS.ToString(false)) != 0)
			printf("%i. %s\n", i + 1, localIP);
		else
			break;
	}

	if (i < 1)
	{
		Log::error_log("Not enough IP addresses to bind to.\n");
		Log::error_log("To run the facilitator you need at least one public IP addresses available.\n\n");
		cleanup();
		return 1;
	}

	if (bindToIP1 == NULL)
		bindToIP1 = ipList[0].ToString(false);

	if (bindToIP2 == NULL) 
		bindToIP2 = ipList[1].ToString(false);

	// If RakPeer is started on 2 IP addresses, NATPunchthroughServer supports port stride detection, improving success rate
	int sdLen = 1;
	SocketDescriptor sd[2];

	sd[0].port = facilitatorPort;
	strcpy(sd[0].hostAddress, bindToIP1);

	if (bindToIP2 != NULL)
	{
		sd[1].port = sd[0].port + 1;
		strcpy(sd[1].hostAddress, bindToIP2);
		sdLen = 2;
	}
	
	if (rakPeer->Startup(8096, sd, sdLen) != RakNet::RAKNET_STARTED)
	{
		Log::error_log("Failed to start RakPeer!\n");
		cleanup();
		return 1;
	}
	rakPeer->SetTimeoutTime(5000, UNASSIGNED_SYSTEM_ADDRESS);
	rakPeer->AttachPlugin(natPunchthrough);

	if (bindToIP2) {
		printf("Facilitator started on %s:%d and %s:%d\n", bindToIP1, sd[0].port, bindToIP2, sd[1].port);
	}
	else {
		printf("Started on %s:%d\n", bindToIP1, sd[0].port);
	}
	printf("\n");

	Log::print_log("Unity Facilitator version 2.0.0\n");
	Log::print_log("Listen port set to %d\n",facilitatorPort);
	Log::print_log("%d connection count limit\n", connectionCount);
	if (printStats)
		Log::print_log("%d sec delay between statistics print to log\n", statDelay);
	
	// Register signal handlers
#ifndef WIN32
	if (signal(SIGHUP, Log::RotateLogFile) == SIG_ERR)
		Log::error_log("Problem setting up hangup signal handler");
	if (signal(SIGINT, shutdown) == SIG_ERR || signal(SIGTERM, shutdown) == SIG_ERR)
		Log::error_log("Problem setting up terminate signal handler");
	else
#endif
		Log::print_log("To quit press Ctrl-C\n----------------------------------------------------\n");

	Packet *p;
	while (!quit)
	{
		p=rakPeer->Receive();
		while (p)
		{
			ProcessPacket(p);
			rakPeer->DeallocatePacket(p);
			p=rakPeer->Receive();
		}
		// Is it time to rotate the logs?
		if (useLogFile)
		{
			if (rotateCheckTimer < time(0)) {
				// We should always be writing to the end of the stream, so the current position should give the total size
				int position = Log::GetLogSize();
				if (position > rotateSizeLimit)
				{
					Log::error_log("Rotating logs, size limit reached (cur %d, limit %d)\n", position, rotateSizeLimit);
					Log::RotateLogFile(0);
				}
				rotateCheckTimer = time(0) + timerInterval;
			}
		}
		RakSleep(30);
	}

	if (pidFile)
	{
		if (remove(pidFile) != 0)
			fprintf(stderr, "Failed to remove PID file at %s\n", pidFile);
	}

	cleanup();
				
	return 0;
}
