/*#include "WorkStealingQueue.hpp"

TRE_NS_START

int32 WorkStealingQueue::Init(int32 capacity, void* buffer, usize bufferSize) 
{
    if ( (capacity & (capacity-1)) != 0) {
        return -2; // capacity must be a power of 2
    }

    usize minBufferSize = BufferSize(capacity);

    if (bufferSize < minBufferSize) {
        return -1; // inadequate buffer size
    }

    uint8_t* bufferNext = (uint8_t*) buffer;
    m_Entries = (Task**) bufferNext;
    bufferNext += capacity * sizeof(Task*);

    ASSERT( bufferNext - (uint8_t*)buffer != (intptr_t) minBufferSize );

    for(int32 iEntry=0; iEntry < capacity; iEntry+=1) {
        m_Entries[iEntry] = NULL;
    }

    printf("Queue initlised !\n");

    m_Top = 0;
    m_Bottom = 0;
    m_Capacity = capacity;

    return 0;
}

int32 WorkStealingQueue::Push(Task* job) 
{
    // TODO: assert that this is only ever called by the owning thread
    uint64_t jobIndex = m_Bottom;
    m_Entries[jobIndex & (m_Capacity-1)] = job;

    // Ensure the job is written before the m_Bottom increment is published.
    // A StoreStore memory barrier would also be necessary on platforms with a weak memory model.
    TASK_COMPILER_BARRIER;

    m_Bottom = jobIndex+1;
    return 0;
}

Task* WorkStealingQueue::Pop() 
{
    // TODO: assert that this is only ever called by the owning thread
    uint64_t bottom = m_Bottom - 1;
    m_Bottom = bottom;

    // Make sure m_Bottom is published before reading top.
    // Requires a full StoreLoad memory barrier, even on x86/64.
    TASK_MEMORY_BARRIER;

    uint64_t top = m_Top;
    if (top <= bottom) {
        Task* job = m_Entries[bottom & (m_Capacity-1)];

        if (top != bottom) {
            // still >0 jobs left in the queue
            return job;
        } else {
            // popping the last element in the queue
            if (!std::atomic_compare_exchange_strong(&m_Top, &top, top)) {
                // failed race against Steal()
                job = NULL;
            }

            m_Bottom = top+1;
            return job;
        }

    } else {
        // queue already empty
        m_Bottom = top;
        return NULL;
    }
}

Task* WorkStealingQueue::Steal() 
{
    // TODO: assert that this is never called by the owning thread
    uint64_t top    = m_Top;

    // Ensure top is always read before bottom.
    // A LoadLoad memory barrier would also be necessary on platforms with a weak memory model.
    TASK_COMPILER_BARRIER;

    uint64_t bottom = m_Bottom;

    if (top < bottom) {
        Task *job = m_Entries[top & (m_Capacity-1)];

        // CAS serves as a compiler barrier as-is.
        if (!std::atomic_compare_exchange_strong(&m_Top, &top, top+1)) {
            // concurrent Steal()/Pop() got this entry first.
            return NULL;
        }

        m_Entries[top & (m_Capacity-1)] = NULL;
        return job;
    } else {
        return NULL; // queue empty
    }
}

TRE_NS_END*/