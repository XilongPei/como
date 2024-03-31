#include <mutex.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <gtest/gtest.h>

using namespace como;

class MutexTest : public testing::Test
{
public:
    static Mutex* m;
    static Condition* c;
    static int completeThreadNum;

    static int sum;         // Used in test 3
    static int arr_i;       // Used in test 4
    static int arr[40];     // Used in test 4

    static void* mutexTestFunction1(void* myrank) {
        const long rank = (long)myrank;
        printf("==== Thread %ld start ====\n", rank);
        // Critical section start
        {
            m->Lock();
            printf("==== Thread %ld get lock ====\n", rank);
            sleep(0.5);
            m->Unlock();
        }
        // Critical section end
        printf("==== Thread %ld release lock ====\n", rank);
        return nullptr;
    }

    static void* mutexTestFunction2(void* myrank) {
        const long rank = (long)myrank;
        printf("==== Thread %ld start ====\n", rank);
        // Critical section start
        {
            Mutex::AutoLock a(m);
            printf("==== Thread %ld get lock ====\n", rank);
            sleep(0.5);
        }
        // Critical section end
        printf("==== Thread %ld release lock ====\n", rank);
        return nullptr;
    }

    static void* mutexTestFunction3(void* myrank) {
        const long rank = (long)myrank;
        printf("==== Thread %ld start ====\n", rank);
        // Critical section start
        {
            m->Lock();
            sum += rank;
            m->Unlock();
        }
        // Critical section end
        printf("==== Thread %ld finish ====\n", rank);
        return nullptr;
    }

    static void* mutexTestFunction4(void* myrank) {
        const long rank = (long)myrank;
        printf("==== Thread %ld start ====\n", rank);
        // Critical section start
        {
            m->Lock();
            for(int i = 0; i < 10; i++)
            {
                if(arr_i < 2)
                    arr[arr_i] = 1;
                else
                    arr[arr_i] = arr[arr_i - 1] + arr[arr_i - 2];
                arr_i++;
            }
            m->Unlock();
        }
        // Critical section end
        printf("==== Thread %ld finish ====\n", rank);
        return nullptr;
    }

    static void* mutexTestFunction5(void* myrank) {
        const long rank = (long)myrank;
        printf("==== Thread %ld start ====\n", rank);

        std::function<void(const int&)> a_sum;
        a_sum = [&](const int& i) {
            m->Lock();
            sum += i;
            if(i > 1)
                a_sum(i - 1);
            m->Unlock();
        };

        // Critical section start
        {
            a_sum(rank * 3 + 1);
        }
        // Critical section end
        printf("==== Thread %ld finish ====\n", rank);
        return nullptr;
    }

    static void* mutexTestFunction6(void* myrank) {
        const long rank = (long)myrank;
        printf("==== Thread %ld start ====\n", rank);
        // Critical section start
        {
            m->Lock();
            printf("==== Thread %ld get lock ====\n", rank);
            
            if(rank == 2) {
                printf("==== Thread %ld is teminated  unexpectedly. ===\n", rank);
                return nullptr;
            }

            m->Unlock();
        }
        // Critical section end
        printf("==== Thread %ld release lock ====\n", rank);
        return nullptr;
    }

    static void* conditionWait(void* myrank) {
        const long rank = (long)myrank;
        printf("==== Thread %ld start and wait ====\n", rank);
        // Critical section start
        {
            m->Lock();
            c->Wait();
            completeThreadNum += 1;
            m->Unlock();
        }
        // Critical section end
        printf("==== Thread %ld finish ====\n", rank);

        return nullptr;
    }

    static void* conditionTimedWait(void* myrank) {
        const long rank = (long)myrank;
        bool ret;
        printf("==== Thread %ld start and wait %ld seconds ====\n", rank, (rank + 1) * 2);
        // Critical section start
        {
            m->Lock();
            ret = c->TimedWait((rank + 1) * 2000, 0);
            completeThreadNum += 1;
            m->Unlock();
        }
        // Critical section end
        if(ret)
            printf("==== Thread %ld time out ====\n", rank);
        else
            printf("==== Thread %ld finish ====\n", rank);

        return nullptr;
    }

    static void* conditionSignal(void* myrank) {
        const long rank = (long)myrank;
        printf("==== Thread %ld start and signal ====\n", rank);
        // Critical section start
        {
            m->Lock();
            c->Signal();
            m->Unlock();
        }
        // Critical section end
        printf("==== Thread %ld finish ====\n", rank);

        return nullptr;
    }

    static void* conditionSignalAll(void* myrank) {
        const long rank = (long)myrank;
        printf("==== Thread %ld start and signalall ====\n", rank);
        // Critical section start
        {
            m->Lock();
            c->SignalAll();
            m->Unlock();
        }
        // Critical section end
        printf("==== Thread %ld finish ====\n", rank);

        return nullptr;
    }

protected:
    virtual void SetUp() { 
        m = new Mutex();
        c = new Condition(*m);
        sum = 0;
        memset(arr, 0, sizeof(arr));
        arr_i = 0;
        completeThreadNum = 0;
    }

    virtual void TearDown() { 
        delete m;
        delete c;
    };
};

Mutex* MutexTest::m;
Condition* MutexTest::c;
int MutexTest::completeThreadNum;

int MutexTest::sum;
int MutexTest::arr[40];
int MutexTest::arr_i;

// Use 4 threads.
// Test 1: Mutex
TEST_F(MutexTest, testMutex1)
{
    long threadNum = 4;
    pthread_t* thread_handles = (pthread_t*)malloc(threadNum * sizeof(pthread_t));
    long thread;

    for (thread = 0; thread < threadNum; thread++)
        pthread_create(&thread_handles[thread], NULL, MutexTest::mutexTestFunction1, (void *)thread);

    for (thread = 0; thread < threadNum; thread++)
        pthread_join(thread_handles[thread], NULL);

    free(thread_handles);
}

// Test 2: AutoMutex
TEST_F(MutexTest, testMutex2)
{
    long threadNum = 4;
    pthread_t* thread_handles = (pthread_t*)malloc(threadNum * sizeof(pthread_t));
    long thread;

    for (thread = 0; thread < threadNum; thread++)
        pthread_create(&thread_handles[thread], NULL, MutexTest::mutexTestFunction2, (void *)thread);

    for (thread = 0; thread < threadNum; thread++)
        pthread_join(thread_handles[thread], NULL);

    free(thread_handles);
}

// Test 3: Parallel Computing 1: Calculate the sum of thread id.
TEST_F(MutexTest, testMutex3)
{
    long threadNum = 4;
    pthread_t* thread_handles = (pthread_t*)malloc(threadNum * sizeof(pthread_t));
    long thread;

    for (thread = 0; thread < threadNum; thread++)
        pthread_create(&thread_handles[thread], NULL, MutexTest::mutexTestFunction3, (void *)thread);

    for (thread = 0; thread < threadNum; thread++)
        pthread_join(thread_handles[thread], NULL);

    EXPECT_EQ(MutexTest::sum, 6);

    free(thread_handles);
}

// Test 4: Parallel Computing 2: Calculate the 40th Fibonacci number.
TEST_F(MutexTest, testMutex4)
{
    long threadNum = 4;
    pthread_t* thread_handles = (pthread_t*)malloc(threadNum * sizeof(pthread_t));
    long thread;

    for (thread = 0; thread < threadNum; thread++)
        pthread_create(&thread_handles[thread], NULL, MutexTest::mutexTestFunction4, (void *)thread);

    for (thread = 0; thread < threadNum; thread++)
        pthread_join(thread_handles[thread], NULL);

    EXPECT_EQ(MutexTest::arr[39], 102334155);

    free(thread_handles);
}

// Test 5: Recursive mutex. Add 1+(1+2+3+4)+(1+2+...+7)+(1+2+...+10)
TEST_F(MutexTest, testMutex5)
{
    delete MutexTest::m;
    MutexTest::m = new Mutex(true);

    long threadNum = 4;
    pthread_t* thread_handles = (pthread_t*)malloc(threadNum * sizeof(pthread_t));
    long thread;

    for (thread = 0; thread < threadNum; thread++)
        pthread_create(&thread_handles[thread], NULL, MutexTest::mutexTestFunction5, (void *)thread);

    for (thread = 0; thread < threadNum; thread++)
        pthread_join(thread_handles[thread], NULL);

    EXPECT_EQ(MutexTest::sum, 94);

    free(thread_handles);
}

// Test 6: Unexpected thread termination
/*
TEST_F(MutexTest, testMutex6)
{
    long threadNum = 4;
    pthread_t* thread_handles = (pthread_t*)malloc(threadNum * sizeof(pthread_t));
    long thread;

    for (thread = 0; thread < threadNum; thread++)
        pthread_create(&thread_handles[thread], NULL, MutexTest::mutexTestFunction6, (void *)thread);

    for (thread = 0; thread < threadNum; thread++)
        pthread_join(thread_handles[thread], NULL);

    free(thread_handles);
}
*/

// Test 7: Condition
TEST_F(MutexTest, testMutex7)
{
    long threadNum = 7;
    pthread_t* thread_handles = (pthread_t*)malloc(threadNum * sizeof(pthread_t));
    long thread;

    // 0.0s: Create 4 threads.
    for (thread = 0; thread < 4; thread++) {
        pthread_create(&thread_handles[thread], NULL, MutexTest::conditionWait, (void *)thread);
    }

    // 1.0s: Wake a thread.
    sleep(1);
    pthread_create(&thread_handles[thread], NULL, MutexTest::conditionSignal, (void *)thread);
    thread++;

    // 2.0s: Wake a thread again.
    sleep(1);
    EXPECT_EQ(MutexTest::completeThreadNum, 1);
    pthread_create(&thread_handles[thread], NULL, MutexTest::conditionSignal, (void *)thread);
    thread++;

    // 3.0s: Wake all threads.
    sleep(1);
    EXPECT_EQ(MutexTest::completeThreadNum, 2);
    pthread_create(&thread_handles[thread], NULL, MutexTest::conditionSignalAll, (void *)thread);
    thread++;

    for (thread = 0; thread < threadNum; thread++)
        pthread_join(thread_handles[thread], NULL);

    EXPECT_EQ(MutexTest::completeThreadNum, 4);

    free(thread_handles);
}

// Test 8: Condition::TimedWait
TEST_F(MutexTest, testMutex8)
{
    long threadNum = 6;
    pthread_t* thread_handles = (pthread_t*)malloc(threadNum * sizeof(pthread_t));
    long thread;

    // 0.0s: Create 4 threads. They wait 2.0s/4.0s/6.0s/8.0s
    for (thread = 0; thread < 4; thread++) {
        pthread_create(&thread_handles[thread], NULL, MutexTest::conditionTimedWait, (void *)thread);
        usleep(1);
    }

    // 1.0s: Wake thread 0. 
    sleep(1);
    pthread_create(&thread_handles[thread], NULL, MutexTest::conditionSignal, (void *)thread);
    thread++;

    // 4.0s: Thread 1 wake because of time out.

    // 5.0s: Wake thread 2.
    sleep(4);
    pthread_create(&thread_handles[thread], NULL, MutexTest::conditionSignal, (void *)thread);
    thread++;

    // 8.0s: Thread 3 wake because of time out.

    for (thread = 0; thread < threadNum; thread++)
        pthread_join(thread_handles[thread], NULL);

    EXPECT_EQ(MutexTest::completeThreadNum, 4);

    free(thread_handles);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}