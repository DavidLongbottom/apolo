#include <iostream>
#include <fstream>
#include <unordered_map>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <chrono>


#include "MdSpiUser.h"


#include "../ipc/zmq_ipc/zmq_context_singleton.hpp"
#include "../ipc/zmq_ipc/zmq_socket_factory.hpp"
#include "../ipc/zmq_ipc/zmqhelper.hpp"
#include "../pojo/market_data.h"



using namespace std;

extern TThostFtdcBrokerIDType brokerID;
extern TThostFtdcInvestorIDType investorID;
extern TThostFtdcPasswordType investorPassword;

extern char mdFrontAddress[];
extern char *instrumentID[];
extern int instrumentNum;

extern CThostFtdcMdApi *pMdApiUser;
MdSpiUser::MdSpiUser(){

    zmq_context_singleton & singleton = zmq_context_singleton::singleton();
    void * context = singleton.get_singleton_context();

    socket = zmq_socket_factory::get_socket(context, "tick", 1);
}

void MdSpiUser::OnFrontConnected()
{
    cout<<"this is OnFrontConnected"<<endl;
    cout <<"api version:   " << pMdApiUser ->GetApiVersion() <<endl;
    //cout << "broker id:  "<< brokerID <<endl;
    //cout << "investor id:  " << investorID <<endl;
    
    CThostFtdcReqUserLoginField loginField;
    memset(&loginField, 0, sizeof(loginField));
    strcpy(loginField.BrokerID, brokerID);
    strcpy(loginField.UserID, investorID);
    strcpy(loginField.Password, investorPassword);

    int requestID = 0;
    int resultSendLogin = pMdApiUser ->ReqUserLogin(&loginField,requestID);
    if (!resultSendLogin)
    {
        cout << "send login request successful" << endl;
    }
    else
    {
        cout << "send login request failed" <<endl;
    }
    
}

void MdSpiUser::OnFrontDisconnected(int nReason)
{
    cout << "this is onFrontDisconnected" <<endl;
}

void MdSpiUser::OnHeartBeatWarning(int nTimeLapse)
{
    cout << "this is OnHeartBeatWarning" <<endl;
}

void MdSpiUser::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cout << "this is OnRspUserLogin" <<endl;
    cout << "login successfully" << endl;

    bool resultLogin = pRspInfo && (pRspInfo ->ErrorID !=0);
    if (!resultLogin)
    {
        cout << "login response successfully " << endl;
        cout << "账户登录成功" << endl;
        cout << "交易日期:  " << pRspUserLogin ->TradingDay <<endl;
        cout << "登录时间:  " << pRspUserLogin ->LoginTime <<endl;
        cout << "经纪商ID:  " << pRspUserLogin ->BrokerID <<endl;
        cout << "账户ID:  " << pRspUserLogin ->UserID <<endl;

        int resultSubscribe = pMdApiUser->SubscribeMarketData(instrumentID,instrumentNum);    
        if (!resultSubscribe)
        {
            cout << "send market data request successfelly" <<endl;
        }
        else
        {
            cout << "send market data request failed" <<endl;
        }
    }
    else 
    {
        cout << "login response failed" <<endl;
    }


}


void MdSpiUser::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cout << "this is OnRspUserLogout" <<endl;
}

///错误应答
void MdSpiUser::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
    cout << "this is OnRspError" <<endl;
}

///订阅行情应答
void MdSpiUser::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
    cout << "this is OnRspSubMarketData" <<endl;
    bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (!bResult)
	{
		std::cout << "=====订阅行情成功=====" << std::endl;
		std::cout << "合约代码： " << pSpecificInstrument->InstrumentID << std::endl;
		// 如果需要存入文件或者数据库，在这里创建表头,不同的合约单独存储
		char filePath[100] = {'\0'};
		sprintf(filePath, "%s_market_data.csv", pSpecificInstrument->InstrumentID);
		std::ofstream outFile;
		outFile.open(filePath, std::ios::out); // 新开文件
		outFile << "合约代码" << ","
			<< "更新时间" << ","
			<< "最新价" << ","
			<< "成交量" << ","
			<< "买价一" << ","
			<< "买量一" << ","
			<< "卖价一" << ","
			<< "卖量一" << ","
			<< "持仓量" << ","
			<< "换手率"
			<< std::endl;
		outFile.close();

	}
	else
		std::cerr << "返回错误--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

///取消订阅行情应答
void MdSpiUser::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
    cout <<"this is OnRspUnSubMarketData" <<endl;
}

///订阅询价应答
void MdSpiUser::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
    cout << "this is OnRspSubForQuoteRsp" <<endl;
}

///取消订阅询价应答
void MdSpiUser::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
    cout << "this is OnRspUnSubForQuoteRsp" <<endl;
}

///深度行情通知
void MdSpiUser::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) 
{
    cout << "this is OnRtnDepthMarketData" <<endl;
    cout << pDepthMarketData ->TradingDay << "   ";
    cout << pDepthMarketData ->UpdateTime << "   ";
    cout << pDepthMarketData ->InstrumentID << "   ";
    cout << pDepthMarketData ->OpenPrice << "   ";
    cout << pDepthMarketData ->LastPrice << "   ";
    cout << pDepthMarketData ->Volume << "   ";
    cout << pDepthMarketData ->BidPrice1 << "   ";
    cout << pDepthMarketData ->BidVolume1 << "   ";
    cout << pDepthMarketData ->AskPrice1 << "   ";
    cout << pDepthMarketData ->AskVolume1 << "   ";
    cout << pDepthMarketData ->HighestPrice << "   ";
    cout << pDepthMarketData ->LowestPrice << endl;
    //std::this_thread::sleep_for(chrono::seconds(1));
    



    int i = 0;
    while(i < 3){
        
        market_data md;
        md.open_interest = 100;
        md.asks[0] = 1;
        md.asks[1] = 2;
        md.is_initialized = true;
        md.count = i;

        z_msg_send<CThostFtdcDepthMarketDataField>(socket,pDepthMarketData->InstrumentID,*pDepthMarketData);


        i = i+1;
        //sleep(1);
        
    }

    //zmq_socket_factory::close_socket(socket);


}

///询价通知
void MdSpiUser::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp) 
{
    cout << "this is OnRtnForQuoteRsp" <<endl;
}

