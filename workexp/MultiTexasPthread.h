#pragma once

#include "General.h"
#include "DBBusiness.h"
#include "EntityS.h"
#include "../../TexasPokerHall/TexasPokerHallAgentS.h"
#include "../../TexasPokerHall/TexasPokerHallPlayerS.h"
#include "../../TexasPokerHall/TexasPokerTable.h"
#include "Server.h"
#include "../../TexasPokerHall/Global.h"
#include "su_md5.h"
class MultiTexasPthread
{
public:
	MultiTexasPthread(void);
	~MultiTexasPthread(void);

	static MultiTexasPthread* instance;
	static MultiTexasPthread* getInstance();

	void doMyPThread();
	static void su_initcurl();
	void ServerInit(su_MultiClientInfo *su_sttMultiInfo);
	static DWORD WINAPI MultiThread(LPVOID lpData);

	static int doMultiFunCodeFun(void* su_sttCodeInfo);////ÑéÖ¤Âë
	static int socketcount;
};