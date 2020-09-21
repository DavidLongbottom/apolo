#ifndef SYNC_QUEUE_HPP
#define SYNC_QUEUE_HPP

#include <queue>
#include <deque>
#include <mutex>
#include <condition_variable>


template <typename T, typename Container = std::deque<T>>
class sync_queue{

    typedef typename Container::value_type  value_type;
    typedef typename Container::size_type   size_type;

private:
    std::queue<T, Container> syn_q;
    std::mutex mtx;
    std::condition_variable cond;

public:

    bool push(const value_type & item){
        std::lock_guard<std::mutex> locker(mtx);
        syn_q.push(item);
        cond.notify_all();
        return true;
    }

    bool empty(){
        std::lock_guard<std::mutex> locker(mtx);
        return syn_q.empty();
    }

    value_type & get(){
        std::unique_lock<std::mutex> locker(mtx);
        cond.wait(locker, [this](){
            return !syn_q.empty();
        });
        value_type & item = syn_q.front();
        syn_q.pop();
        return item;
    }

};

#endif
