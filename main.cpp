
#include "KratosProtocol.h"
#include "TCPServer.h"
#include "SbcICD.h"
#include <iostream>
#include <asm-generic/termbits.h>
#include <unistd.h>
#include "MemoryAccess.h"
#include "Logger.h"
#include "Config.h"

void printVersion();

int main() {
    // Create TCP server, client, or UART controller objects
    TCPServer server(TCPServer::getIPAddress(NETWORK_INTERFACE), port);
    // Create KratosProtocol object using the server, client, or UART controller
    KratosProtocol SBC(&server); // Use TCP server for communication
    Frame Txframe;
    Frame receivedFrame;
	Logger::initialize(LOG_LEVEL); // Initialize the logger
    printVersion();
    int clientSocket = server.acceptConnection();
    while (true) {
        if(!SBC.recvFrame(clientSocket,receivedFrame))
        {
            server.Close(clientSocket);
            clientSocket = server.acceptConnection();
            continue;
        }
        Txframe = SbcICD::SBCIcdRun(receivedFrame);
        SBC.sendFrame(Txframe,clientSocket);
        }

    server.stop();
    return 0;
  

    
}
void printVersion(){
    Logger::log(Logger::Level::INFO,
						"Version of SBC : V%d.%d.%d Date : %d/%d/%d",
						version.major, (version.minor) / 10, version.minor,
						version.day, version.month, version.year);
#ifdef DEBUGBIT	
    Logger::log(Logger::Level::DEBUG, "Debug bit version is on");
#endif

}