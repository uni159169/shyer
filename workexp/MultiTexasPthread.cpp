#include "MultiTexasPthread.h"
#include <Winsock2.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib,"ws2_32.lib")

#define SU_PTHREAD_MAX_CLIENT_NUM 6000
static su_MultiClientInfo S_Proxy[SU_PTHREAD_MAX_CLIENT_NUM];


int MultiTexasPthread::socketcount = 0;
MultiTexasPthread* MultiTexasPthread::instance = NULL;

MultiTexasPthread::MultiTexasPthread(void)
{
}

MultiTexasPthread::~MultiTexasPthread(void)
{
}
MultiTexasPthread* MultiTexasPthread::getInstance()
{
	if(instance == NULL)
	{
		instance = new MultiTexasPthread();
		//su_initcurl();
	}
	return instance;
}

void MultiTexasPthread_mlog(std::string str)
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


int MultiTexasPthread::doMultiFunCodeFun(void* su_sttCodeInfo)
{
	su_MultiClientInfo *su_sttInfo  = (su_MultiClientInfo *)su_sttCodeInfo;
	
	std::string su_strBack = "";
	std::string su_strpost = "";
	//std::string su_strposturl= "http://www.yzmsms.cn/sendSmsYZM.do";
	std::string su_strposturl = "http://www.568sms.net/api/sdk.php";
	std::string su_struname = "test100";
	std::string su_strPWcode = "123456";
	std::string su_strmobile = "13616183828";

	std::string su_strSendString = "ÑéÖ¤ÂëÊÇ: ";

	//std::cout<<" NUm is : "<<su_sttInfo->su_u32SendNum<<std::endl;

	char tmp[16]={0};
	_itoa(su_sttInfo->su_u32SendNum,tmp,10);
	std::string su_strcode(tmp);
	std::string su_mypwd = getMD5(su_strPWcode);
	su_strSendString = su_strSendString+su_strcode;

	su_strpost = "commandid=1&username="+su_struname+"&password="+su_mypwd+"&mobile="+su_strmobile+"&sendtime=&content="+su_strSendString+" ¡¾Ô¶·«Èí¼þ¿Æ¼¼¡¿";

	//std::cout<<"post string is "<<su_strpost<<std::endl;

	//std::string su_strContent = "commandid=1&username=test100&password=e10adc3949ba59abbe56e057f20f883e&mobile=13616183828&sendtime=&content=²âÊÔ¡¾²âÊÔ¡¿";
	su_curlPost(su_strposturl,su_strpost,"UTF-8",su_strBack);

	//std::cout<<" #######  Back is "<<su_strBack<<std::endl;

	return 0;

}

DWORD WINAPI MultiTexasPthread::MultiThread(LPVOID lpData)
{
	su_MultiClientInfo *su_sttInfo = (su_MultiClientInfo *)lpData;

	//std::cout<<" su_sttInfo "<<su_sttInfo->su_u32FunType<<std::endl;
	//std::cout<<" su_sttInfo "<<su_sttInfo->su_u32SendNum<<std::endl;
	socketcount++;
	if(socketcount>=SU_PTHREAD_MAX_CLIENT_NUM)socketcount=0;
	S_Proxy[socketcount].su_u32FunType = su_sttInfo->su_u32FunType;
	S_Proxy[socketcount].su_u32SendNum =su_sttInfo->su_u32SendNum;
	strcpy(S_Proxy[socketcount].su_strSendInfo,su_sttInfo->su_strSendInfo);

	if( 1 == su_sttInfo->su_u32FunType)
	{
		doMultiFunCodeFun(su_sttInfo);

	}else if( 2 == su_sttInfo->su_u32FunType)
	{

		doMultiFunCodeFun(su_sttInfo);
	}else
	{
		doMultiFunCodeFun(su_sttInfo);
	}
	

	return 0;
}

void MultiTexasPthread::ServerInit(su_MultiClientInfo *su_sttMultiInfo)
{
	static su_MultiClientInfo su_sttInfo;
	su_sttInfo.su_u32FunType = su_sttMultiInfo->su_u32FunType;
	su_sttInfo.su_u32SendNum = su_sttMultiInfo->su_u32SendNum;
	strcpy(su_sttInfo.su_strSendInfo,su_sttMultiInfo->su_strSendInfo);
	//std::cout<<" ServerInit "<<su_sttInfo.su_u32FunType<<std::endl;
	//std::cout<<" ServerInit "<<su_sttInfo.su_u32SendNum<<std::endl;
	HANDLE hThread2 = CreateThread(NULL,0,MultiThread,(LPVOID)&su_sttInfo,0,NULL);
	
	CloseHandle(hThread2);
	return ;
}


void MultiTexasPthread::doMyPThread()
{
	//HANDLE thhread1 = CreateThread(NULL,0,ServerInit,0,0,NULL);
	//CloseHandle(thhread1);
}
void MultiTexasPthread::su_initcurl()
{
	std::string su_strBack = "";
	std::string su_strpost = "";
	std::string su_strposturl = "http://www.568sms.net/api/sdk.php";
	std::string su_struname = "test100";
	std::string su_strPWcode = "123456";
	std::string su_strmobile = "13616183828";

	std::string su_strContent = "commandid=1&username=test100&password=e10adc3949ba59abbe56e057f20f883e&mobile=13616183828&sendtime=&content=²âÊÔ¡¾²âÊÔ¡¿";
	//su_curlPost(su_strposturl,su_strpost,"UTF-8",su_strBack);
	su_curlPost(su_strposturl,su_strContent,"UTF-8",su_strBack);
	std::cout<<" #######  su_initcurl  is "<<su_strBack<<std::endl;

}