# Unity Facilitator Makefile
# -------------------------------------

CC      = g++
DEFINES = -DUNITY_FACILITATOR
CFLAGS  = -Wall -lpthread $(DEFINES)
DEBUG   = -ggdb
INCLUDE = .
PROGRAMNAME = Facilitator
PROGRAMSOURCES = Facilitator.cpp

# -------------------------------------

RAKNET_INCLUDE = RakNet/Sources

RAKNET_SOURCES = \
$(RAKNET_INCLUDE)/WSAStartupSingleton.cpp\
$(RAKNET_INCLUDE)/RakNetSocket2.cpp\
$(RAKNET_INCLUDE)/HTTPConnection2.cpp\
$(RAKNET_INCLUDE)/SecureHandshake.cpp\
$(RAKNET_INCLUDE)/Gets.cpp\
$(RAKNET_INCLUDE)/RandSync.cpp\
$(RAKNET_INCLUDE)/RelayPlugin.cpp\
$(RAKNET_INCLUDE)/LocklessTypes.cpp\
$(RAKNET_INCLUDE)/Rackspace.cpp\
$(RAKNET_INCLUDE)/RakWString.cpp\
$(RAKNET_INCLUDE)/DR_SHA1.cpp\
$(RAKNET_INCLUDE)/TeamManager.cpp\
$(RAKNET_INCLUDE)/CloudServer.cpp\
$(RAKNET_INCLUDE)/DynDNS.cpp\
$(RAKNET_INCLUDE)/TwoWayAuthentication.cpp\
$(RAKNET_INCLUDE)/CloudCommon.cpp\
$(RAKNET_INCLUDE)/Base64Encoder.cpp\
$(RAKNET_INCLUDE)/CloudClient.cpp\
$(RAKNET_INCLUDE)/StatisticsHistory.cpp\
$(RAKNET_INCLUDE)/BitStream.cpp\
$(RAKNET_INCLUDE)/GetTime.cpp\
$(RAKNET_INCLUDE)/RakPeer.cpp\
$(RAKNET_INCLUDE)/RakSleep.cpp\
$(RAKNET_INCLUDE)/CheckSum.cpp\
$(RAKNET_INCLUDE)/Rand.cpp\
$(RAKNET_INCLUDE)/CCRakNetSlidingWindow.cpp\
$(RAKNET_INCLUDE)/ReliabilityLayer.cpp\
$(RAKNET_INCLUDE)/LinuxStrings.cpp\
$(RAKNET_INCLUDE)/CommandParserInterface.cpp\
$(RAKNET_INCLUDE)/ConsoleServer.cpp\
$(RAKNET_INCLUDE)/DS_BytePool.cpp\
$(RAKNET_INCLUDE)/MessageFilter.cpp\
$(RAKNET_INCLUDE)/DS_ByteQueue.cpp\
$(RAKNET_INCLUDE)/SimpleMutex.cpp\
$(RAKNET_INCLUDE)/DS_HuffmanEncodingTree.cpp\
$(RAKNET_INCLUDE)/NetworkIDManager.cpp\
$(RAKNET_INCLUDE)/SocketLayer.cpp\
$(RAKNET_INCLUDE)/DS_Table.cpp\
$(RAKNET_INCLUDE)/NetworkIDObject.cpp\
$(RAKNET_INCLUDE)/StringCompressor.cpp\
$(RAKNET_INCLUDE)/StringTable.cpp\
$(RAKNET_INCLUDE)/DataCompressor.cpp\
$(RAKNET_INCLUDE)/PacketFileLogger.cpp\
$(RAKNET_INCLUDE)/DirectoryDeltaTransfer.cpp\
$(RAKNET_INCLUDE)/PacketLogger.cpp\
$(RAKNET_INCLUDE)/TCPInterface.cpp\
$(RAKNET_INCLUDE)/EmailSender.cpp\
$(RAKNET_INCLUDE)/TableSerializer.cpp\
$(RAKNET_INCLUDE)/TelnetTransport.cpp\
$(RAKNET_INCLUDE)/RakNetCommandParser.cpp\
$(RAKNET_INCLUDE)/ThreadsafePacketLogger.cpp\
$(RAKNET_INCLUDE)/FileList.cpp\
$(RAKNET_INCLUDE)/RakNetStatistics.cpp\
$(RAKNET_INCLUDE)/_FindFirst.cpp\
$(RAKNET_INCLUDE)/FileListTransfer.cpp\
$(RAKNET_INCLUDE)/FileOperations.cpp\
$(RAKNET_INCLUDE)/RakNetTypes.cpp\
$(RAKNET_INCLUDE)/CCRakNetUDT.cpp\
$(RAKNET_INCLUDE)/RakNetSocket.cpp\
$(RAKNET_INCLUDE)/RakString.cpp\
$(RAKNET_INCLUDE)/RakMemoryOverride.cpp\
$(RAKNET_INCLUDE)/SignaledEvent.cpp\
$(RAKNET_INCLUDE)/SuperFastHash.cpp\
$(RAKNET_INCLUDE)/PluginInterface2.cpp\
$(RAKNET_INCLUDE)/Itoa.cpp\
$(RAKNET_INCLUDE)/RakThread.cpp\
$(RAKNET_INCLUDE)/NatPunchthroughServer.cpp

RAKNET_OBJECTS = $(RAKNET_SOURCES:.cpp=.o)

COMMON_INCLUDE = Common

COMMON_SOURCES = \
$(COMMON_INCLUDE)/Log.cpp\
$(COMMON_INCLUDE)/Utility.cpp

COMMON_OBJECTS = $(COMMON_SOURCES:.cpp=.o)

all: $(COMMON_SOURCES) $(RAKNET_SOURCES) $(PROGRAMNAME)

$(PROGRAMNAME): $(COMMON_OBJECTS) $(RAKNET_OBJECTS)
	$(CC) $(DEBUG) -I$(INCLUDE) -I$(RAKNET_INCLUDE) -I$(COMMON_INCLUDE) $(CFLAGS) $(COMMON_OBJECTS) $(RAKNET_OBJECTS) $(PROGRAMSOURCES) -o $(PROGRAMNAME) 
	chmod +x $(PROGRAMNAME)

clean:
	rm -f $(PROGRAMNAME)
	
cleanall:
	rm -f $(PROGRAMNAME)
	rm -f $(RAKNET_OBJECTS)
	rm -f $(COMMON_OBJECTS)
	
# Compile all RakNet cpp source files
.cpp.o:
	$(CC) -c -Wall -I$(INCLUDE) -I$(COMMON_INCLUDE) $(DEFINES) $< -o $@
