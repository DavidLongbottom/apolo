
#pragma once
#include "../ipc/zmq_ipc/zmq_context_singleton.hpp"
#include "../ipc/zmq_ipc/zmq_socket_factory.hpp"
#include "../ipc/zmq_ipc/zmqhelper.hpp"

#include "../api/ctp/ThostFtdcMdApi.h"
#include "../api/ctp/ThostFtdcTraderApi.h"
#include "../api/ctp/ThostFtdcUserApiDataType.h"
#include "../api/ctp/ThostFtdcUserApiStruct.h"

#include "../event/event_engine.h"
#include "../event/event_tick.h"
#include "../event/event_order_status.h"
#include "../event/event_trade_status.h"



#include "../pojo/market_data.h"
#include "../pojo/bar.h"
#include "../pojo/proceeding_order.h"
#include "../util/bar_generator.h"
#include "../strategy/macd.h"
#include "../util/util_func.h"
#include "../util/config_processor.h"

