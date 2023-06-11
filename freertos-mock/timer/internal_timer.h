#pragma once
#include "cpptime.h"
#include "portmacro.h"

using namespace std::chrono;

typedef void (*callback_function_t)(void*);

class InternalTimerHandle
{
public:
    InternalTimerHandle& operator=(const InternalTimerHandle&t)
    {
        period=t.period;
        single_shot=t.single_shot;
        pxCallbackFunction=t.pxCallbackFunction;
        active=t.active;
        arg=t.arg;
        pcTimerName=t.pcTimerName;
        expiry_time=t.expiry_time;
    }

    InternalTimerHandle(int set_period=0,
                        bool set_single_shot=false,
                        callback_function_t callback=nullptr,
                        void* set_arg=nullptr,
                        const char* name="") :
        m{},
        period(set_period),
        single_shot(set_single_shot),
        pxCallbackFunction(callback),
        active(false),
        arg(set_arg),
        pcTimerName(name),
        expiry_time(0)
    {
    }

    void start(void)
    {
        stop();
        scoped_m lock(m);
        if(single_shot)
        {
            id = xtimer().add(milliseconds(period),
                                         [this](CppTime::timer_id) {
                                             if (pxCallbackFunction) {
                                                pxCallbackFunction(arg);
                                             }
                                             active = false;
                                         });
            active = true;
        }
        else
        {
            id = xtimer().add(milliseconds(period),
                [this](CppTime::timer_id) {
                    if (pxCallbackFunction) {
                        pxCallbackFunction(arg);
                    }
                }, milliseconds(period));
            active = true;
        }
        expiry_time = port_get_time_ms() + period;
    }

    void stop(void)
    {
        scoped_m lock(m);
        if(active)
        {
            xtimer().remove(id);
            active = false;
        }
    }

    void reset(void)
    {
        stop();
        start();
    }

    void setPeriod(int new_period)
    {
        scoped_m lock(m);
        if(active)
        {
            stop();
        }
        period = new_period;
        if(active)
        {
            start();
        }
    }

    void setSingleShot(bool set_single_shot)
    {
        scoped_m lock(m);
        if(active)
        {
            stop();
        }
        single_shot = set_single_shot;
        if(active)
        {
            start();
        }
    }

    static CppTime::Timer& xtimer() {
        static CppTime::Timer _xtimer;
        return _xtimer;
    }

    using scoped_m = std::unique_lock<std::mutex>;
    uint64_t expiry_time;
    std::mutex m;
    CppTime::timer_id id;
    int period;
    bool single_shot;
    callback_function_t pxCallbackFunction;
    bool active;
    void *arg;
    const char* pcTimerName;
};
