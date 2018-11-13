#include <pthread.h>
#include "orgQueue.h"

pthread_mutex_t enqueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t dequeueMutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * queue_constructor -- constructor of the queue container
 */
int OrgQueue::constructor(PMEMobjpool *pop, void *ptr, void *arg)
{
	struct OrgQueue *q = (OrgQueue*)ptr;
	size_t *capacity = (size_t *)arg;

	q->front_ = 0;
	q->back_ = 0;
	q->capacity_ = *capacity;
    printf("orgqueue_constructor front:%zu, back:%zu, capacity:%zu\n", q->front_, q->back_, q->capacity_);

	/* atomic API requires that objects are persisted in the constructor */
	pmemobj_persist(pop, q, sizeof(*q));

	return 0;
}

/*
 * queue_new -- allocates a new queue container using the atomic API
 */
int OrgQueue::newQueue(PMEMobjpool *pop, TOID(struct OrgQueue) *q, size_t nentries)
{
	return POBJ_ALLOC(pop,
		q,
		struct OrgQueue,
		sizeof(struct OrgQueue) + sizeof(TOID(struct OrgQueueEntry)) * nentries,
		OrgQueue::constructor,
		&nentries);
}

/*
 * queue_nentries -- returns the number of entries
 */
size_t OrgQueue::nentries()
{
    if(back_ >= front_) {
        return back_ - front_;
    }
    else {
        return capacity_-front_+back_;
    }
	
}

/*
 * queue_enqueue -- allocates and inserts a new entry into the queue
 */
int OrgQueue::enqueue(PMEMobjpool *pop, const char *data, size_t len)
{
    int ret = 0;
    pthread_mutex_lock(&enqueueMutex);

    size_t curSize = nentries();
	if (capacity_ == curSize) {
        std::cout << "***********nentries:" << curSize << " equal to capacity." << std::endl;
        pthread_mutex_unlock (&enqueueMutex);
        return -1; /* at capacity */
    }
		
	/* back is never decreased, need to calculate the real position */
	//size_t pos = back_ % capacity_;
	size_t pos = back_;

	//printf("inserting %zu %zu: %s\n", back_, pos, data);
	TX_BEGIN(pop) {
		/* let's first reserve the space at the end of the queue */
		TX_ADD_DIRECT(&back_);
		//back_ += 1;
        back_ = (back_+1)%capacity_;
        
		/* now we can safely allocate and initialize the new entry */
		TOID(struct OrgQueueEntry) entry = TX_ALLOC(struct OrgQueueEntry,
			sizeof(struct OrgQueueEntry) + len);
		D_RW(entry)->len_ = len;  
		memcpy(D_RW(entry)->data_, data, len);

		/* and then snapshot the queue entry that we will modify */
		TX_ADD_DIRECT(&entries_[pos]);
		entries_[pos] = entry;
	} TX_ONABORT { /* don't forget about error handling! ;) */
        std::cout << "****************enqueue abort " << ", pos:" << pos 
            << ", back_:" << back_ << std::endl;
		ret = -1;
	} TX_END

    pthread_mutex_unlock (&enqueueMutex);
	return ret;
}

/*
 * queue_dequeue - removes and frees the first element from the queue
 */
int OrgQueue::dequeue(PMEMobjpool *pop)
{
    int ret = 0;
    pthread_mutex_lock(&dequeueMutex);

    if (back_ == front_) {
        pthread_mutex_unlock(&dequeueMutex);
        return -1; /* no entries to remove */
    }
		
    /* front is also never decreased */
	//size_t pos = front_ % capacity_;
    size_t pos = front_;
    
	//printf("removing %zu: %s\n", pos, D_RO(entries_[pos])->data_);
	TX_BEGIN(pop) {
		/* move the queue forward */
		TX_ADD_DIRECT(&front_);
        //front_ += 1;
        front_ = (front_+1)%capacity_;
        
		/* and since this entry is now unreachable, free it */
		TX_FREE(entries_[pos]);
		/* notice that we do not change the PMEMoid itself */
	} TX_ONABORT {
	    std::cout << "*******************dequeue abort" << std::endl;
		ret = -1;
	} TX_END

    pthread_mutex_unlock (&dequeueMutex);
	return ret;
}

/*
 * queue_show -- prints all queue entries
 */
void OrgQueue::show(PMEMobjpool *pop)
{
	size_t numEntry = nentries();
	printf("Entries %zu/%zu\n", numEntry, capacity_);
	for (size_t i = front_; i < back_; ++i) {
		size_t pos = i % capacity_;
		printf("%zu: %s\n", pos, D_RO(entries_[pos])->data_);
	}
}



