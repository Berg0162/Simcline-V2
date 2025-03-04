#ifndef SERVERSIDEXP_H
#define SERVERSIDEXP_H

#include <ServerSide.h>

class ServerSideXP : public ServerSide {
private:
ServerSideXP();
static ServerSideXP* instance;  // Singleton class instance

public:
~ServerSideXP() override;
static ServerSideXP* getInstance();

void start(void);
void serverConnectionCallbacksOnConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override;
void serverConnectionCallbacksOnDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override;

}; // Class ServerSideXP

#endif // SERVERSIDEXP_H