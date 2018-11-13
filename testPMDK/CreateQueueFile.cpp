#include<stdio.h>
#include<unistd.h>
#include "orgQueue.h"




int main(int argc, char *argv[])
{
	if (argc < 2)
		fail("usage: fileName");

    std::string filePath = argv[1];
    if( (access(filePath.c_str(), F_OK )) != -1) {
        std::cout << "delete file" << std::endl;
        remove(filePath.c_str());
    }

    PMEMobjpool * pop = pmemobj_create(filePath.c_str(),
			    POBJ_LAYOUT_NAME(orgQueue), PMEMOBJ_MIN_POOL*50, 0666);
    if (pop == NULL) {
        fail("pmemobj_create");
	}

    TOID(struct OrgQueueRoot) root = POBJ_ROOT(pop, struct OrgQueueRoot);
    //OrgQueueRoot * rootp = D_RW(root);

    /*if (D_RW(rootp->queue) == NULL) {
        std::cout << "queue not exist. create queue." << std::endl;

        if(OrgQueue::newQueue(pop, &rootp->queue, 4096) != 0) {
            fail("failed to create a new queue");
        }
    }*/

    pmemobj_close(pop);
    return 0;
}


















