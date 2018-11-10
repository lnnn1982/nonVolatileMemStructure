#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libpmemobj.h>


POBJ_LAYOUT_BEGIN(orgQueue);
POBJ_LAYOUT_ROOT(orgQueue, struct OrgQueueRoot);
POBJ_LAYOUT_TOID(orgQueue, struct OrgQueueEntry);
POBJ_LAYOUT_TOID(orgQueue, struct OrgQueue);
POBJ_LAYOUT_END(orgQueue);

struct OrgQueueEntry { /* queue entry that contains arbitrary data */
	size_t len_; /* length of the data buffer */
	char data_[];
};

struct OrgQueue { /* array-based queue container */
	size_t front_; /* position of the first entry */
	size_t back_; /* position of the last entry */

	size_t capacity_; /* size of the entries array */
	TOID(struct OrgQueueEntry) entries_[];

    int enqueue(PMEMobjpool *pop, const char *data, size_t len);
    int dequeue(PMEMobjpool *pop);
    size_t nentries();
    void show(PMEMobjpool *pop);


    static int constructor(PMEMobjpool *pop, void *ptr, void *arg);
    static int newQueue(PMEMobjpool *pop, TOID(struct OrgQueue) *q, size_t nentries);
};

struct OrgQueueRoot {
	TOID(struct OrgQueue) queue;
};


/*
 * fail -- helper function to exit the application in the event of an error
 */
static void __attribute__((noreturn)) /* this function terminates */
fail(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}













