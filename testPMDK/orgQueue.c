#include "orgQueue.h"


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
	return back_ - front_;
}

/*
 * queue_enqueue -- allocates and inserts a new entry into the queue
 */
int OrgQueue::enqueue(PMEMobjpool *pop, const char *data, size_t len)
{
	if (capacity_ - nentries() == 0)
		return -1; /* at capacity */

    /* back is never decreased, need to calculate the real position */
	size_t pos = back_ % capacity_;

	int ret = 0;

	printf("inserting %zu: %s\n", pos, data);
 
	TX_BEGIN(pop) {
		/* let's first reserve the space at the end of the queue */
		TX_ADD_DIRECT(&back_);
		back_ += 1;

		/* now we can safely allocate and initialize the new entry */
		TOID(struct OrgQueueEntry) entry = TX_ALLOC(struct OrgQueueEntry,
			sizeof(struct OrgQueueEntry) + len);
        
		D_RW(entry)->len_ = len;
		memcpy(D_RW(entry)->data_, data, len);

		/* and then snapshot the queue entry that we will modify */
		TX_ADD_DIRECT(&entries_[pos]);
		entries_[pos] = entry;
	} TX_ONABORT { /* don't forget about error handling! ;) */
		ret = -1;
	} TX_END

	return ret;
}

/*
 * queue_dequeue - removes and frees the first element from the queue
 */
int OrgQueue::dequeue(PMEMobjpool *pop)
{
	if (nentries() == 0)
		return -1; /* no entries to remove */

	/* front is also never decreased */
	size_t pos = front_ % capacity_;

	int ret = 0;

	printf("removing %zu: %s\n", pos, D_RO(entries_[pos])->data_);

	TX_BEGIN(pop) {
		/* move the queue forward */
		TX_ADD_DIRECT(&front_);
		front_ += 1;
		/* and since this entry is now unreachable, free it */
		TX_FREE(entries_[pos]);
		/* notice that we do not change the PMEMoid itself */
	} TX_ONABORT {
		ret = -1;
	} TX_END

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



