#include <Winsock2.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "SocketContral.h"
#include <json/json.h>
#include "../../TexasPokerHall/TexasPokerHallAgentS.h"
#include "../../TexasPokerHall/TexasPokerHallPlayerS.h"
#include "../../TexasPokerHall/TexasPokerTable.h"


#pragma comment(lib,"ws2_32.lib")

#define ServerListenPort 8190////定义服务端监听端口

#define MaxThreadNum 64  ////最大并发连接数;
static SOCKET ServerThreadSocket;	 

#define SYSMAIL_CMD 101    ////系统邮件指令

#define SIGN_KEY "x$dh%GxNVwwyXOjWGPiPGejd&!3DT7OeL$Z26fu77#v@wjHC1grkme^MYvM0E3n$"

SocketContral* SocketContral::instance = NULL;

SocketContral::SocketContral()
{

};
SocketContral::~SocketContral()
{

}
SocketContral* SocketContral::getinstance(){

	if (instance == NULL)
	{	
		instance = new SocketContral();
		instance->StartMyServerThread();
	}
	return instance;
}
unsigned int SocketContral::su_dydays = 0;

void CALLBACK TimerProc(HWND   hWnd,UINT   nMsg,UINT   nTimerid,DWORD   dwTime);

std::vector<std::string> su_strSplit(std::string str,std::string pattern)
{
	std::string::size_type pos;
	std::vector<std::string> result;
	str+=pattern;//扩展字符串以方便操作
	int su_intsize=str.size();

	for(int i=0; i<su_intsize; i++)
	{
		pos=str.find(pattern,i);
		if(pos<su_intsize)
		{
			std::string s=str.substr(i,pos-i);
			result.push_back(s);
			i=pos+pattern.size()-1;
		}
	}
	return result;
}

void SocketControl_mlog(std::string str)
{

	FILE *stream; 
	str  = str +"\n";
	char vcbuf[512]; 
	memset(vcbuf,0,sizeof(vcbuf));
	strcpy(vcbuf,str.c_str());
	int i=0, numread=0, numwritten; 

	if( (stream = fopen( "c:/serverRcv.txt", "ab+" )) != NULL ) 
	{ 
		/* Write 25 characters to stream */ 
		numwritten = fwrite( vcbuf, str.length() ,1,stream ); 
		printf( "Write %d items\n", numwritten ); 

	} 

	fclose( stream );
}


void SocketContral::domyCommand(GlobalContral su_sttInfo)
{
	ENUM_Main_Command nowType = ENUM_Main_Command(su_sttInfo.MainCommand);
	switch(nowType){
		case GLOBAL_CONTRAL_MAIL : 
			
			break;
		case GLOBAL_CONTRAL_ROBOT : 
			dorobotFunc(su_sttInfo);
			break;
		case GLOBAL_CONTRAL_BROADCAST : 

			break;
		
		default:

			break;
	}

	

	//m_pkServer->SendSysMailToOnline();	

}


int SocketContral::doPurchaseFun(const char* order)
{


	//SocketControl_mlog(order);

	//定义变量存储订单json
	std::string cporderid;
	std::string transid; 
	std::string appuserid ;
	std::string appid ; 
	std::string currency ;
	std::string transtime ; 
	std::string cpprivate ;


	int transtype , waresid , result , feetype , paytype ;
	double money;

	Json::Reader reader;  
	Json::Value root;  
	////解析json
	if ( reader.parse( order, root))	
	{  

		transtype = root["transtype"].asInt();
		cporderid = root["cporderid"].asString(); 
		transid = root["transid"].asString(); 
		appuserid = root["appuserid"].asString(); 
		appid = root["appid"].asString(); 
		waresid = root["waresid"].asInt();
		feetype = root["feetype"].asInt();
		money = root["money"].asDouble();
		currency = root["currency"].asString();
		result = root["result"].asInt();
		transtime = root["transtime"].asString(); 
		cpprivate = root["cpprivate"].asString(); 
		paytype = root["paytype"].asInt();

	}
	DbIapppaydata Iapp;
	strcpy(Iapp.appid,appid.c_str());
	strcpy(Iapp.appuserid,appuserid.c_str());
	strcpy(Iapp.cporderid,cporderid.c_str());
	strcpy(Iapp.cpprivate,cpprivate.c_str());
	strcpy(Iapp.currency,currency.c_str());
	strcpy(Iapp.transtime,transtime.c_str());
	strcpy(Iapp.transid,transid.c_str());
	Iapp.transtype = transtype;
	Iapp.waresid = waresid;
	Iapp.feetype = feetype;
	Iapp.result = result;
	Iapp.paytype = paytype;
	Iapp.money = money;


	int payresult = -1;////支付处理
	VEString veuuid = appuserid.c_str();
	bool userisonline = false;
	std::map<VeUInt32,ClientAgent*> mapclient = m_pkServer->getClientIDMap();
	for (std::map<VeUInt32,ClientAgent*>::iterator it = mapclient.begin();it != mapclient.end();++it)
	{
		ClientAgent* pkAgent = (*it).second;
		if(0 == (pkAgent->UUID).StrCmp(veuuid))
		{
			EntityS* pkEnt = pkAgent->GetEntity("Player");
			TexasPokerHallPlayerS* pkPlayer = static_cast<TexasPokerHallPlayerS*>(pkEnt);
			if(pkAgent && 0 == result)
			{
				payresult = pkPlayer->FinishIapppay(appuserid.c_str(),&Iapp,result);			
				userisonline = true;
			}
			if(pkAgent && 1 == result)
			{
				payresult = pkPlayer->FinishIapppay(appuserid.c_str(),&Iapp,result);			
				userisonline = true;
			}
		}
	}

	if( false == userisonline )
	{
		m_pkServer->serverFinishIapppay(appuserid.c_str(),&Iapp,result);
	}

	return payresult;
}


int SocketContral::ParseCommand(const char* command)
{
	unsigned int result = -1;
	unsigned int ciscmd = 0;
	printf("receive data from Client is : %s\n",command);
	
	char recbuf[512]={0};
	char cisdig[8]={0};
	strcpy(recbuf,command);
	
	char* tmpMainCmd = "Main_Command";
	char* tmpSubCmd = "Sub_Command";
	char* tmpMainInfo = "Main_Info";
	char* tmpSubInfo = "Sub_Info";
	char* tmppwd = "SIGN";
	////判断是不是命令
	char* MainCommand = strstr(recbuf,"Main_Command");
	char* SubCommand = strstr(recbuf,"Sub_Command");
	char* q = cisdig;

	char tmp[16]={0};
	int MainCmd;
	int SubCmd;
	std::string MainInfo;
	std::string SubInfo;
	
	std::string tmpstrMcmd;
	std::string tmpstrScmd;
	std::string CheckSign;
	std::string SignStr;
	bool scheck=false;

	//ifstream ifs;
	//ifs.open("testjson.json");
	//assert(ifs.is_open());

	//Json::Reader reader;
	//Json::Value root;
	//if (!reader.parse(ifs, root, false))
	//{
	//	return -1;
	//}

	if(NULL == MainCommand || NULL == SubCommand)
	{
		////不是命令，不做处理
	}else
	{
		scheck = true;
		typedef Json::Writer JsonWriter;  
		typedef Json::Reader JsonReader;  
		typedef Json::Value  JsonValue;  

		JsonReader reader;  
		JsonValue jsvalue;  
		if (!reader.parse(recbuf, jsvalue))  
		{  
			return 0;  
		}  
		char tmp[16]={0};

		JsonValue::Members mem = jsvalue.getMemberNames();  
		for (auto iter = mem.begin(); iter != mem.end(); iter++)  
		{  
			std::string mykey;
			std::string myvalue;
			std::cout << *iter << "\t: "<<std::endl;  
			if( !strcmp( iter->c_str(),"SIGN"))
			{
				SignStr = jsvalue[*iter].asString();
				continue;
			}
			if (jsvalue[*iter].type() == Json::objectValue)  
			{  
				//print_json(data[*iter]);  
			}  
			else if (jsvalue[*iter].type() == Json::arrayValue)  
			{  
				
				auto cnt = jsvalue[*iter].size();  
				for (auto i = 0; i < cnt; i++)  
				{  
					//print_json(data[*iter][i]);  
				}  
			}  
			else if (jsvalue[*iter].type() == Json::stringValue)  
			{  
				myvalue = jsvalue[*iter].asString();
				std::cout << jsvalue[*iter].asString() << std::endl;  
			}  
			else if (jsvalue[*iter].type() == Json::realValue)  
			{  
				std::cout << jsvalue[*iter].asDouble() << std::endl;  
			}  
			else if (jsvalue[*iter].type() == Json::intValue)  
			{  
				sprintf(tmp,"%d",jsvalue[*iter].asInt());
				myvalue = tmp;
				std::cout << jsvalue[*iter].asInt() << std::endl;  
			}  
			else  
			{  
				std::cout << jsvalue[*iter].asUInt() << std::endl;  
			}  
			mykey= *iter;
			CheckSign += mykey+"="+myvalue+"&";

		}  
		
		//int su_index = CheckSign.find_last_of('&');
		//CheckSign.erase(su_index,1);

		std::string ssskey = SIGN_KEY;
		CheckSign+="signKey=" + ssskey;
		std::string smd5;

		smd5 = getMD5(CheckSign);

		//std::cout<< " ##### CheckSign is " <<CheckSign<<std::endl;
		//std::cout<< " ##### smd5 is " <<smd5<<std::endl;
		//std::cout<< " ##### SIGN is " <<SignStr<<std::endl;
		if(scheck)
		{
			Json::Reader reader;
			Json::Value root;
			if(!reader.parse(recbuf,root)){
				return -1;
			}
			
			try
			{
				MainCmd = root["Main_Command"].asInt();
				SubCmd = root["Sub_Command"].asInt(); 
				MainInfo   = root["Main_Info"].asString();
				SubInfo   = root["Sub_Info"].asString();
			}
			catch (...)
			{
				return -1;
			}
			
		}

		su_sttContralInfo.MainCommand = MainCmd;
		su_sttContralInfo.SubCommand = SubCmd;
		su_sttContralInfo.MainInfo = MainInfo;
		su_sttContralInfo.SubInfo = SubInfo;
		if(!strcmp(smd5.c_str(),SignStr.c_str()))
		{
			domyCommand(su_sttContralInfo);
		}
		
		
	}

	////不是命令判断是不是订单信息
	std::string corder(command); 
	std::string subtransdataerr = "errmsg";
	std::string subtransdataok = "appid";
	int loc = corder.find( subtransdataerr, 0 );
	if( loc != std::string::npos )
	{
		std::cout << "find errmsg !!! " << loc << std::endl;////返回 errmsg 说明订单失败
		return 3;
	}
	loc = corder.find( subtransdataok, 0 );
	if( loc != std::string::npos )
	{
		std::cout << "find appid !!! " << loc << std::endl;////
		//result = SocketContral::instance->doPurchaseFun(recbuf);
	}


	return result;
}


DWORD WINAPI SocketContral::doListenFuction(LPVOID lpData)
{
	SOCKET ThreadSocket  = *((SOCKET*)lpData);

	char buf[1024];
	int m_result;
	int rval = 0;
	memset(buf,0,sizeof(buf));  
	rval = recv(ThreadSocket,buf,1024,0);  

	//printf("new thread socket is : %d\n",ThreadSocket);
	printf("Data from Client is : %s \n",buf);

	if (rval == SOCKET_ERROR)				//!这应该是个异常，当客户端没有调用closeSocket就直接退出游戏的时候，将会进入这里  
		printf("recv socket error\n"); 

	if (rval == 0)							//recv返回0表示正常退出  
		printf("ending connection");  

	m_result = SocketContral::instance->ParseCommand(buf);
	m_result = 0;
	Json::Value root;  
	if( 0 == m_result)
	{
		root["result"] = Json::Value("SUCCESS");   
		printf(" Succcess !!! \n");
	}else
	{
		root["result"] = Json::Value("FAILURE");   
		printf(" Failure !!! \n");
	}

	//! 将执行结果返回给第三方支付

	char sendBuf[128]={0};
	char *p = "Server recieve success!!!";
	//strcpy(sendBuf,p);
	std::string su_strstr = root.toStyledString();
	strcpy(sendBuf,su_strstr.c_str());
	send(ThreadSocket,sendBuf,strlen(sendBuf)+1,0);
	
	//关闭对应Accept的socket
	closesocket(ThreadSocket); 

	return 0;

}

DWORD WINAPI SocketContral::ServerRun(LPVOID lpData)  
{  
	//公开连接 
	int rlt = 0;  

	//socketMutex = MaxThreadNum;
	//用于记录错误信息，并输出  
	int iErrorMsg;  

	//初始化WinSock  
	WSAData wsaData;  
	iErrorMsg = WSAStartup(MAKEWORD(1,1),&wsaData);  

	if (iErrorMsg != NO_ERROR)  
	{  
		//初始化WinSock失败  
		printf("wsastartup failed with error : %d\n",iErrorMsg); 
		rlt = 1;  
		return rlt;  
	}  

	//创建服务端Socket  
	static SOCKET __ServerSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);  
	if (__ServerSocket == INVALID_SOCKET)           
	{  
		//创建Socket异常  
		rlt = 2;  
		return rlt;  
	}  

	//声明信息  

	sockaddr_in serverAddr,clientAddr;  
	serverAddr.sin_family = AF_INET;  
	//serverAddr.sin_addr.s_addr = inet_addr(IP);
	serverAddr.sin_port = htons(ServerListenPort);
	//serverAddr.sin_port = htons(ServerListenPort);  
	//serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	//serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	//serverAddr.sin_addr.S_un.S_addr = inet_addr(SU_SERVER_IP);
	serverAddr.sin_addr.s_addr = inet_addr(SU_SERVER_IP);
	//绑定  
	iErrorMsg = bind(__ServerSocket,(sockaddr*)&serverAddr,sizeof(serverAddr));  
	if (iErrorMsg < 0)  
	{  
		//绑定失败  
		printf("bind failed with error : %d\n",iErrorMsg); 
		rlt = 3;  
		return rlt;  
	}  

	if( listen(__ServerSocket,5) == SOCKET_ERROR) 
	{
		printf("Listen error!\n");	
	}
	int len = sizeof(SOCKADDR);  
	//char buf[1024];
	printf("SocketContral ServerRun Success ... \n");
	do   
	{  
		//接收信息  
		SOCKET Client = accept(__ServerSocket,(sockaddr*)&clientAddr,&len);  

		if (Client == INVALID_SOCKET)  
		{  
			//非可用socket  
			printf(" socket err !!! \n");
		}  
		else  
		{  
			//新socket连接  
			printf("new socket connect : %d\n",Client);    
			ServerThreadSocket = Client;
			HANDLE hThread1 = CreateThread(NULL,0,doListenFuction,(LPVOID)&ServerThreadSocket,0,NULL);//创建新的线程处理不同的接收数据
			CloseHandle(hThread1);

		}  

	} while (1);  

	//关闭自身的Socket  
	::closesocket(__ServerSocket); 
	return 0;
}

void SocketContral::StartMyServerThread()//启动线程
{  
	m_pkServer = g_pServerManager->GetServer("TexasPokerHall");
	su_timer1 = 8190;
	//SetTimer(NULL,su_timer1,500,TimerProc);
	HANDLE hThread = CreateThread(NULL,0,ServerRun,(LPVOID)&port,0,NULL);//创建新线程  
	CloseHandle(hThread);  
	HANDLE hThread1 = CreateThread(NULL,0,updatePoolRun,(LPVOID)&port,0,NULL);//创建新线程  
	CloseHandle(hThread1); 
} 

void SocketContral::dorobotFunc(GlobalContral& su_sttInfo)
{
	ENUM_Sub_Command nowsubtype =ENUM_Sub_Command(su_sttInfo.SubCommand);
	switch(nowsubtype)
	{
	case CUB_COMMAND_2:
		ModifyCommonRoomRobotConf(su_sttInfo);
		break;
	case CUB_COMMAND_3:
		ModifyMetchRoomConf(su_sttInfo);
		break;
	case CUB_COMMAND_4:

		break;
	case CUB_COMMAND_5:
		ModifySingleRobotInfo(su_sttInfo);
		break;
	case CUB_COMMAND_6:

		break;
	case CUB_COMMAND_7:

		break;
	case CUB_COMMAND_8:

		break;
	}

}
void SocketContral::ModifyCommonRoomRobotConf(GlobalContral su_sttInfo)
{
	VeVector<DBCommonRoomInfo>& su_CommonRoom = m_HServer->GetCommonRoomConfigInfo();

	VeUInt32 su_u32RoomID = 0;
	VeUInt32 su_u32RobotAI = 0;
	VeUInt32 su_u32MaxProfit = 0;
	VeUInt32 su_u32Fold = 0;
	VeUInt32 su_u32AIWin = 0;
	VeUInt32 su_u32PokerType = 0;
	su_u32RoomID = (VeUInt32)atoi(su_sttInfo.MainInfo.c_str());

	//std::cout<<" In String &&&&&&&&&&&&&& is "<<su_sttInfo.SubInfo<<std::endl;
	std::vector<std::string> su_strcommoninfo = su_strSplit(su_sttInfo.SubInfo,"&");

	//std::cout<<" In String &&&&&&&&&&&&&& NEW size  is "<<su_strcommoninfo.size()<<std::endl;

	for(VeUInt32 i(0);i<su_strcommoninfo.size();i++)
	{
		if(0 == i)
		{
			su_u32RobotAI = (VeUInt32)atoi(su_strcommoninfo[i].c_str());
		}
		else if(1 == i)
		{
			su_u32MaxProfit  = (VeUInt32)atoi(su_strcommoninfo[i].c_str());
		}
		else if(2 == i)
		{
			su_u32Fold = (VeUInt32)atoi(su_strcommoninfo[i].c_str());
		}
		else if(3 == i)
		{
			su_u32AIWin = (VeUInt32)atoi(su_strcommoninfo[i].c_str());
		}
		else if(4 == i)
		{
			su_u32PokerType = (VeUInt32)atoi(su_strcommoninfo[i].c_str());
		}
	}
	
	//std::cout<<" In su_u32RobotAI &&&&&&&&&&&&&& su_u32RobotAI  is "<<su_u32RobotAI<<std::endl;
	//std::cout<<" In su_u32MaxProfit &&&&&&&&&&&&&& su_u32MaxProfit  is "<<su_u32MaxProfit<<std::endl;
	//std::cout<<" In su_u32Fold &&&&&&&&&&&&&& su_u32Fold  is "<<su_u32Fold<<std::endl;
	//std::cout<<" In su_u32AIWin &&&&&&&&&&&&&& su_u32AIWin  is "<<su_u32AIWin<<std::endl;
	//std::cout<<" In su_u32PokerType &&&&&&&&&&&&&& su_u32PokerType  is "<<su_u32PokerType<<std::endl;

	for(VeUInt32 i(0);i<su_CommonRoom.Size();i++)
	{
		if(su_CommonRoom[i].su_u32RoomID == su_u32RoomID)
		{
			su_CommonRoom[i].su_u32RobotAI = su_u32RobotAI;
			su_CommonRoom[i].su_u32RoomMaxProfit = su_u32MaxProfit;
			su_CommonRoom[i].su_u32AIFold = su_u32Fold;
			su_CommonRoom[i].su_u32AIWin = su_u32AIWin;
			su_CommonRoom[i].su_u32PokerType = su_u32PokerType;
		}
	}
}

void SocketContral::ModifyMetchRoomConf(GlobalContral su_sttInfo)
{
	VeUInt32 su_u32MatchID = 0;
	VeUInt32 su_u32RobotAI = 0;
	su_u32MatchID = (VeUInt32)atoi(su_sttInfo.MainInfo.c_str());
	su_u32RobotAI = (VeUInt32)atoi(su_sttInfo.SubInfo.c_str());
	VeVector<MatchRoomConfig>&  su_MatchConfInfo= m_HServer->GetMatchConfigInfoModify();
	VeVector<RoomInfo>& su_MatchRoomInfo = m_HServer->GetMatchRoomInfoModify();

	for(VeUInt32 i(0);i<su_MatchConfInfo.Size();i++)
	{
		if(su_MatchConfInfo[i].su_u32MatchID == su_u32MatchID)
		{
			su_MatchConfInfo[i].su_u32RobotAI = su_u32RobotAI;
			su_MatchRoomInfo[i].m_u32M_Easy = su_u32RobotAI;
		}
	}

}


void SocketContral::ModifySingleRobotInfo(GlobalContral su_sttInfo)
{
	VeUInt32 playerid = 0;
	VeUInt32 playerchips = 100000;
	VeUInt32 playervip = 1;
	std::string playernickname = "";
	std::string playeruuid = "";
	std::string playerImg ="http://dezhou.yuanfan365.com:7070/100/1A_1255.jpg";

	VeVector<RebotInfo>& su_vecRobot = m_HServer->GetRobotInfoModify();
	std::vector<std::string> su_strrobotinfo = su_strSplit(su_sttInfo.SubInfo,"&");

	for(VeUInt32 i(0);i<su_strrobotinfo.size();i++)
	{
		if(0 == i)
		{

		}else if(1 == i)
		{
			try
			{
				playerid = (VeUInt32)atoi(su_strrobotinfo[i].c_str());
			}
			catch (...)
			{
				
			}
			
		}else if(2== i)
		{
			
		}else if(3== i)
		{

		}else if(4== i)
		{
			try
			{
				playerchips = (VeUInt32)atoi(su_strrobotinfo[i].c_str());
			}
			catch (...)
			{

			}
			
		}else if(5 == i)
		{
			try
			{
				playervip = (VeUInt32)atoi(su_strrobotinfo[i].c_str());
			}
			catch (...)
			{

			}
		}
	}

	for( VeUInt32 i(0); i< su_vecRobot.Size();i++)
	{
		if(!strcmp(su_vecRobot[i].r_PlayerUUID,playeruuid.c_str()))
		{
			su_vecRobot[i].r_u8Vip = (VeUInt8)playervip;
			su_vecRobot[i].r_u64TotalChips = (long long)
			memset(su_vecRobot[i].r_strPlayerUrl,0,30);
			strcpy(su_vecRobot[i].r_strPlayerUrl,playerImg.c_str());
			//memset(su_vecRobot[i].r_strName,0,30);
			//strcpy(su_vecRobot[i].r_strName,playernickname.c_str());
		}
	}

}

void SocketContral::initHserver(TexasPokerHallServer* pserver)
{
	m_HServer = pserver;
}
void SocketContral::UpdateRobotInfo()
{
	VeVector<RebotInfo> su_newrobot;
	su_newrobot = m_HServer->GetDBBusiness()->SelectRebotMessage();
	m_HServer->SetUpdateRobotInfo(su_newrobot);
}

unsigned int  SocketContral::GetdyDayOfYear()
{
	return su_dydays;
}

void CALLBACK TimerProc(HWND   hWnd,UINT   nMsg,UINT   nTimerid,DWORD   dwTime)
{
	switch(nTimerid)   
	{   
	case   8190:   ///处理ID为8190的定时器   
		{
			//time_t timer;
			//timer = time((time_t *)NULL);
			//struct tm *tblock;  
			//tblock=localtime(&timer);  
			//unsigned int my_uhour  = tblock->tm_hour;
			//unsigned int my_umin  = tblock->tm_min;
			//unsigned int my_sec = tblock->tm_sec;
			//unsigned int my_dydays = tblock->tm_yday;
			//std::cout<<" my_dydays : "<<my_dydays<<" su_dydays : "<<SocketContral::getinstance()->GetdyDayOfYear() <<std::endl;
		};   
		break;   
	case   2:   ///处理ID为2的定时器   
		;   
		break;  
	default :
		break;
	}   

	//time_t timer;
	//timer = time((time_t *)NULL);
	//struct tm *tblock;  
	//tblock=localtime(&timer);  
	//unsigned int my_uhour  = tblock->tm_hour;
	//unsigned int my_umin  = tblock->tm_min;
	//unsigned int my_sec = tblock->tm_sec;
	//unsigned int my_dydays = tblock->tm_yday;
	//std::cout<<" my_dydays : "<<my_dydays<<" su_dydays : "<<SocketContral::getinstance()->GetdyDayOfYear() <<std::endl;
	
	//if(my_dydays > su_dydays && 3 ==my_uhour && 0 == my_umin && 0 == my_sec)
	//{
	//	su_dydays = my_dydays;
	//	getinstance()->UpdateRobotInfo();
	//}
}


DWORD WINAPI SocketContral::updatePoolRun(LPVOID lpData)
{
	while(1)
	{
		time_t timer;
		timer = time((time_t *)NULL);
		struct tm *tblock;  
		tblock=localtime(&timer);  
		unsigned int my_uhour  = tblock->tm_hour;
		unsigned int my_umin  = tblock->tm_min;
		unsigned int my_sec = tblock->tm_sec;
		unsigned int my_dydays = tblock->tm_yday;
		//std::cout<<" my_dydays : "<<my_dydays<<" su_dydays : "<<SocketContral::getinstance()->GetdyDayOfYear() <<std::endl;

		if(my_dydays > su_dydays && 3 ==my_uhour && 0 == my_umin && 0 == my_sec)
		{
			su_dydays = my_dydays;
			getinstance()->UpdateRobotInfo();
		}
		Sleep(500);
	}
}
