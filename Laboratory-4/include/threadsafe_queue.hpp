#pragma once
#include <cstdlib>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

template<typename T>
class threadsafe_queue
{
  private:
      mutable std::mutex _m; //mutable allows to modify an object in a const function member. We need this modification for the empty() function.   
      std::queue<T> _data_queue;
      std::condition_variable _cv;

  public:
    /*Empty constructor*/
    threadsafe_queue() {} 
    
    /*Normal Constructor*/
    threadsafe_queue(const threadsafe_queue& other)
    {
        std::lock_guard<std::mutex> lk(other._m);
        _data_queue=other._data_queue;
    }

    threadsafe_queue& operator=(const threadsafe_queue&) = delete;

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(_m); //Enter the critical section
        _data_queue.push(new_value);
        _cv.notify_one(); //Notify one thread, could we notify all of them?
    }

    /*This function DOES NOT BLOCK.
      > If the queue is empty, it returns FALSE, otherwise
      > it writes the front of the queue in 'value' and returns TRUE*/
    bool try_pop(T& value)
    {
	    std::lock_guard<std::mutex> lk(_m);
        if (!data_queue.empty()){ //if it's not empty
            value = _data_queue.front(); //extract value
            _data_queue.pop(); //pop value
            return true; 
        } else { //if it's empty
            return false;
        }
    }
    
    /*This fuction DOES BLOCK.
      > Same thing as try_pop, but doesn't return anything and blocks with wait() */
    void wait_and_pop(T& value)
    {
	    std::unique_lock<std::mutex> lk(_m);
        _cv.wait(lk, [this]{return !_data_queue.empty();});
        value = _data_queue.front();
        _data_queue.pop();
    }
    
    /*This fuction DOES BLOCK.
      > Same thing as wait_and_pop, but returns a shared pointer*/
    std::shared_ptr<T> wait_and_pop() //shared pointer: deletes the object it's pointing to when the pointer goes out of scope
    {
        std::unique_lock<std::mutex> lk(_m);
        _cv.wait(lk, [this]{return !_data_queue.empty();});
        std::shared_ptr<T> res(std::make_shared<T>(_data_queue.front()));
        _data_queue.pop();
        return res;
    }

    /*Constant function,
      returns true if the queue is empty, otherwise
      returns false*/
    bool empty() const
    {
        std::lock_guard<std::mutex> lk(_m);
        return _data_queue.empty();
    }
};
