CTP_HEADER := ../api/ctp
CTP_LIB_DIR := ../api/ctp 
USER_LIB := ../api/ctp/thostmduserapi_se.so
TRADER_LIB := ../api/ctp/thosttraderapi_se.so
EVENT_HEADER := ../event

exec:
	g++ -w -std=c++17 -I .  -I ../util/ -I $(CTP_HEADER) -L $(CTP_LIB_DIR)  $(USER_LIB) $(TRADER_LIB)  -l zmq  -l ta_lib ../util/config_processor.cpp ../util/bar_generator.cpp MdSpiUser.cpp main_md.cpp -o main_md
	g++ -w -std=c++17 -I .  -I ../util/ -I $(CTP_HEADER) -I $(EVENT_HEADER) -L $(CTP_LIB_DIR)  $(USER_LIB) $(TRADER_LIB)  -l zmq -l ta_lib -lpthread ../util/config_processor.cpp ../util/bar_generator.cpp ../util/util_func.cpp  ../ipc/zmq_ipc/zmq_context_singleton.hpp ../ipc/zmq_ipc/zmq_socket_factory.hpp ../ipc/zmq_ipc/zmqhelper.hpp main_preprocess.cpp -o main_preprocess
	g++ -w -std=c++17 -I .  -I ../util/ -I $(CTP_HEADER) -I $(EVENT_HEADER) -L $(CTP_LIB_DIR)  $(USER_LIB) $(TRADER_LIB)  -l zmq -l ta_lib -lpthread ../util/config_processor.cpp ../util/bar_generator.cpp ../util/util_func.cpp  ../ipc/zmq_ipc/zmq_context_singleton.hpp ../ipc/zmq_ipc/zmq_socket_factory.hpp ../ipc/zmq_ipc/zmqhelper.hpp main_strategy.cpp -o strategy
	g++ -w -std=c++17 -I .  -I ../util/ -I $(CTP_HEADER) -I $(EVENT_HEADER) -L $(CTP_LIB_DIR)  $(USER_LIB) $(TRADER_LIB)  -l zmq -l ta_lib -lpthread ../util/config_processor.cpp ../util/bar_generator.cpp ../util/util_func.cpp  ../ipc/zmq_ipc/zmq_context_singleton.hpp ../ipc/zmq_ipc/zmq_socket_factory.hpp ../ipc/zmq_ipc/zmqhelper.hpp ../event/event.cpp ../event/event_engine.cpp ../event/event_order_status.cpp ../event/event_tick.cpp ../event/event_trade_status.cpp ../event/sync_queue.hpp TdSpiUser.cpp main_trade.cpp -o trade

.PHONY: clean
clean:
	rm ./build/client
	rm ./build/server
