#include <iostream>
#include <string.h>


#include "MdSpiUser.h"
#include "../util/config_processor.h"

using namespace std;



//这里定义了全局变量，这些全局变量的定义一般来说需要extern，但是定义的时候如果位置在全局变量的位置的话就可以省略。
// 但是当在其他文件中使用本文件的全局变量的时候，必须使用extern.



TThostFtdcBrokerIDType brokerID ;
TThostFtdcInvestorIDType investorID;
TThostFtdcPasswordType investorPassword;
char mdFrontAddress[128];
char tdFrontAddress[128];



void init_account_info(std::string filename){
	config_processor cp = config_processor(filename);
	strcpy(brokerID, cp.get_value("brokerID").c_str());
	strcpy(investorID, cp.get_value("investorID").c_str());
	strcpy(investorPassword, cp.get_value("investorPassword").c_str());
	strcpy(mdFrontAddress, cp.get_value("mdFrontAddress").c_str());
	strcpy(tdFrontAddress, cp.get_value("tdFrontAddress").c_str());
}









char *instrumentID[] = {"rb2009","rb2011"};
int instrumentNum = 2;

CThostFtdcMdApi *pMdApiUser =nullptr;

// 这个文件对应的进程是提供和ctp api md 相关功能, 这里对md 数据不做任何处理， 只是单纯的转发数据到其他进程上
// 这个目的类似与nginx 的 master， 提高并发
int main(int argc, char *argv[])
{

	init_account_info("account.properties");
	cout << "init market data ......" << endl;

	MdSpiUser *pMdSpiUser = new MdSpiUser();

	pMdApiUser= CThostFtdcMdApi::CreateFtdcMdApi();
	pMdApiUser ->RegisterSpi(pMdSpiUser);
	pMdApiUser ->RegisterFront(mdFrontAddress);
	pMdApiUser ->Init();

	

	pMdApiUser ->Join();
	delete pMdSpiUser;
	pMdApiUser ->Release();
	
}