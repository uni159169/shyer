#pragma once

#include "General.h"
#include "DBBusiness.h"
#include "EntityS.h"
#include "../../TexasPokerHall/TexasPokerHallAgentS.h"
#include "../../TexasPokerHall/TexasPokerHallPlayerS.h"
#include "../../TexasPokerHall/TexasPokerTable.h"
#include "Server.h"
#include "../../TexasPokerHall/Global.h"
class IappHttpListen
{
public:
	IappHttpListen(void);
	~IappHttpListen(void);

	static IappHttpListen* instance;
	static IappHttpListen* getInstance();

	void doMyListenThread();

	static DWORD WINAPI ServerInit(LPVOID lpData);
	static DWORD WINAPI OrderThread(LPVOID lpData);

	static int doPurchaseFun(const char* order);
};

