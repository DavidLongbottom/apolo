#include <fstream>
#include <assert.h>
#include <thread>
#include <memory>


#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include <stdlib.h>
#include "TdSpiUser.h"


//global variables

TThostFtdcBrokerIDType brokerID ;
TThostFtdcInvestorIDType investorID;
TThostFtdcPasswordType investorPassword;
char mdFrontAddress[128];
char tdFrontAddress[128];

CThostFtdcMdApi *pMdApiUser =nullptr;
CThostFtdcTraderApi *pTdApiUser = nullptr;
TdSpiUser *pTdSpiUser = nullptr;

// 会话参数
TThostFtdcFrontIDType	trade_front_id;	//前置编号
TThostFtdcSessionIDType	session_id;	//会话编号
TThostFtdcOrderRefType	order_ref;	//报单引用


char *instrumentID[] = {"rb2009","rb2011"};
int instrumentNum = 2;



void init_account_info(std::string filename);
void msg_processor();
void schedule();
void trade_init(){
	pTdSpiUser = new TdSpiUser();
	pTdApiUser = CThostFtdcTraderApi::CreateFtdcTraderApi();
	pTdApiUser -> RegisterSpi(pTdSpiUser);
	pTdApiUser -> RegisterFront(tdFrontAddress);
	pTdApiUser ->Init();
	std::cout << "Td started ......." << std::endl;
    
    pTdApiUser ->Join();
	delete pTdSpiUser;
	pTdApiUser ->Release();
}



// 这个目的是处理和 trade 相关的内容
int main(){
    
    init_account_info("account.properties");
    
    trade_init();
    //std::thread t1(trade_init);
    sleep(5);

    std::thread t2(schedule);
    
    msg_processor();



}

void init_account_info(std::string filename){
	config_processor cp = config_processor(filename);
	strcpy(brokerID, cp.get_value("brokerID").c_str());
	strcpy(investorID, cp.get_value("investorID").c_str());
	strcpy(investorPassword, cp.get_value("investorPassword").c_str());
	strcpy(mdFrontAddress, cp.get_value("mdFrontAddress").c_str());
	strcpy(tdFrontAddress, cp.get_value("tdFrontAddress").c_str());
}



std::string get_order_id(int order_ref){
    std::string order_id;
    order_id.append(boost::lexical_cast<std::string>(trade_front_id));
    order_id.append(boost::lexical_cast<std::string>(session_id));
    order_id.append(boost::lexical_cast<std::string>(order_ref));
    return order_id;
}



void find_order(std::map<std::string, std::map<std::string, proceeding_order>> & order_record, std::string instrument_id, std::string order_ref, std::shared_ptr<proceeding_order>  order){
    std::string order_id;
    order_id.append(boost::lexical_cast<std::string>(trade_front_id));
    order_id.append(boost::lexical_cast<std::string>(session_id));
    order_id.append(boost::lexical_cast<std::string>(order_ref));
    
    std::map<std::string, std::map<std::string, proceeding_order>>::iterator ite = order_record.find(instrument_id);
    if(ite == order_record.end()) return;

    order_id = get_order_id(boost::lexical_cast<int>(order_ref));
    std::map<std::string, proceeding_order>::iterator sub_ite = ite->second.find(order_id);
    if(sub_ite == ite->second.end()) return;

    order = std::make_shared<proceeding_order>(sub_ite->second);
    

}


void msg_processor(){

    std::map<std::string, std::vector<CThostFtdcDepthMarketDataField *>> tick_map;
    std::map<std::string, std::vector<kbar *>> m_bar_map;

    // 第一个instrument id, 第二个用orderref 生成的id, 第三个单字简略信息
    std::map<std::string, std::map<std::string, proceeding_order>> order_record;






    zmq_context_singleton & singleton = zmq_context_singleton::singleton();
    void * context = singleton.get_singleton_context();

    // 因为下单以后要一直的监控，所以需要实时的tick 以及bar
    void * socket_tick = zmq_socket_factory::get_socket(context, "tick", 0);
    void * socket_bar = zmq_socket_factory::get_socket(context, "bar", 0);
    void * socket_suggestion = zmq_socket_factory::get_socket(context, "suggestion", 0);
    void * socket_status = zmq_socket_factory::get_socket(context, "status", 0);
    void * socket_order = zmq_socket_factory::get_socket(context, "order", 1);


    zmq_pollitem_t items[] ={
        {socket_suggestion, 0, ZMQ_POLLIN, 0},
        {socket_tick, 0, ZMQ_POLLIN, 0},
        {socket_bar, 0, ZMQ_POLLIN, 0},
        {socket_status, 0, ZMQ_POLLIN, 0}
    };

    std::cout << "trade test " << std::endl;
    int load_history_flag = 0;
    std::vector<double> v;

    int i = 0;
    while(i < 30){
        
        zmq_poll(items, 1, -1);

        // socket_suggestion
        if(items[0].revents & ZMQ_POLLIN){
            std::pair<std::string, int *> pa = z_msg_recv<int>(socket_suggestion);
            // 进行风控检查
            // 确定自成交的风险 ---- socket_status 里面更新了所有的正在进行的单子， 这里直接获取一下判断就可以了

            //获取tick 数据， 然后确定价格以及数量--- 可能需要使用var
            // 因为如果最后实现了对某一个合约的下单，那么肯定tick 数据是已经存在的， 所以可以用来确定价格
            std::map<std::string, std::vector<CThostFtdcDepthMarketDataField *>>::iterator ite  = tick_map.find(pa.first);
            if(ite == tick_map.end()) continue;

            //构造下单的数据
            int size = ite->second.size();
            double price = (ite->second[size-1]->AskPrice1 + ite->second[size-1]->BidPrice1)/2;
            double volume = (ite->second[size-1]->AskVolume1 +  ite->second[size-1]->BidVolume1)/2;


            CThostFtdcInputOrderField order_insert;
            memset(&order_insert, 0, sizeof(order_insert));
            strcpy(order_insert.BrokerID, brokerID);
            strcpy(order_insert.InvestorID, investorID);
            strcpy(order_insert.InstrumentID, pa.first.c_str());
            strcpy(order_insert.OrderRef, order_ref);

            order_insert.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
            if(* pa.second  == 1){
                order_insert.Direction = THOST_FTDC_D_Buy;
            }else{
                order_insert.Direction = THOST_FTDC_D_Sell;
            }
            order_insert.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
            order_insert.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
            order_insert.LimitPrice = price;
            order_insert.VolumeTotalOriginal = volume;
            order_insert.TimeCondition = THOST_FTDC_TC_GFD;
            order_insert.VolumeCondition = THOST_FTDC_VC_AV;
            order_insert.MinVolume = 1;
            order_insert.ContingentCondition = THOST_FTDC_CC_Immediately;
            order_insert.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
            order_insert.IsAutoSuspend = 0;
            order_insert.UserForceClose = 0;

            int request_id = 0;
            int r = pTdApiUser ->ReqOrderInsert(&order_insert, request_id);
            if(!r){
                std::cout << "send order successful" << std::endl;
                z_msg_send(socket_order, pa.first, order_insert);
            }else{
                std::cout << "send order failed" << std::endl;
            }

        }
        // socket_tick
        if(items[1].revents & ZMQ_POLLIN){
            std::pair<std::string, CThostFtdcDepthMarketDataField  *> pa = z_msg_recv<CThostFtdcDepthMarketDataField >(socket_bar);    
            std::map<std::string, std::vector<CThostFtdcDepthMarketDataField *>>::iterator ite  = tick_map.find(pa.first);
            if(ite == tick_map.end()){
                std::vector<CThostFtdcDepthMarketDataField *> v;
                v.push_back(pa.second);
                tick_map.insert(std::make_pair(pa.first, v));
            }else{
                ite ->second.push_back(pa.second);
            }

        }

        //socket_bar
        if(items[2].revents & ZMQ_POLLIN){

            std::pair<std::string, kbar *> pa = z_msg_recv<kbar>(socket_bar);    
            std::map<std::string, std::vector<kbar *>>::iterator ite  = m_bar_map.find(pa.first);
            if(ite == m_bar_map.end()){
                std::vector<kbar *> v;
                v.push_back(pa.second);
                m_bar_map.insert(std::make_pair(pa.first, v));
            }else{
                ite ->second.push_back(pa.second);
            }

        }
        

        // socket_status ---  这个处理已经发出去的单， 进行处理OnRtnOrder 以及OnRtnTrade 以及更新资金， 更新持仓状态 
        if(items[3].revents & ZMQ_POLLIN){
            std::pair<std::string, event *> pa = z_msg_recv<event>(socket_status);
            if(pa.second->get_event_type()=="order_error"){
                event_order_status * order_status = static_cast<event_order_status *>(pa.second);
                CThostFtdcInputOrderField order_field = order_status->get_event_data();
                //接收到这个要把目前本地维护的对应的instrument 正在进行的单数量设置为0
                
                std::shared_ptr<proceeding_order> order;
                find_order(order_record,order_status->get_event_data().InstrumentID, order_status->get_event_data().OrderRef, order);
                if(order != nullptr){
                    order->status = -1;
                    order->left_volume = 0;
                }
                
            }


            






        }

    }

    zmq_socket_factory::close_socket(socket_bar);
    singleton.close_context();
}




void schedule(){

    float interval = 1;    // 最小时间间隔
    int interval_count = 1;  // 可以为每一个任务设定count ，  从而调整时间的间隔

    while(true){

        // 定时任务 --- 查询目前正在有什么单， 以及单的时间是多少
        
        CThostFtdcQryInstrumentField instrumentReq;
        memset(&instrumentReq, 0, sizeof(instrumentReq));
        strcpy(instrumentReq.InstrumentID, "cu");//全部合约

		int requestID = 0;
	    int rt = pTdApiUser->ReqQryInstrument(&instrumentReq, requestID);
        if (!rt)
            std::cout << ">>>>>>发送合约查询请求成功" << std::endl;
        else
            std::cerr << "--->>>发送合约查询请求失败" << std::endl;
		sleep(10);
	}

}








//////////////////////////////////////////////////////////////////

