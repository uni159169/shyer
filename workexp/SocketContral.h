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
#include "../../TexasPokerHall/TexasPokerHallServer.h"

////������
enum ENUM_Main_Command{
	GLOBAL_CONTRAL_MAIL,	////Ⱥ���ʼ�
	GLOBAL_CONTRAL_ROBOT,	////�����˿���
	GLOBAL_CONTRAL_BROADCAST	////����㲥
};
////������ 
enum ENUM_Sub_Command{
	CUB_COMMAND_1,		//// Ⱥ���ʼ�
	CUB_COMMAND_2,		//// ��ͨ�������� ����
	CUB_COMMAND_3,		//// ������������ �Ѷ��޸�
	CUB_COMMAND_4,		//// ����
	CUB_COMMAND_5,		//// ������������Ϣ�޸�
	CUB_COMMAND_6,		////
	CUB_COMMAND_7,		////
	CUB_COMMAND_8,		////
	CUB_COMMAND_9,		////
	CUB_COMMAND_10,		////
	CUB_COMMAND_11,		////
	CUB_COMMAND_12,		////
	CUB_COMMAND_13,		////
	CUB_COMMAND_14,		////
	CUB_COMMAND_15,		////
	CUB_COMMAND_16,		////
	CUB_COMMAND_17		////
};


static unsigned int port;


class SocketContral{
public:

	static SocketContral* instance;
	static SocketContral* getinstance();

	SocketContral();
	~SocketContral();

	static DWORD WINAPI doListenFuction(LPVOID lpData);
	void domyCommand(GlobalContral su_sttInfo);
	int ParseCommand(const char* command);
	void StartMyServerThread();
	int doPurchaseFun(const char* order);
	static DWORD WINAPI ServerRun(LPVOID lpData);

	static DWORD WINAPI updatePoolRun(LPVOID lpData);

	////�����˿���
	void dorobotFunc(GlobalContral& su_sttInfo);

	////�޸���ͨ���䷿�����������
	void ModifyCommonRoomRobotConf(GlobalContral su_sttInfo);

	////�޸ı���������������Ϣ
	void ModifyMetchRoomConf(GlobalContral su_sttInfo);

	////�޸ĵ�����������Ϣ
	void ModifySingleRobotInfo(GlobalContral su_sttInfo);

	void initHserver(TexasPokerHallServer* pserver);

	unsigned int  GetdyDayOfYear();

protected:
	GlobalContral su_sttContralInfo;
	Server* m_pkServer;
	int su_timer1;
	void UpdateRobotInfo();
	static unsigned int su_dydays;

private:
	friend class TexasPokerHallServer;
	TexasPokerHallServer* m_HServer;

};


