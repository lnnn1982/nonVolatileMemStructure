#pragma once

#include <atomic>
#include <deque>
#include <thread>
#include <chrono>
#include "Common.h"


class Benchmark {
public:
    Benchmark()
        : threadsReady_(0)
        , threadsFinished_(0)
        , startRunning_(false)
        , isShutdown_(false)
        , runSeconds_(0)
        , totalOpNum_(0)
    {
    }


    virtual void setup(size_t thread_count) = 0;
    virtual void teardown() = 0;
    virtual void doTest(size_t thread_index) = 0;
    
    void run(size_t threadCnt, uint64_t runSeconds){
        threadsFinished_ = 0;
        threadsReady_ = 0;
        startRunning_ = false;

        setup(threadCnt);

        // Start threads
        std::deque<std::thread> threads;
        for(size_t i = 0; i < threadCnt; ++i) {
            threads.emplace_back(&Benchmark::entry, this, i, threadCnt);
        }

        // Wait for threads to be ready
        while(threadsReady_.load() < threadCnt);
        std::cout << "Starting benchmark." << std::endl;

        // Start the benchmark
        auto start = std::chrono::high_resolution_clock::now();
        startRunning_.store(true, std::memory_order_release);

        // Sleep the required amount of time before setting the shutdown flag.
        std::this_thread::sleep_for(std::chrono::milliseconds(runSeconds*1000));
        
        isShutdown_.store(true, std::memory_order_release);

        for(auto& thread : threads) {
            thread.join();
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end-start;
        runSeconds_ = elapsed.count()/1000;
        
        std::cout << "Benchmark stopped." << std::endl;

        teardown();
    }


    void waitForStart() {
        threadsReady_.fetch_add(1, std::memory_order_acq_rel);
        while(!startRunning_.load(std::memory_order_acquire));
    }

    bool isShutdown() {
        return isShutdown_.load(std::memory_order_acquire);
    }

    void entry(size_t threadIndex, size_t threadCnt) {
        doTest(threadIndex);
    }

    double getRunSeconds() {
        return runSeconds_;
    }

    virtual uint64_t getOperationCount() {
        return totalOpNum_.load();
    }

protected:
    
  std::atomic<size_t> threadsReady_;
  std::atomic<size_t> threadsFinished_;
  std::atomic<bool> startRunning_;
  std::atomic<bool> isShutdown_;

  double runSeconds_;
  std::atomic<uint64_t> totalOpNum_;

  


};
























