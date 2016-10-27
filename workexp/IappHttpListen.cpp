#include "IappHttpListen.h"
#include <Winsock2.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <json/json.h>
#include "SignUtils.h"
#include <AccessFiles/TradingResultsNotice.h>

#pragma comment(lib,"ws2_32.lib")
#define MaxBuf  2048  //数据缓冲区大小
#define MaxReqBuf 2048  //解析post过来的参数数据缓存
#define Addr_Len 64    //服务器地址长度例如www.nohack.cn的长度
#define TIME_WAIT   2000 //连接超时

#define MAX_CLIENT_NUM 6000
#define SU_PORT 8191

typedef struct ClientInfo{
	SOCKET   client; 
	struct sockaddr_in ClientProxy;
}ClientInfo;

static ClientInfo S_Proxy[MAX_CLIENT_NUM];
SOCKET   client[MAX_CLIENT_NUM]; //最大并发连接数


IappHttpListen* IappHttpListen::instance = NULL;

IappHttpListen::IappHttpListen(void)
{
}

IappHttpListen::~IappHttpListen(void)
{
}
IappHttpListen* IappHttpListen::getInstance()
{
	if(instance == NULL)
	{
		instance = new IappHttpListen();
		instance->doMyListenThread();
	}
	return instance;
}

void IappHttpListen_mlog(std::string str)
{

	FILE *stream; 
	str  = str +"\r\n";
	char vcbuf[512]; 
	memset(vcbuf,0,sizeof(vcbuf));
	strcpy(vcbuf,str.c_str());
	int numwritten; 

	if( (stream = fopen( "c:/serverRcv.txt", "ab+" )) != NULL ) 
	{ 
		numwritten = fwrite( vcbuf, str.length() ,1,stream ); 
		printf( "Write %d items\n", numwritten ); 
	} 

	fclose( stream );
}


int IappHttpListen::doPurchaseFun(const char* order)
{
	//SU_PRINT("doPurchaseFun","IappHttpListen");
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
	Server* pkServer = g_pServerManager->GetServer("TexasPokerHall");
	std::map<VeUInt32,ClientAgent*> mapclient = pkServer->getClientIDMap();
	for (std::map<VeUInt32,ClientAgent*>::iterator it = mapclient.begin();it != mapclient.end();++it)
	{
		ClientAgent* pkAgent = (*it).second;
		if(0 == (pkAgent->UUID).StrCmp(veuuid))
		{
			userisonline = true;
			EntityS* pkEnt = pkAgent->GetEntity("Player");
			TexasPokerHallPlayerS* pkPlayer = static_cast<TexasPokerHallPlayerS*>(pkEnt);
			if(pkAgent && 0 == result )
			{
				payresult = pkPlayer->FinishIapppay(appuserid.c_str(),&Iapp,result);				
				break;
			}
			if(pkAgent && 1 == result)
			{
				payresult = pkPlayer->FinishIapppay(appuserid.c_str(),&Iapp,result);						
				break;
			}
		}
	}

	if( false == userisonline )
	{
		payresult = pkServer->serverFinishIapppay(appuserid.c_str(),&Iapp,result);
	}

	return payresult;
}

DWORD WINAPI IappHttpListen::OrderThread(LPVOID lpData)
{
	struct ClientInfo* Remote = (struct ClientInfo*) lpData;
	SOCKET ClientSocket; 
	char ReceiveBuf[MaxBuf]; 
	char ReceiveBufData[MaxBuf];
	//char reqData[MaxReqBuf];
	int DataLen; 
	int Postlen = -3;
	int CheckResult = -3;
	int orderresult = -3;
	//struct sockaddr_in  ServerAddr;
	//SOCKET ProxySocket; 
	int time = TIME_WAIT;
	char* RecdataStartPost = "POST";
	//char* RecdataStartPost = "GET";
	char* FindStrTmp = NULL;
	char* FirstPostLocation = NULL;
	char* FirstPost = NULL;
	char* EndPost = NULL;
	char* FindTrandsDataStart = "transdata";
	char* FindTrandsDataEnd = "RSA";
	std::string sstr_transdata="transdata";
	ClientSocket = (SOCKET)Remote->client; 
	char* HTTPHEAD = "HTTP/1.1  200 Ok\r\n"
		"Server: yuanfan365\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"Accept-Ranges: bytes\r\n"
		"Content-Length: 64\r\n"	
		"Connection: close\r\n\r\n ";
	////接受第一次请求信息 
	////首先从客户中取得数据 HTTP 头部
	memset(ReceiveBuf,0,MaxBuf);
	DataLen = recv(ClientSocket,ReceiveBuf,MaxBuf,0); 
	//printf("The Socket ID is %d\n",ClientSocket);
	//printf("OrderThread ReceiveBuf HTTP Head is : %s \n",ReceiveBuf);
	FindStrTmp = strstr(ReceiveBuf,RecdataStartPost);
	if(FindStrTmp == NULL)
	{
		printf("Recv DataLen is 0 \n");
		closesocket(ClientSocket);
		return -2;
	}
	if(DataLen == SOCKET_ERROR)
	{
		printf("Receive error!\n");
		closesocket(ClientSocket);
		return 1;
	}
	if(DataLen == 0)
	{
		printf("Recv DataLen is 0 \n");
		closesocket(ClientSocket);
		return 2;
	}
	////post 方式，接收数据 接收数据
	memset(ReceiveBufData,0,MaxBuf);
	FirstPostLocation = strstr(ReceiveBuf,RecdataStartPost) + strlen(RecdataStartPost);

	FirstPost = strstr(ReceiveBuf,FindTrandsDataStart);
	EndPost = (ReceiveBuf,FindTrandsDataEnd) + strlen(FindTrandsDataEnd);
	EndPost = '\0';
	strcpy(ReceiveBufData,FirstPost);


	////printf("Data of ReceiveBufData is %s\n",ReceiveBufData);

	////if(FirstPostLocation !=NULL)
	////{
	////	Postlen = recv(ClientSocket,ReceiveBufData,MaxBuf,0); 
	////	printf("OrderThread ReceiveBufData Post Data : %s \n",ReceiveBufData);

	////	FirstPost = strstr(ReceiveBufData,FindTrandsDataStart);
	////	EndPost = (ReceiveBufData,FindTrandsDataEnd) + strlen(FindTrandsDataEnd);
	////	EndPost = '\0';
	////	char tmpbuf[MaxBuf]= {0};
	////	strcpy(tmpbuf,FirstPost);
	////	printf("size of ReceiveBufData transdata is %s\n",tmpbuf);
	////}


	if(Postlen == SOCKET_ERROR)
	{
		printf("Receive error!\n");
		closesocket(ClientSocket);
		return 1;
	}
	if(Postlen == 0)
	{
		closesocket(ClientSocket);
		return 2;
	}
	std::string strReqData(ReceiveBufData); 
	CheckResult = TradingResultsNotice::CheckSign(strReqData,sstr_transdata);
	if( 0 == CheckResult)
	{
		orderresult = IappHttpListen::doPurchaseFun(sstr_transdata.c_str());

	}else if( -1 == CheckResult)
	{
		orderresult = -1;
	}

	IappHttpListen_mlog(sstr_transdata);

	char sendBuf[512]={0};
	char bak_buf[20]={0};
	char strsendbuf[512] = {0};
	//Json::Value root;  
	if( 0 == orderresult)
	{
		//printf("CheckSign Success !\n");
		//root["result"] = Json::Value("SUCCESS");
		strcat(bak_buf,"SUCCESS");
	}else
	{
		//printf("CheckSign Failure error!\n");
		//root["result"] = Json::Value("FAILURE");  
		strcat(bak_buf,"FAILURE");
	}

	//////测试用，返回值
	//root["result"] = Json::Value("SUCCESS"); 

	//std::string stringout= root.toStyledString(); 

	

	memset(sendBuf,0,512);
	//strcat(sendBuf,HTTPHEAD);////20160721 添加头部
	strcat(sendBuf,bak_buf);////20160721 添加头部
	//strcpy(sendBuf,stringout.c_str());
	//send(ClientSocket,sendBuf,strlen(sendBuf)+1,0);

	memset(strsendbuf,0,512);
	strcat(strsendbuf,HTTPHEAD);////20160721 添加头部
	std::string sendstrs = SignUtils::UrlUTF8(sendBuf);
	strcat(strsendbuf,sendstrs.c_str());
	send(ClientSocket,strsendbuf,strlen(strsendbuf)+1,0);
	//printf(" Send Data is : %s\n\n",strsendbuf);
	//关闭对应Accept的socket
	closesocket(ClientSocket); 

	return 0;
}

DWORD WINAPI IappHttpListen::ServerInit(LPVOID lpData)
{
	SOCKET  MainSocket,ClientSocket;

	//这里建立两个socket，一个用于接受连接，另一个用于保存新建的连接
	struct sockaddr_in Host,Client; 
	WSADATA WsaData;
	int AddLen;
	static int socketcount;
	//初始化 
	if(WSAStartup(MAKEWORD(2,2),&WsaData) < 0)
	{
		printf("WSAStartup Failed!\n");
		return 1;
	}
	//创建socket端口 
	MainSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(MainSocket == SOCKET_ERROR)
	{
		printf("Create socket error!\n");
		return 2;
	}
	Host.sin_family = AF_INET; 
	Host.sin_port = htons(SU_PORT); //代理服务器端口
	Host.sin_addr.s_addr = inet_addr(SU_SERVER_IP); 

	//绑定socket
	if(bind(MainSocket,(SOCKADDR *)&Host,sizeof(Host)) != 0)
	{
		printf("Bind error!\n");
	}
	socketcount = 0; //socket连接数
	//监听 
	if(listen(MainSocket,5) == SOCKET_ERROR)
	{
		printf("Listen error!\n");
	}
	AddLen = sizeof(Client);
	printf("IappHttpListen ServerInit Success ...\n");
	//接受新的客户连接
	for(;;)
	{
		
		ClientSocket = accept(MainSocket,(SOCKADDR *)&Client,&AddLen);
		//当有连接请求时，把该连接保存到新的socket中
		if(ClientSocket == SOCKET_ERROR)
		{
			printf("Accept error!\n");
		}
		printf("new accept : %d \n",ClientSocket);

		socketcount++;
		if(socketcount>=MAX_CLIENT_NUM)socketcount=0;
		S_Proxy[socketcount].client = ClientSocket;
		S_Proxy[socketcount].ClientProxy = Client;
		//对于每一个客户连接启动新的线程进行控制
		HANDLE hThread2 = CreateThread(NULL,0,OrderThread,(LPVOID)&S_Proxy[socketcount],0,NULL);
		CloseHandle(hThread2);
	}
	::closesocket(MainSocket);
	return 0;
}


void IappHttpListen::doMyListenThread()
{
	HANDLE thhread1 = CreateThread(NULL,0,ServerInit,0,0,NULL);
	CloseHandle(thhread1);
}