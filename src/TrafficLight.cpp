#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */
#include <chrono>
#include <random>
#include <future>

class Timer
{
public:
    Timer() : beg_(clock_::now()) {}
    void reset() { beg_ = clock_::now(); }
    double elapsed() const {
        return std::chrono::duration_cast<std::chrono::seconds>
        (clock_::now() - beg_).count(); }
    
private:
    typedef std::chrono::high_resolution_clock clock_;
   // typedef std::chrono::duration<double, std::ratio<1> > second_;
    std::chrono::time_point<clock_> beg_;
};

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] { return !_messages.empty(); }); // pass unique lock to condition variable
    
    // remove last vector element from queue
    T msg = std::move(_messages.back());
    _messages.pop_back();
    
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    
        // simulate some work
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // perform vector modification under the lock
        std::lock_guard<std::mutex> uLock(_mutex);
        
        // add vector to queue
        std::cout << "   Message " << msg << " has been sent to the queue" << std::endl;
        _messages.push_back(std::move(msg));
        _cond.notify_one(); // notify client after pushing new Vehicle into vector
    
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    sp = std::make_shared<MessageQueue<TrafficLightPhase>>();
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
        auto message = sp->receive();
        if(message == TrafficLightPhase::green)
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases ,this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    Timer timer;
    //std::vector<std::future<void>> futures;
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<>distr(4,6); //4 to 6 seconds
    int sleepinseconds = distr(eng);
  
    while(true)
    {
         std::cout << "toggling lights to...sleepinseconds  " << sleepinseconds  << " time elapsed "<< timer.elapsed() << std::endl;
        
        if(timer.elapsed() < sleepinseconds)
            continue;
        timer.reset();
        _currentPhase = (_currentPhase == TrafficLightPhase::red ) ? TrafficLightPhase::green : TrafficLightPhase::red;
    
       
        distr(eng);
        std::cout << "toggling lights to... " << _currentPhase << std::endl;
       // auto  message = _currentPhase;
        /*25 May 2020*/
        //Ratul - understand the std::async (like thread spawning)-> MethodName , Object of the method to be passed like
        //queue->MessageQueue(..) and the method parameter to be passed to the MethodName.
        //futures.emplace_back(std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::send, sp, std::move(message)));
        sp->send(std::move(_currentPhase));
        sleepinseconds = distr(eng);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        
    }
}


