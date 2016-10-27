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

////主命令
enum ENUM_Main_Command{
	GLOBAL_CONTRAL_MAIL,	////群发邮件
	GLOBAL_CONTRAL_ROBOT,	////机器人控制
	GLOBAL_CONTRAL_BROADCAST	////公告广播
};
////辅命令 
enum ENUM_Sub_Command{
	CUB_COMMAND_1,		//// 群发邮件
	CUB_COMMAND_2,		//// 普通场机器人 数据
	CUB_COMMAND_3,		//// 竞技场机器人 难度修改
	CUB_COMMAND_4,		//// 公告
	CUB_COMMAND_5,		//// 单个机器人信息修改
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

	////机器人控制
	void dorobotFunc(GlobalContral& su_sttInfo);

	////修改普通房间房间机器人配置
	void ModifyCommonRoomRobotConf(GlobalContral su_sttInfo);

	////修改比赛场比赛配置信息
	void ModifyMetchRoomConf(GlobalContral su_sttInfo);

	////修改单个机器人信息
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


