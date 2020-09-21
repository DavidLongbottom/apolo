#include <iostream>
#include <fstream>
#include <unordered_map>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <chrono>
#include <iconv.h>




#include "TdSpiUser.h"




extern TThostFtdcBrokerIDType brokerID;
extern TThostFtdcInvestorIDType investorID;
extern TThostFtdcPasswordType investorPassword;

extern char tdFrontAddress[];

extern CThostFtdcTraderApi *pTdApiUser;


extern TThostFtdcFrontIDType	trade_front_id;	//前置编号
extern TThostFtdcSessionIDType	session_id;	//会话编号
extern TThostFtdcOrderRefType	order_ref;	//报单引用

int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen) 
{ 
    iconv_t cd; 
    int rc; 
    char **pin = &inbuf; 
    char **pout = &outbuf; 

    cd = iconv_open(to_charset,from_charset); 
    if (cd==0) return -1; 
    memset(outbuf,0,outlen); 
    size_t ilen = inlen;
    size_t olen = outlen;
    if (iconv(cd,pin,&ilen,pout,&olen)==-1) return -1; 
    iconv_close(cd); 
    return 0; 
} 

int u2g(char *inbuf,int inlen,char *outbuf,int outlen) 
{ 
    return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen); 
} 
//GB2312码转为UNICODE码 
int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen) 
{ 
    return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen); 
} 


TdSpiUser::TdSpiUser(){
    zmq_context_singleton & singleton = zmq_context_singleton::singleton();
    void * context = singleton.get_singleton_context();
    socket_status = zmq_socket_factory::get_socket(context, "status", 1);
};




///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
void TdSpiUser::TdSpiUser::OnFrontConnected(){
    std::cout << "td on front connectd" << std::endl;;

    CThostFtdcReqUserLoginField loginField;
    memset(&loginField, 0, sizeof(loginField));
    strcpy(loginField.BrokerID, brokerID);
    strcpy(loginField.UserID, investorID);
    strcpy(loginField.Password, investorPassword);


    int requestID = 0;
    /*
    int resultSendLogin  = pTdApiUser -> ReqUserLogin(&loginField,requestID);
    if (!resultSendLogin)
    {
        std::cout << "send td login request successful" << std::endl;
    }
    else
    {
        std::cout << "send td login request failed" <<std::endl;
    }
    */
    // 第一次登录trade api 的话必须要修改密码， 修改的结果成功与否在OnRspUserPasswordUpdate函数里面
    CThostFtdcUserPasswordUpdateField reqUserPasswordUpdate;
    memset(&reqUserPasswordUpdate, 0, sizeof(reqUserPasswordUpdate));
    strcpy(reqUserPasswordUpdate.BrokerID, brokerID);
    strcpy(reqUserPasswordUpdate.UserID, investorID);
    strcpy(reqUserPasswordUpdate.OldPassword, investorPassword);
    strcpy(reqUserPasswordUpdate.NewPassword, "**********");
    int resultUpdatepass = pTdApiUser -> ReqUserPasswordUpdate(&reqUserPasswordUpdate, 0);

    if (!resultUpdatepass)
    {
        std::cout << "update password req send" << std::endl;
    }
    else
    {
        std::cout << "update password send request failed" <<std::endl;
    }




};

///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
///@param nReason 错误原因
///        0x1001 网络读失败
///        0x1002 网络写失败
///        0x2001 接收心跳超时
///        0x2002 发送心跳失败
///        0x2003 收到错误报文
void TdSpiUser::OnFrontDisconnected(int nReason){
    std::cout << "on front disconnected" << std::endl;

}

///心跳超时警告。当长时间未收到报文时，该方法被调用。
///@param nTimeLapse 距离上次接收报文的时间
void TdSpiUser::OnHeartBeatWarning(int nTimeLapse){};

///客户端认证响应
void TdSpiUser::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};


///登录请求响应
void TdSpiUser::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {

    std::cout << pRspInfo ->ErrorID << std::endl;

    bool resultLogin = pRspInfo && (pRspInfo ->ErrorID !=0);

    if (!resultLogin)
    {
		std::cout << "=====账户登录成功=====" << std::endl;
		std::cout << "交易日： " << pRspUserLogin->TradingDay << std::endl;
		std::cout << "登录时间： " << pRspUserLogin->LoginTime << std::endl;
		std::cout << "经纪商： " << pRspUserLogin->BrokerID << std::endl;
		std::cout << "帐户名： " << pRspUserLogin->UserID << std::endl;
		// 保存会话参数
		trade_front_id = pRspUserLogin->FrontID;
		session_id = pRspUserLogin->SessionID;
		strcpy(order_ref, pRspUserLogin->MaxOrderRef);
    }else{
        std::cout << "login failed " <<std::endl;
        char out[255];

        g2u(pRspInfo ->ErrorMsg,strlen(pRspInfo ->ErrorMsg),out,255);
        std::cout << out << std::endl;
        //std::cout << pRspInfo ->ErrorMsg << std::endl;
        return;
    }

    //在登录成功后要进行OnRspSettlementInfoConfirm， 这是必须要做的事情

	CThostFtdcSettlementInfoConfirmField settlementConfirmReq;
	memset(&settlementConfirmReq, 0, sizeof(settlementConfirmReq));
	strcpy(settlementConfirmReq.BrokerID, brokerID);
	strcpy(settlementConfirmReq.InvestorID, investorID);
	int requestID = 0; // 请求编号
	int rt = pTdApiUser->ReqSettlementInfoConfirm(&settlementConfirmReq, requestID);
	if (!rt)
		std::cout << ">>>>>>发送投资者结算结果确认请求成功" << std::endl;
	else
		std::cerr << "--->>>发送投资者结算结果确认请求失败" << std::endl;



};

///登出请求响应
void TdSpiUser::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::cout << "user td logout" << std::endl;
};

///用户口令更新请求响应
void TdSpiUser::OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    
    std::cout << " on password update" << std::endl;
    bool resultUpdate = pRspInfo && (pRspInfo ->ErrorID !=0);

    if(!resultUpdate){
        //第一次登录账户的时候必须要修改密码
        std::cout << pUserPasswordUpdate->BrokerID << "    ";
        std::cout << pUserPasswordUpdate->OldPassword << "    ";
        std::cout << pUserPasswordUpdate->NewPassword << "    ";
        std::cout << pUserPasswordUpdate->UserID << std::endl;
    }else{
        std::cout << "update fail" << std::endl;
    }
    




};

///资金账户口令更新请求响应
void TdSpiUser::OnRspTradingAccountPasswordUpdate(CThostFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///查询用户当前支持的认证模式的回复
void TdSpiUser::OnRspUserAuthMethod(CThostFtdcRspUserAuthMethodField *pRspUserAuthMethod, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///获取图形验证码请求的回复
void TdSpiUser::OnRspGenUserCaptcha(CThostFtdcRspGenUserCaptchaField *pRspGenUserCaptcha, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///获取短信验证码请求的回复
void TdSpiUser::OnRspGenUserText(CThostFtdcRspGenUserTextField *pRspGenUserText, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///报单录入请求响应
void TdSpiUser::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    //“Thost 收到报单指令，如果没有通过参数校验，拒绝接受报单指令。用户就会收到 OnRspOrderInsert 消息，其中包含了错误编码和错误消息。
    // 如果Thost接受了报单指令， 用户不会收到OnRspOrderInser， 而会收到OnRtnOrder， 用来更新委托状态”。
    // 所以这里是处理报单错误的

    std::cout << " OnRspOrderInsert ： 交易核心返回报单错误" << std::endl;
    bool resultUpdate = pRspInfo && (pRspInfo ->ErrorID !=0);

    if(!resultUpdate){
    }else{
        event_order_status order_status("order_error", * pInputOrder);
        z_msg_send(socket_status, pInputOrder->InstrumentID, order_status);
    }
    

};

///预埋单录入请求响应
void TdSpiUser::OnRspParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///预埋撤单录入请求响应
void TdSpiUser::OnRspParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///报单操作请求响应
void TdSpiUser::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///查询最大报单数量响应
void TdSpiUser::OnRspQueryMaxOrderVolume(CThostFtdcQueryMaxOrderVolumeField *pQueryMaxOrderVolume, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///投资者结算结果确认响应
void TdSpiUser::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    //由于国内期货在交易日内首次登录时需要做投资者结算结果确认操作, 当确认成功以后才算登录成功。

    bool resultSettlement = pRspInfo && (pRspInfo ->ErrorID !=0);
    if(!resultSettlement){
        std::cout << "settlement success" << std::endl;
        std::cout << "settle date  " << pSettlementInfoConfirm ->ConfirmDate << "   ";
        std::cout << "settle time  " << pSettlementInfoConfirm ->ConfirmTime << std::endl;



        CThostFtdcQryInstrumentField instrumentReq;
        memset(&instrumentReq, 0, sizeof(instrumentReq));
        //strcpy(instrumentReq.InstrumentID, "cu1609");//全部合约
        strcpy(instrumentReq.InstrumentID, "cu");//全部合约

        int requestID = 0; // 请求编号
        int rt = pTdApiUser->ReqQryInstrument(&instrumentReq, requestID);
        if (!rt)
            std::cout << ">>>>>>发送合约查询请求成功" << std::endl;
        else
            std::cerr << "--->>>发送合约查询请求失败" << std::endl;



    }else{
        std::cout << "settlement error " << std::endl;
        std::cout << pRspInfo ->ErrorID << std::endl;
        return ;

    }



};

///删除预埋单响应
void TdSpiUser::OnRspRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///删除预埋撤单响应
void TdSpiUser::OnRspRemoveParkedOrderAction(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///执行宣告录入请求响应
void TdSpiUser::OnRspExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///执行宣告操作请求响应
void TdSpiUser::OnRspExecOrderAction(CThostFtdcInputExecOrderActionField *pInputExecOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///询价录入请求响应
void TdSpiUser::OnRspForQuoteInsert(CThostFtdcInputForQuoteField *pInputForQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///报价录入请求响应
void TdSpiUser::OnRspQuoteInsert(CThostFtdcInputQuoteField *pInputQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///报价操作请求响应
void TdSpiUser::OnRspQuoteAction(CThostFtdcInputQuoteActionField *pInputQuoteAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///批量报单操作请求响应
void TdSpiUser::OnRspBatchOrderAction(CThostFtdcInputBatchOrderActionField *pInputBatchOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///期权自对冲录入请求响应
void TdSpiUser::OnRspOptionSelfCloseInsert(CThostFtdcInputOptionSelfCloseField *pInputOptionSelfClose, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///期权自对冲操作请求响应
void TdSpiUser::OnRspOptionSelfCloseAction(CThostFtdcInputOptionSelfCloseActionField *pInputOptionSelfCloseAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///申请组合录入请求响应
void TdSpiUser::OnRspCombActionInsert(CThostFtdcInputCombActionField *pInputCombAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询报单响应
void TdSpiUser::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询成交响应
void TdSpiUser::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询投资者持仓响应
void TdSpiUser::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询资金账户响应
void TdSpiUser::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询投资者响应
void TdSpiUser::OnRspQryInvestor(CThostFtdcInvestorField *pInvestor, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询交易编码响应
void TdSpiUser::OnRspQryTradingCode(CThostFtdcTradingCodeField *pTradingCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询合约保证金率响应
void TdSpiUser::OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询合约手续费率响应
void TdSpiUser::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询交易所响应
void TdSpiUser::OnRspQryExchange(CThostFtdcExchangeField *pExchange, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询产品响应
void TdSpiUser::OnRspQryProduct(CThostFtdcProductField *pProduct, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询合约响应
void TdSpiUser::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {

    bool resultQry = pRspInfo && (pRspInfo ->ErrorID !=0);

    if(!resultQry){
        std::cout << "qry successful now" << std::endl;
        std::cout << pInstrument ->ExchangeID << "       ";
        std::cout << pInstrument ->InstrumentID << "       ";
        std::cout << pInstrument ->InstrumentName << "       ";
        std::cout << pInstrument ->CreateDate<< "       ";
        std::cout << pInstrument ->OpenDate << "       ";
        std::cout << pInstrument ->EndDelivDate << "       ";
        std::cout << pInstrument ->IsTrading << "       ";
    }else{
        std::cout << "query failed " << std::endl;
        if(pRspInfo == nullptr){
            std::cout << "nullptr" << std::endl;
        }
        else{
            std::cout << "other pRspinfo fail" << std::endl;
            std::cout << pRspInfo ->ErrorID << std::endl;
        }
        return;
    }

};

///请求查询行情响应
void TdSpiUser::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询投资者结算结果响应
void TdSpiUser::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询转帐银行响应
void TdSpiUser::OnRspQryTransferBank(CThostFtdcTransferBankField *pTransferBank, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询投资者持仓明细响应
void TdSpiUser::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询客户通知响应
void TdSpiUser::OnRspQryNotice(CThostFtdcNoticeField *pNotice, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询结算信息确认响应
void TdSpiUser::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询投资者持仓明细响应
void TdSpiUser::OnRspQryInvestorPositionCombineDetail(CThostFtdcInvestorPositionCombineDetailField *pInvestorPositionCombineDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///查询保证金监管系统经纪公司资金账户密钥响应
void TdSpiUser::OnRspQryCFMMCTradingAccountKey(CThostFtdcCFMMCTradingAccountKeyField *pCFMMCTradingAccountKey, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询仓单折抵信息响应
void TdSpiUser::OnRspQryEWarrantOffset(CThostFtdcEWarrantOffsetField *pEWarrantOffset, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询投资者品种/跨品种保证金响应
void TdSpiUser::OnRspQryInvestorProductGroupMargin(CThostFtdcInvestorProductGroupMarginField *pInvestorProductGroupMargin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询交易所保证金率响应
void TdSpiUser::OnRspQryExchangeMarginRate(CThostFtdcExchangeMarginRateField *pExchangeMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询交易所调整保证金率响应
void TdSpiUser::OnRspQryExchangeMarginRateAdjust(CThostFtdcExchangeMarginRateAdjustField *pExchangeMarginRateAdjust, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询汇率响应
void TdSpiUser::OnRspQryExchangeRate(CThostFtdcExchangeRateField *pExchangeRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询二级代理操作员银期权限响应
void TdSpiUser::OnRspQrySecAgentACIDMap(CThostFtdcSecAgentACIDMapField *pSecAgentACIDMap, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询产品报价汇率
void TdSpiUser::OnRspQryProductExchRate(CThostFtdcProductExchRateField *pProductExchRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询产品组
void TdSpiUser::OnRspQryProductGroup(CThostFtdcProductGroupField *pProductGroup, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询做市商合约手续费率响应
void TdSpiUser::OnRspQryMMInstrumentCommissionRate(CThostFtdcMMInstrumentCommissionRateField *pMMInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询做市商期权合约手续费响应
void TdSpiUser::OnRspQryMMOptionInstrCommRate(CThostFtdcMMOptionInstrCommRateField *pMMOptionInstrCommRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询报单手续费响应
void TdSpiUser::OnRspQryInstrumentOrderCommRate(CThostFtdcInstrumentOrderCommRateField *pInstrumentOrderCommRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询资金账户响应
void TdSpiUser::OnRspQrySecAgentTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询二级代理商资金校验模式响应
void TdSpiUser::OnRspQrySecAgentCheckMode(CThostFtdcSecAgentCheckModeField *pSecAgentCheckMode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询二级代理商信息响应
void TdSpiUser::OnRspQrySecAgentTradeInfo(CThostFtdcSecAgentTradeInfoField *pSecAgentTradeInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询期权交易成本响应
void TdSpiUser::OnRspQryOptionInstrTradeCost(CThostFtdcOptionInstrTradeCostField *pOptionInstrTradeCost, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询期权合约手续费响应
void TdSpiUser::OnRspQryOptionInstrCommRate(CThostFtdcOptionInstrCommRateField *pOptionInstrCommRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询执行宣告响应
void TdSpiUser::OnRspQryExecOrder(CThostFtdcExecOrderField *pExecOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询询价响应
void TdSpiUser::OnRspQryForQuote(CThostFtdcForQuoteField *pForQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询报价响应
void TdSpiUser::OnRspQryQuote(CThostFtdcQuoteField *pQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询期权自对冲响应
void TdSpiUser::OnRspQryOptionSelfClose(CThostFtdcOptionSelfCloseField *pOptionSelfClose, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询投资单元响应
void TdSpiUser::OnRspQryInvestUnit(CThostFtdcInvestUnitField *pInvestUnit, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询组合合约安全系数响应
void TdSpiUser::OnRspQryCombInstrumentGuard(CThostFtdcCombInstrumentGuardField *pCombInstrumentGuard, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询申请组合响应
void TdSpiUser::OnRspQryCombAction(CThostFtdcCombActionField *pCombAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询转帐流水响应
void TdSpiUser::OnRspQryTransferSerial(CThostFtdcTransferSerialField *pTransferSerial, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询银期签约关系响应
void TdSpiUser::OnRspQryAccountregister(CThostFtdcAccountregisterField *pAccountregister, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///错误应答
void TdSpiUser::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///报单通知
void TdSpiUser::OnRtnOrder(CThostFtdcOrderField *pOrder) {
    



};

///成交通知
void TdSpiUser::OnRtnTrade(CThostFtdcTradeField *pTrade) {};

///报单录入错误回报
void TdSpiUser::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo) {

    std::cout << " OnErrRtnOrderInsert ： 交易所前置返回报单错误" << std::endl;
    bool resultUpdate = pRspInfo && (pRspInfo ->ErrorID !=0);

    if(!resultUpdate){
    }else{
        event_order_status order_status("order_error", * pInputOrder);
        z_msg_send(socket_status, pInputOrder->InstrumentID, order_status);
    }
    
};

///报单操作错误回报
void TdSpiUser::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo) {};

///合约交易状态通知
void TdSpiUser::OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus) {};

///交易所公告通知
void TdSpiUser::OnRtnBulletin(CThostFtdcBulletinField *pBulletin) {};

///交易通知
void TdSpiUser::OnRtnTradingNotice(CThostFtdcTradingNoticeInfoField *pTradingNoticeInfo) {};

///提示条件单校验错误
void TdSpiUser::OnRtnErrorConditionalOrder(CThostFtdcErrorConditionalOrderField *pErrorConditionalOrder) {};

///执行宣告通知
void TdSpiUser::OnRtnExecOrder(CThostFtdcExecOrderField *pExecOrder) {};

///执行宣告录入错误回报
void TdSpiUser::OnErrRtnExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, CThostFtdcRspInfoField *pRspInfo) {};

///执行宣告操作错误回报
void TdSpiUser::OnErrRtnExecOrderAction(CThostFtdcExecOrderActionField *pExecOrderAction, CThostFtdcRspInfoField *pRspInfo) {};

///询价录入错误回报
void TdSpiUser::OnErrRtnForQuoteInsert(CThostFtdcInputForQuoteField *pInputForQuote, CThostFtdcRspInfoField *pRspInfo) {};

///报价通知
void TdSpiUser::OnRtnQuote(CThostFtdcQuoteField *pQuote) {};

///报价录入错误回报
void TdSpiUser::OnErrRtnQuoteInsert(CThostFtdcInputQuoteField *pInputQuote, CThostFtdcRspInfoField *pRspInfo) {};

///报价操作错误回报
void TdSpiUser::OnErrRtnQuoteAction(CThostFtdcQuoteActionField *pQuoteAction, CThostFtdcRspInfoField *pRspInfo) {};

///询价通知
void TdSpiUser::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp) {};

///保证金监控中心用户令牌
void TdSpiUser::OnRtnCFMMCTradingAccountToken(CThostFtdcCFMMCTradingAccountTokenField *pCFMMCTradingAccountToken) {};

///批量报单操作错误回报
void TdSpiUser::OnErrRtnBatchOrderAction(CThostFtdcBatchOrderActionField *pBatchOrderAction, CThostFtdcRspInfoField *pRspInfo) {};

///期权自对冲通知
void TdSpiUser::OnRtnOptionSelfClose(CThostFtdcOptionSelfCloseField *pOptionSelfClose) {};

///期权自对冲录入错误回报
void TdSpiUser::OnErrRtnOptionSelfCloseInsert(CThostFtdcInputOptionSelfCloseField *pInputOptionSelfClose, CThostFtdcRspInfoField *pRspInfo) {};

///期权自对冲操作错误回报
void TdSpiUser::OnErrRtnOptionSelfCloseAction(CThostFtdcOptionSelfCloseActionField *pOptionSelfCloseAction, CThostFtdcRspInfoField *pRspInfo) {};

///申请组合通知
void TdSpiUser::OnRtnCombAction(CThostFtdcCombActionField *pCombAction) {};

///申请组合录入错误回报
void TdSpiUser::OnErrRtnCombActionInsert(CThostFtdcInputCombActionField *pInputCombAction, CThostFtdcRspInfoField *pRspInfo) {};

///请求查询签约银行响应
void TdSpiUser::OnRspQryContractBank(CThostFtdcContractBankField *pContractBank, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询预埋单响应
void TdSpiUser::OnRspQryParkedOrder(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询预埋撤单响应
void TdSpiUser::OnRspQryParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询交易通知响应
void TdSpiUser::OnRspQryTradingNotice(CThostFtdcTradingNoticeField *pTradingNotice, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询经纪公司交易参数响应
void TdSpiUser::OnRspQryBrokerTradingParams(CThostFtdcBrokerTradingParamsField *pBrokerTradingParams, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询经纪公司交易算法响应
void TdSpiUser::OnRspQryBrokerTradingAlgos(CThostFtdcBrokerTradingAlgosField *pBrokerTradingAlgos, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///请求查询监控中心用户令牌
void TdSpiUser::OnRspQueryCFMMCTradingAccountToken(CThostFtdcQueryCFMMCTradingAccountTokenField *pQueryCFMMCTradingAccountToken, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///银行发起银行资金转期货通知
void TdSpiUser::OnRtnFromBankToFutureByBank(CThostFtdcRspTransferField *pRspTransfer) {};

///银行发起期货资金转银行通知
void TdSpiUser::OnRtnFromFutureToBankByBank(CThostFtdcRspTransferField *pRspTransfer) {};

///银行发起冲正银行转期货通知
void TdSpiUser::OnRtnRepealFromBankToFutureByBank(CThostFtdcRspRepealField *pRspRepeal) {};

///银行发起冲正期货转银行通知
void TdSpiUser::OnRtnRepealFromFutureToBankByBank(CThostFtdcRspRepealField *pRspRepeal) {};

///期货发起银行资金转期货通知
void TdSpiUser::OnRtnFromBankToFutureByFuture(CThostFtdcRspTransferField *pRspTransfer) {};

///期货发起期货资金转银行通知
void TdSpiUser::OnRtnFromFutureToBankByFuture(CThostFtdcRspTransferField *pRspTransfer) {};

///系统运行时期货端手工发起冲正银行转期货请求，银行处理完毕后报盘发回的通知
void TdSpiUser::OnRtnRepealFromBankToFutureByFutureManual(CThostFtdcRspRepealField *pRspRepeal) {};

///系统运行时期货端手工发起冲正期货转银行请求，银行处理完毕后报盘发回的通知
void TdSpiUser::OnRtnRepealFromFutureToBankByFutureManual(CThostFtdcRspRepealField *pRspRepeal) {};

///期货发起查询银行余额通知
void TdSpiUser::OnRtnQueryBankBalanceByFuture(CThostFtdcNotifyQueryAccountField *pNotifyQueryAccount) {};

///期货发起银行资金转期货错误回报
void TdSpiUser::OnErrRtnBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo) {};

///期货发起期货资金转银行错误回报
void TdSpiUser::OnErrRtnFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo) {};

///系统运行时期货端手工发起冲正银行转期货错误回报
void TdSpiUser::OnErrRtnRepealBankToFutureByFutureManual(CThostFtdcReqRepealField *pReqRepeal, CThostFtdcRspInfoField *pRspInfo) {};

///系统运行时期货端手工发起冲正期货转银行错误回报
void TdSpiUser::OnErrRtnRepealFutureToBankByFutureManual(CThostFtdcReqRepealField *pReqRepeal, CThostFtdcRspInfoField *pRspInfo) {};

///期货发起查询银行余额错误回报
void TdSpiUser::OnErrRtnQueryBankBalanceByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, CThostFtdcRspInfoField *pRspInfo) {};

///期货发起冲正银行转期货请求，银行处理完毕后报盘发回的通知
void TdSpiUser::OnRtnRepealFromBankToFutureByFuture(CThostFtdcRspRepealField *pRspRepeal) {};

///期货发起冲正期货转银行请求，银行处理完毕后报盘发回的通知
void TdSpiUser::OnRtnRepealFromFutureToBankByFuture(CThostFtdcRspRepealField *pRspRepeal) {};

///期货发起银行资金转期货应答
void TdSpiUser::OnRspFromBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///期货发起期货资金转银行应答
void TdSpiUser::OnRspFromFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///期货发起查询银行余额应答
void TdSpiUser::OnRspQueryBankAccountMoneyByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

///银行发起银期开户通知
void TdSpiUser::OnRtnOpenAccountByBank(CThostFtdcOpenAccountField *pOpenAccount) {};

///银行发起银期销户通知
void TdSpiUser::OnRtnCancelAccountByBank(CThostFtdcCancelAccountField *pCancelAccount) {};

///银行发起变更银行账号通知
void TdSpiUser::OnRtnChangeAccountByBank(CThostFtdcChangeAccountField *pChangeAccount) {};

