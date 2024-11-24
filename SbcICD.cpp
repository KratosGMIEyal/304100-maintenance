#include "SbcICD.h"
#include "Logger.h"
#include <termios.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include "BitManagement.h"
#include <vector>
#include <algorithm>

#ifdef DEBUGBIT
uint32_t pbit = 0;
#endif
uint32_t status = 0;
Frame responseFrame;
uint32_t value_32 = 0;
bool statusFrame = 0;

TCPClient client0(RFSOM_ip, RFSOM_port);
UartController uart0("/dev/ttyS1", B115200);
inline std::vector<uint16_t> McuDelayCommands = {0x211,0x202,0x206,0x204,0x229,0x218,0x22c,0x22b};
BitManagement SBClocalbit (UDS_BIT_PATH,CONFIG_FILE);

KratosProtocol RFSOM(&client0); // Use TCP client for communication
KratosProtocol MCU(&uart0); // Use uart for communication

Frame SbcICD::SBCIcdRun(Frame receivedFrame) {
    struct timeval tv;
    fd_set readfds;
 	int sel;
    // Set up the timeout (5 seconds)
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    
	int uart_fd = uart0.getFileDescriptor();  // Assume UartController has a method to get the file descriptor
    
	if (uart_fd < 0) {
        Logger::log(Logger::Level::ERROR, "Invalid UART file descriptor");
        return dataError();
    }

    // Set UART to non-blocking mode
    int flags = fcntl(uart_fd, F_GETFL, 0);
    fcntl(uart_fd, F_SETFL, flags | O_NONBLOCK);

 //--------RFSOM-STATE-------------//
    if (receivedFrame.command >= 0x300 && receivedFrame.command <= 0x3ff) {
        switch (receivedFrame.command) {
            default:
                Logger::log(Logger::Level::DEBUG, "Send Opcode %04x To RFSOM ", receivedFrame.command);
                receivedFrame.rawOrPacket = PACKET;
                RFSOM.sendFrame(receivedFrame, client0.clientSocket);

                FD_SET(client0.clientSocket, &readfds);
                sel = select(client0.clientSocket + 1, &readfds, NULL, NULL, &tv);

                if (sel == 0) {  // Timeout
                    Logger::log(Logger::Level::ERROR, "Timeout occurred while waiting for response from RFSOM");
                    return TimeOutError();
                } else if (sel > 0) {  // Data available to read
                    for (size_t i = 0; i < RFSOM_RETRY; i++)                  
                    {
                        statusFrame = RFSOM.recvFrame(client0.clientSocket, responseFrame);
                        if (!statusFrame | responseFrame.command != receivedFrame.command) {
                            if (i == RFSOM_RETRY -1)
                            {
                                return TimeOutError();
                            }
                            Logger::log(Logger::Level::ERROR, "RFSOM retry num : %d , responseFrame.command : %04x , receivedFrame.command : %04x" , i ,responseFrame.command,receivedFrame.command);
                            client0.Close();
                            client0.openClient();
                            RFSOM.sendFrame(receivedFrame, client0.clientSocket);
                            continue;
                        }
                        break;
                    }
                    
                    
                } else {  // Error in select
                    Logger::log(Logger::Level::ERROR, "Select error on RFSOM socket");
                    return dataError();
                }

                responseFrame.rawOrPacket = PACKET;
                break;
        }
    }
    //--------SBC-STATE-------------//
    else if (receivedFrame.command >= 0x100 && receivedFrame.command <= 0x1ff) {
        switch (receivedFrame.command) {
            case 0x0101: // Get software version
                if (receivedFrame.dataLength == 0) {
                    responseFrame.dataLength = 0x00000006;
                    responseFrame.rawOrPacket = PACKET;
					//responseFrame.data.resize(sizeof(Version));
                    memcpy(responseFrame.data.data(), &version, sizeof(Version));
                    Logger::log(Logger::Level::DEBUG, "Send the Version of SBC : V%d.%d.%d Date : %d/%d/%d",
                                version.major, (version.minor) / 10, version.minor, version.day, version.month, version.year);
                }
                break;

			 case 0x0102: // Get PBIT PASS/FAIL
                if (receivedFrame.dataLength == 0) {
                    
                    responseFrame.dataLength = 0x00000003;
                    //responseFrame.data.resize(responseFrame.dataLength );
                    responseFrame.data.clear();
                    // Copy the 4-byte array into responseFrame.data
					SBClocalbit.printHandleResponse(SBClocalbit.readPBITResults());
					value_32 = SBClocalbit.convertToBitResults(SBClocalbit.readPBITResults());
					#ifdef DEBUGBIT
					value_32 = pbit;
					#endif
                    
                    uint8_t value_bytes[sizeof(value_32)];
                    std::memcpy(value_bytes, &value_32, sizeof(value_32));
                    responseFrame.data.insert(responseFrame.data.end(), value_bytes, value_bytes + sizeof(value_32));
                    responseFrame.data.push_back(0);
                    responseFrame.data.push_back(0);
                    responseFrame.data.resize(5000000);
                    Logger::log(Logger::Level::DEBUG,"%08x",value_32);
                    Logger::log(Logger::Level::ERROR, "sData receivedFrame pbit is : %d ",receivedFrame.data.size());
                    Logger::log(Logger::Level::ERROR, "Data responseFrame pbit is : %d ",responseFrame.data.size());
                    Logger::log(Logger::Level::DEBUG, "Get local PBit ");


                }
                break;
			 case 0x0104: // Get cBIT PASS/FAIL
                if (receivedFrame.dataLength == 0) {
                    responseFrame.dataLength = 0x00000003;
                    //responseFrame.data.resize(responseFrame.dataLength );
                    responseFrame.data.clear();
                    // Copy the 4-byte array into responseFrame.data
					value_32 = SBClocalbit.convertToBitResults(SBClocalbit.readLatestResults())|  SBClocalbit.convertToBitResults(SBClocalbit.readPBITResults());
					#ifdef DEBUGBIT
					value_32 = pbit;
					#endif
                    
                    uint8_t value_bytes[sizeof(value_32)];
                    std::memcpy(value_bytes, &value_32, sizeof(value_32));

                    responseFrame.data.insert(responseFrame.data.end(), value_bytes, value_bytes + sizeof(value_32));
                    responseFrame.data.push_back(0);
                    responseFrame.data.push_back(0);
                    responseFrame.data.resize(5000000);
                    Logger::log(Logger::Level::DEBUG,"%08x",value_32);
                    Logger::log(Logger::Level::DEBUG, "Get local PBit ");


                }
                break;

			case 0x0103: // Get cbit 
                if (receivedFrame.dataLength == 0) {
                    responseFrame.dataLength = 0x00000012;
                    //responseFrame.data.resize(responseFrame.dataLength);
                    responseFrame.data.clear();
                    // Copy the 4-byte array into responseFrame.data
					std::vector<uint8_t> temp = SBClocalbit.CbitHandleResponse(SBClocalbit.readLatestResults());
                    responseFrame.data = temp;

                    responseFrame.data.resize(5000000);
                    Logger::log(Logger::Level::DEBUG, "Get local CBit ");


                }
                break;
		#ifdef DEBUGBIT	
			case 0x01ff: // change pbit 
                if (receivedFrame.dataLength == 3) {
                    responseFrame.dataLength = 0x0000000;
					memcpy(&value_32,receivedFrame.data.data(),3);
					pbit = value_32;
                    Logger::log(Logger::Level::DEBUG, "change local PBit ");


                }
                break;
		#endif			
            default:
                responseFrame = commandError(receivedFrame);
                break;
        }
		responseFrame.command = receivedFrame.command;
    }

    
   else if (receivedFrame.command >= 0x200 && receivedFrame.command <= 0x2ff) {
        switch (receivedFrame.command) {
            default:
                Logger::log(Logger::Level::DEBUG, "Send Opcode %04x To MCU ", receivedFrame.command);
                receivedFrame.rawOrPacket = PACKET;
                uart0.flush();
                MCU.sendFrame(receivedFrame, uart_fd);

                FD_SET(uart_fd, &readfds);
               if (find(McuDelayCommands.begin(), McuDelayCommands.end(), receivedFrame.command) != McuDelayCommands.end()) 
                {
                    usleep(1000000);
                }
                else{
                    usleep(200000);
                }
                sel = select(uart_fd + 1, &readfds, NULL, NULL, &tv);
				
                if (sel == 0) {  // Timeout
                    Logger::log(Logger::Level::ERROR, "Timeout occurred while waiting for response from MCU");
                    return TimeOutError();
                } else if (sel > 0 && FD_ISSET(uart_fd, &readfds)) {  // Data available to read
                    if (!MCU.recvFrame(uart_fd, responseFrame)) {
                        return datalengthError();
                    }
                } else {  // Error in select or no data
                    Logger::log(Logger::Level::ERROR, "Select error or no data from MCU");
                    return dataError();
                }

                responseFrame.rawOrPacket = PACKET;
                break;
        }
    }

    return responseFrame;
}


Frame SbcICD::dataError() {
	Frame errorFrame;

	errorFrame.preamble = 0x0054;
	errorFrame.command = 0x00F3;
	errorFrame.dataLength = 0x00000000;
	Logger::log(Logger::Level::ERROR, "Data error get from client ");
	errorFrame.rawOrPacket = PACKET;
	return errorFrame;

}

Frame SbcICD::datalengthError() {
	Frame errorFrame;

	errorFrame.preamble = 0x0054;
	errorFrame.command = 0x00F9;
	errorFrame.dataLength = 0x00000000;
	errorFrame.rawOrPacket = PACKET;
	Logger::log(Logger::Level::ERROR, "Data length error get from client ");
	return errorFrame;

}

Frame SbcICD::TimeOutError() {
	Frame errorFrame;

	errorFrame.preamble = 0x0054;
	errorFrame.command = 0x00F5;
	errorFrame.dataLength = 0x00000000;
	errorFrame.rawOrPacket = PACKET;
	Logger::log(Logger::Level::ERROR, "TimeOut error get from other unit ");
	return errorFrame;

}

Frame SbcICD::commandError(Frame receivedFrame) {
	Frame errorFrame;

	errorFrame.preamble = 0x0054;
	errorFrame.command = 0x00F1;
	errorFrame.dataLength = 0x00000000;
	errorFrame.rawOrPacket = PACKET;
	Logger::log(Logger::Level::ERROR, "The command %04x is not supported",
			receivedFrame.command);
	return errorFrame;

}
