#ifndef SBCICD_H
#define SBCICD_H
#include "MemoryAccess.h"
#include <iostream>
#include <string.h>
#include "Logger.h"
#include "Config.h"
#include "KratosProtocol.h"
#include "UartController.h"

class SbcICD {
public:
 //  static TxFrame responseFrame;
   static Frame SBCIcdRun(Frame receivedFrame);


private:
   static Frame TimeOutError();
   static Frame dataError();
   static Frame datalengthError();
   static Frame commandError(Frame receivedFrame);

};

#endif // SBCICD_H
