extern "C"
{
    #include "freertos/FreeRTOS.h"
    #include "freertos/event_groups.h"
}
#include <mutex>
#include <condition_variable>
#include <atomic>

struct EventGroup_t {
    std::mutex mutex;
    std::condition_variable condition;
    std::atomic<EventBits_t> bits;
};

EventBits_t xEventGroupSetBits(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet)
{
    EventGroup_t* group = reinterpret_cast<EventGroup_t*>(xEventGroup);
    std::unique_lock<std::mutex> locker(group->mutex);
    group->bits |= uxBitsToSet;
    group->condition.notify_all();
    return group->bits.load();
}

EventGroupHandle_t xEventGroupCreate()
{
    EventGroup_t* eventGroup = new EventGroup_t;
    eventGroup->bits.store(0);
    return reinterpret_cast<EventGroupHandle_t>(eventGroup);
}

void vEventGroupDelete(EventGroupHandle_t xEventGroup)
{
    EventGroup_t* group = reinterpret_cast<EventGroup_t*>(xEventGroup);
    delete group;
}

EventBits_t xEventGroupClearBits(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToClear)
{
    EventGroup_t* group = reinterpret_cast<EventGroup_t*>(xEventGroup);
    std::unique_lock<std::mutex> locker(group->mutex);
    group->bits &= ~uxBitsToClear;
    return group->bits.load();
}

EventBits_t xEventGroupWaitBits(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToWaitFor,
                                const BaseType_t xClearOnExit, const BaseType_t xWaitForAllBits,
                                TickType_t xTicksToWait)
{
    EventGroup_t* group = reinterpret_cast<EventGroup_t*>(xEventGroup);
    std::unique_lock<std::mutex> locker(group->mutex);

    while (true) {
        if (xWaitForAllBits) {
            if ((group->bits.load() & uxBitsToWaitFor) == uxBitsToWaitFor) {
                if (xClearOnExit) {
                    group->bits.fetch_and(~uxBitsToWaitFor);
                }
                return group->bits.load();
            }
        } else {
            if (group->bits.load() & uxBitsToWaitFor) {
                if (xClearOnExit) {
                    group->bits.fetch_and(~uxBitsToWaitFor);
                }
                return group->bits.load();
            }
        }

        if (xTicksToWait == 0) {
            return 0; // Timeout expired
        }

        if (group->condition.wait_for(locker, std::chrono::milliseconds(xTicksToWait)) == std::cv_status::timeout) {
            return 0; // Timeout expired
        }
    }
}
