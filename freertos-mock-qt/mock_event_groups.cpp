extern "C"
{
    #include "FreeRTOS.h"
    #include "event_groups.h"
}
#include <QtCore>

struct EventGroup_t {
    QMutex mutex;
    QWaitCondition condition;
    QAtomicInt bits;
};

// Sets bits in the event group
EventBits_t xEventGroupSetBits(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet)
{
    EventGroup_t* group = (EventGroup_t*)xEventGroup;
    QMutexLocker locker(&group->mutex);
    group->bits |= uxBitsToSet;
    group->condition.wakeAll();
    return group->bits;
}

// Creates an event group
EventGroupHandle_t xEventGroupCreate()
{
    EventGroup_t* eventGroup = new EventGroup_t;
    eventGroup->bits = 0;
    return (EventGroupHandle_t)eventGroup;
}

void vEventGroupDelete( EventGroupHandle_t xEventGroup )
{
    EventGroup_t* group = (EventGroup_t*)xEventGroup;
    delete group;
}

// Clears bits in the event group
EventBits_t xEventGroupClearBits(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToClear)
{
    EventGroup_t* group = (EventGroup_t*)xEventGroup;
    QMutexLocker locker(&group->mutex);
    group->bits &= ~uxBitsToClear;
    return group->bits;
}

// Blocks until the specified bits are set in the event group
EventBits_t xEventGroupWaitBits(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToWaitFor,
                                const BaseType_t xClearOnExit, const BaseType_t xWaitForAllBits,
                                TickType_t xTicksToWait)
{
    EventGroup_t* group = (EventGroup_t*)xEventGroup;
    QMutexLocker locker(&group->mutex);

    while (true) {
        if (xWaitForAllBits) {
            if ((group->bits & uxBitsToWaitFor) == uxBitsToWaitFor) {
                if (xClearOnExit) {
                    group->bits &= ~uxBitsToWaitFor;
                }
                return group->bits;
            }
        } else {
            if (group->bits & uxBitsToWaitFor) {
                if (xClearOnExit) {
                    group->bits &= ~uxBitsToWaitFor;
                }
                return group->bits;
            }
        }

        if (xTicksToWait == 0) {
            return 0; // Timeout expired
        }

        if (!group->condition.wait(&group->mutex, xTicksToWait)) {
            return 0; // Timeout expired
        }
    }
}
