#include "orgQueue.h"
#include "Benchmark.hpp"

enum QueueType {
    OrgQueueType
};

class QueueBenchMark : public Benchmark {
public:
    QueueBenchMark(QueueType queueType, std::string filePath)
        : queueType_(queueType), filePath_(filePath) {}

    void setup(size_t threadCnt) {
        pop_ = pmemobj_open(filePath_.c_str(), POBJ_LAYOUT_NAME(orgQueue));
        //if (pop_ == NULL) {
            //printf("create pool\n");
            //pop_ = pmemobj_create(filePath_.c_str(),
			    //POBJ_LAYOUT_NAME(orgQueue), PMEMOBJ_MIN_POOL, 0666);
        //}

        if (pop_ == NULL) {
		    fail("pop_ null");
	    }

	    TOID(struct OrgQueueRoot) root = POBJ_ROOT(pop_, struct OrgQueueRoot);
	    rootp_ = D_RW(root);

        if (D_RW(rootp_->queue) == NULL) {
            std::cout << "queue not exist. create queue." << std::endl;

            if(OrgQueue::newQueue(pop_, &rootp_->queue, 128*6) != 0) {
                fail("failed to create a new queue");
            }
        }

        queue_ = D_RW(rootp_->queue);
        orgQueueSize_ = queue_->nentries();
        std::cout << "orgQueueSize:" << orgQueueSize_ <<  ", front:" <<
            queue_->front_ << ", back:" << queue_-> back_ 
            << ", capacity:" << queue_->capacity_ << std::endl;

        enqNum_ = 0;
        deqNum_ = 0;
    }

    virtual void doTest(size_t thread_index) {
        //setup(thread_index);
        waitForStart();

        uint64_t n_enq = 0;
        uint64_t n_deq = 0;
        uint64_t n_success = 0;
        
        while(!isShutdown()) {
            if (queue_->enqueue(pop_, "123", strlen("123") + 1) == 0) n_enq++;
            if (queue_->dequeue(pop_) == 0) n_deq++;

            n_success += 2;
            //break;
        }

        std::cout << "n_success:" << n_success << ", n_enq:" << n_enq
          << ", n_deq:" << n_deq << std::endl;

        totalOpNum_.fetch_add(n_success, std::memory_order_seq_cst);
        enqNum_.fetch_add(n_enq, std::memory_order_seq_cst);
        deqNum_.fetch_add(n_deq, std::memory_order_seq_cst);
        //teardown();
    }

    virtual void teardown() {
        queue_->show(pop_);

        std::cout << "new size:" << queue_->nentries() << ", enqNum_:" << enqNum_.load()
            << ", deqNum_:" << deqNum_.load() << ", orgSize:" << orgQueueSize_ << std::endl;

        if (orgQueueSize_+enqNum_.load()-deqNum_.load() != queue_->nentries()) {
            std::cout << "**************size not match****************" << std::endl;
        }

        pmemobj_close(pop_);
    }

    int enqueue() {
        return queue_->enqueue(pop_, "123", strlen("123") + 1);
    }

    int dequeue() {
        return queue_->dequeue(pop_);
    }

    void show() {
        queue_->show(pop_);
    }

private:
    QueueType queueType_;
    std::string filePath_;
    //uint64_t seconds_;

    PMEMobjpool * pop_;
    OrgQueueRoot *rootp_;
    OrgQueue * queue_;

    size_t orgQueueSize_;
    std::atomic<uint64_t> enqNum_;
    std::atomic<uint64_t> deqNum_;
};

void doQueueTest(QueueBenchMark & benchMark, uint64_t threadCount, uint64_t runSeconds) {
    benchMark.run(threadCount, runSeconds);
    double opPerSec = (double)(benchMark.getOperationCount()) / benchMark.getRunSeconds();
    std::cout << "getOperationCount:" << benchMark.getOperationCount() 
        << ", getRunSeconds:" << benchMark.getRunSeconds() << ", opPerSec:"
        << std::fixed << std::setprecision(4) << opPerSec << "ops/sec" << std::endl;
}

void simpleTest(QueueBenchMark & benchMark) {
    benchMark.setup(0);
    for(int i = 0; i < 10; i++) {
        benchMark.enqueue();
    }
    benchMark.show();
    for(int i = 0; i < 10; i++) {
        benchMark.dequeue();
    }
    benchMark.show();
    benchMark.teardown();
}

int main(int argc, char *argv[])
{
	if (argc < 5)
		fail("usage: fileName queueType threadCount executeTime");

    std::string filePath = argv[1];
    QueueType qType = static_cast<QueueType>(atoi(argv[2]));
    uint64_t threadCount = atol(argv[3]);
    uint64_t runSeconds = atol(argv[4]);
   
	QueueBenchMark benchMark(qType, filePath);
    doQueueTest(benchMark, threadCount, runSeconds);
    //simpleTest(benchMark);

    return 0;
}



