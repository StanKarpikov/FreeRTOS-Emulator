#include <condition_variable>
#include <deque>
#include <mutex>
#include <cstring>

extern "C"
{
    #include "FreeRTOS.h"
    #include "freertos/queue.h"
}

#include <QDebug>
#include <QQueue>
#include <QMutex>
#include <QSemaphore>

class TimedDeque {
public:
    explicit TimedDeque(size_t maxElements, size_t elementSize)
        : maxElements_(maxElements), elementSize_(elementSize) {}

    bool PushFront(const void* element, int timeoutMs) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!condFull_.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]() { return deque_.size() < maxElements_; })) {
            // Timeout occurred
//            qDebug() << "Timeout occurred while waiting to add element to the front of the deque.";
            return false;
        }
        void* buffer = AllocateBuffer();
        std::memcpy(buffer, element, elementSize_);
        deque_.push_front(buffer);
        condEmpty_.notify_all();
        return true;
    }

    bool PushBack(const void* element, int timeoutMs) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!condFull_.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]() { return deque_.size() < maxElements_; })) {
            // Timeout occurred
//            qDebug() << "Timeout occurred while waiting to add element to the back of the deque.";
            return false;
        }
        void* buffer = AllocateBuffer();
        std::memcpy(buffer, element, elementSize_);
        deque_.push_back(buffer);
        condEmpty_.notify_all();
        return true;
    }

    bool PopFront(void* destination, int timeoutMs) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!condEmpty_.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]() { return !deque_.empty(); })) {
            // Timeout occurred
//            qDebug() << "Timeout occurred while waiting to pop element from the front of the deque.";
            return false;
        }
        void* frontBuffer = deque_.front();
        std::memcpy(destination, frontBuffer, elementSize_);
        FreeBuffer(frontBuffer);
        deque_.pop_front();
        condFull_.notify_all();
        return true;
    }

    bool PopBack(void* destination, int timeoutMs) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!condEmpty_.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]() { return !deque_.empty(); })) {
            // Timeout occurred
//            qDebug() << "Timeout occurred while waiting to pop element from the back of the deque.";
            return false;
        }
        void* backBuffer = deque_.back();
        std::memcpy(destination, backBuffer, elementSize_);
        FreeBuffer(backBuffer);
        deque_.pop_back();
        condFull_.notify_all();
        return true;
    }

    bool OverwriteLast(const void* element, int timeoutMs) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!condEmpty_.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]() { return !deque_.empty(); })) {
            // Timeout occurred
//            qDebug() << "Timeout occurred while waiting to overwrite the last element in the deque.";
            return false;
        }
        void* backBuffer = deque_.back();
        std::memcpy(backBuffer, element, elementSize_);
        condEmpty_.notify_all();
        return true;
    }

private:
    std::deque<void*> deque_;
    const size_t maxElements_;
    const size_t elementSize_;
    std::mutex mutex_;
    std::condition_variable condFull_;
    std::condition_variable condEmpty_;

    void* AllocateBuffer() {
        void* buffer = new char[elementSize_];
        return buffer;
    }

    void FreeBuffer(void* buffer) {
        delete[] static_cast<char*>(buffer);
    }
};

typedef struct QueueDefinition /* The old naming convention is used to prevent breaking kernel aware debuggers. */
{
    union{
        TimedDeque *pQueue;
        QMutex *pMutex;
        QSemaphore *pSemaphore;
        QRecursiveMutex *pRecursiveMutex;
    } u;

    UBaseType_t uxLength;                   /*< The length of the queue defined as the number of items it will hold, not the number of bytes. */
    UBaseType_t uxItemSize;                 /*< The size of each items that the queue will hold. */

    UBaseType_t ucQueueType;

    QMutex mutex;
} xQUEUE;

QueueHandle_t xQueueGenericCreate( const UBaseType_t uxQueueLength,
                                  const UBaseType_t uxItemSize,
                                  const uint8_t ucQueueType )
{
    Q_UNUSED(uxItemSize);
    xQUEUE* queue = new xQUEUE();

    switch (ucQueueType) {
        case queueQUEUE_TYPE_BASE:  // queueQUEUE_TYPE_BASE / queueQUEUE_TYPE_SET
            queue->u.pQueue = new TimedDeque(uxQueueLength, uxItemSize);
            break;
        case queueQUEUE_TYPE_MUTEX:  // queueQUEUE_TYPE_MUTEX
            queue->u.pMutex = new QMutex();
            break;
        case queueQUEUE_TYPE_COUNTING_SEMAPHORE:  // queueQUEUE_TYPE_COUNTING_SEMAPHORE
            queue->u.pSemaphore = new QSemaphore(uxQueueLength);
            break;
        case queueQUEUE_TYPE_BINARY_SEMAPHORE:  // queueQUEUE_TYPE_BINARY_SEMAPHORE
            queue->u.pSemaphore = new QSemaphore(1);
            break;
        case queueQUEUE_TYPE_RECURSIVE_MUTEX:  // queueQUEUE_TYPE_RECURSIVE_MUTEX
            queue->u.pRecursiveMutex = new QRecursiveMutex();
            break;
        default:
            qDebug() << "Unexpected queue type (xQueueGenericCreate) " << ucQueueType;
            abort();
            return nullptr;
    }

    queue->ucQueueType = ucQueueType;
    queue->uxItemSize = uxItemSize;
    queue->uxLength = ucQueueType;

    return queue;
}

QueueHandle_t xQueueCreateMutex( const uint8_t ucQueueType )
{
    QueueHandle_t xNewQueue;
    const UBaseType_t uxMutexLength = ( UBaseType_t ) 1, uxMutexSize = ( UBaseType_t ) 0;

    xNewQueue = xQueueGenericCreate( uxMutexLength, uxMutexSize, queueQUEUE_TYPE_MUTEX );

    return xNewQueue;
}

BaseType_t xQueueTakeMutexRecursive( QueueHandle_t xMutex,
                             TickType_t xTicksToWait )
{
    bool success = false;
    QSemaphore* sem = xMutex->u.pSemaphore;
    QMutex* mutex = xMutex->u.pMutex;
    QRecursiveMutex* rec_mutex = xMutex->u.pRecursiveMutex;

    switch (xMutex->ucQueueType) {
        case queueQUEUE_TYPE_RECURSIVE_MUTEX:
            if (xTicksToWait == portMAX_DELAY) {
                mutex->lock();
                success = true;
            } else {
                success = mutex->tryLock(pdTICKS_TO_MS(xTicksToWait));
            }
            break;
        case queueQUEUE_TYPE_MUTEX:
            if (xTicksToWait == portMAX_DELAY) {
                mutex->lock();
                success = true;
            } else {
                success = mutex->tryLock(pdTICKS_TO_MS(xTicksToWait));
            }
            break;
        case queueQUEUE_TYPE_BINARY_SEMAPHORE:
        case queueQUEUE_TYPE_COUNTING_SEMAPHORE:
            if (xTicksToWait == portMAX_DELAY) {
                sem->acquire(1);
                success = true;
            } else {
                success = sem->tryAcquire(1, pdTICKS_TO_MS(xTicksToWait));
            }
            break;
        case queueQUEUE_TYPE_BASE:
        default:
            qDebug() << "Unexpected queue type (xQueueTakeMutexRecursive) " << xMutex->ucQueueType << "; Acceptable types are: " << queueQUEUE_TYPE_RECURSIVE_MUTEX;
            abort();
            return pdFAIL;
    }

    return (success ? pdPASS : pdFAIL);
}

BaseType_t xQueueSemaphoreTake( QueueHandle_t xQueue,
                               TickType_t xTicksToWait )
{
    QSemaphore* sem = xQueue->u.pSemaphore;
    QMutex* mutex = xQueue->u.pMutex;
    QRecursiveMutex* rec_mutex = xQueue->u.pRecursiveMutex;
    bool success = false;

    switch (xQueue->ucQueueType) {
        case queueQUEUE_TYPE_BINARY_SEMAPHORE:
        case queueQUEUE_TYPE_COUNTING_SEMAPHORE:
            if (xTicksToWait == portMAX_DELAY) {
                sem->acquire(1);
                success = true;
            } else {
                success = sem->tryAcquire(1, pdTICKS_TO_MS(xTicksToWait));
            }
            break;
        case queueQUEUE_TYPE_MUTEX:
            if (xTicksToWait == portMAX_DELAY) {
                mutex->lock();
                success = true;
            } else {
                success = mutex->tryLock(pdTICKS_TO_MS(xTicksToWait));
            }
            break;
        case queueQUEUE_TYPE_RECURSIVE_MUTEX:
            if (xTicksToWait == portMAX_DELAY) {
                rec_mutex->lock();
                success = true;
            } else {
                success = rec_mutex->tryLock(pdTICKS_TO_MS(xTicksToWait));
            }
            break;
        case queueQUEUE_TYPE_BASE:
        default:
            qDebug() << "Unexpected queue type (xQueueSemaphoreTake) " << xQueue->ucQueueType
                     << "; Acceptable types are: "
                     << queueQUEUE_TYPE_BINARY_SEMAPHORE << ","
                     << queueQUEUE_TYPE_COUNTING_SEMAPHORE << ","
                     << queueQUEUE_TYPE_MUTEX << ","
                     << queueQUEUE_TYPE_RECURSIVE_MUTEX;
            abort();
            return pdFAIL;
    }
    return (success ? pdPASS : pdFAIL);
}

BaseType_t xQueueGenericSend( QueueHandle_t xQueue,
                             const void * const pvItemToQueue,
                             TickType_t xTicksToWait,
                             const BaseType_t xCopyPosition )
{
    QSemaphore* sem = xQueue->u.pSemaphore;
    TimedDeque *queue = xQueue->u.pQueue;
    QMutex* mutex = xQueue->u.pMutex;
    QRecursiveMutex* rec_mutex = xQueue->u.pRecursiveMutex;
    bool success = false;
    void* element = NULL;

    switch (xQueue->ucQueueType) {
        case queueQUEUE_TYPE_BASE:
            switch(xCopyPosition){
                case queueSEND_TO_BACK:
                    success = queue->PushBack(pvItemToQueue, pdTICKS_TO_MS(xTicksToWait));
                    break;
                case queueSEND_TO_FRONT:
                    success = queue->PushFront(pvItemToQueue, pdTICKS_TO_MS(xTicksToWait));
                    break;
                case queueOVERWRITE:
                    success = queue->OverwriteLast(pvItemToQueue, pdTICKS_TO_MS(xTicksToWait));
                    break;
                default:
                    break;
            }
            break;
        case queueQUEUE_TYPE_MUTEX:
            mutex->unlock();
            success = true;
            break;
        case queueQUEUE_TYPE_RECURSIVE_MUTEX:
            rec_mutex->unlock();
            success = true;
            break;
        case queueQUEUE_TYPE_COUNTING_SEMAPHORE:
        case queueQUEUE_TYPE_BINARY_SEMAPHORE:
            sem->release(1);
            success = true;
            break;
        default:
            qDebug() << "Unexpected queue type (xQueueGenericSend) " << xQueue->ucQueueType;
            abort();
            return pdFAIL;
    }

    return (success ? pdPASS : pdFAIL);
}

BaseType_t xQueueGenericSendFromISR( QueueHandle_t xQueue,
                                    const void * const pvItemToQueue,
                                    BaseType_t * const pxHigherPriorityTaskWoken,
                                    const BaseType_t xCopyPosition )
{
    return xQueueGenericSend(xQueue, pvItemToQueue, 0, xCopyPosition);
}

BaseType_t xQueueReceive(QueueHandle_t xQueue,
                         void * const pvBuffer,
                         TickType_t xTicksToWait )
{
    TimedDeque *queue = xQueue->u.pQueue;
    bool success = false;
    void* element = NULL;

    switch (xQueue->ucQueueType) {
        case queueQUEUE_TYPE_BASE:
            success = queue->PopBack(pvBuffer, pdTICKS_TO_MS(xTicksToWait));
            break;
        case queueQUEUE_TYPE_BINARY_SEMAPHORE:
        case queueQUEUE_TYPE_COUNTING_SEMAPHORE:
        case queueQUEUE_TYPE_MUTEX:
        case queueQUEUE_TYPE_RECURSIVE_MUTEX:
        default:
            qDebug() << "Unexpected queue type (xQueueReceive) " << xQueue->ucQueueType
                     << "; Acceptable types are: "
                     << queueQUEUE_TYPE_BASE;
            abort();
            return pdFAIL;
    }

    return (success ? pdPASS : pdFAIL);
}

QueueHandle_t xQueueCreateMutexStatic( const uint8_t ucQueueType,
                                      StaticQueue_t * pxStaticQueue )
{
    return xQueueCreateMutex(ucQueueType);
}

QueueHandle_t xQueueGenericCreateStatic( const UBaseType_t uxQueueLength,
                                        const UBaseType_t uxItemSize,
                                        uint8_t * pucQueueStorage,
                                        StaticQueue_t * pxStaticQueue,
                                        const uint8_t ucQueueType )
{
    return xQueueGenericCreate(uxQueueLength, uxItemSize, ucQueueType);
}

BaseType_t xQueueGiveMutexRecursive( QueueHandle_t xMutex )
{
    QRecursiveMutex* mutex = xMutex->u.pRecursiveMutex;
    QRecursiveMutex* rec_mutex = xMutex->u.pRecursiveMutex;
    QSemaphore* sem = xMutex->u.pSemaphore;
    bool success = false;

    switch (xMutex->ucQueueType)
    {
        case queueQUEUE_TYPE_MUTEX:
            mutex->unlock();
            success = true;
            break;
        case queueQUEUE_TYPE_RECURSIVE_MUTEX:
            rec_mutex->unlock();
            success = true;
            break;
        case queueQUEUE_TYPE_BINARY_SEMAPHORE:
        case queueQUEUE_TYPE_COUNTING_SEMAPHORE:
            sem->release(1);
            break;
        case queueQUEUE_TYPE_BASE:
        default:
            qDebug() << "Unexpected queue type (xQueueReceive) " << xMutex->ucQueueType
                     << "; Acceptable types are: "
                     << queueQUEUE_TYPE_RECURSIVE_MUTEX << ","
                     << queueQUEUE_TYPE_BINARY_SEMAPHORE << ","
                     << queueQUEUE_TYPE_COUNTING_SEMAPHORE;
            abort();
            return pdFAIL;
    }

    return (success ? pdPASS : pdFAIL);
}

BaseType_t xQueueGiveFromISR(QueueHandle_t xQueue,
                             BaseType_t * const pxHigherPriorityTaskWoken )
{
    QSemaphore* sem = xQueue->u.pSemaphore;
    QMutex* mutex = xQueue->u.pMutex;
    QRecursiveMutex* rec_mutex = xQueue->u.pRecursiveMutex;
    bool success = false;

    switch (xQueue->ucQueueType)
    {
        case queueQUEUE_TYPE_RECURSIVE_MUTEX:
            rec_mutex->unlock();
            success = true;
            break;
        case queueQUEUE_TYPE_MUTEX:
            mutex->unlock();
            success = true;
            break;
        case queueQUEUE_TYPE_BINARY_SEMAPHORE:
        case queueQUEUE_TYPE_COUNTING_SEMAPHORE:
            sem->release(1);
            success = true;
            break;
        case queueQUEUE_TYPE_BASE:
        default:
            qDebug() << "Unexpected queue type (xQueueGiveFromISR) " << xQueue->ucQueueType
                     << "; Acceptable types are: "
                     << queueQUEUE_TYPE_RECURSIVE_MUTEX << ""
                     << queueQUEUE_TYPE_BINARY_SEMAPHORE << ""
                     << queueQUEUE_TYPE_COUNTING_SEMAPHORE << ""
                     << queueQUEUE_TYPE_MUTEX;
            abort();
            return pdFAIL;
    }

    return (success ? pdPASS : pdFAIL);
}

void vQueueDelete( QueueHandle_t xQueue )
{
    delete xQueue->u.pQueue;
    delete xQueue;
}

BaseType_t xQueueAddToSet( QueueSetMemberHandle_t xQueueOrSemaphore,
                          QueueSetHandle_t xQueueSet )
{
    qDebug() << "xQueueAddToSet not implemented";
    abort();
    return pdPASS;
}

BaseType_t xQueueRemoveFromSet( QueueSetMemberHandle_t xQueueOrSemaphore,
                               QueueSetHandle_t xQueueSet )
{
    qDebug() << "xQueueRemoveFromSet not implemented";
    abort();
    return pdPASS;
}
