#include <mutex.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <gtest/gtest.h>

using namespace como;

typedef void *(*ThreadEntryFunc)(void *);

class MutexEnvironment : public testing::Environment
{
public:
    static Mutex* m;
    static Mutex* condition_m;
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

    static void* conditionWait(void* myrank) {
        const long rank = (long)myrank;
        printf("==== Thread %ld start and wait ====\n", rank);
        // Critical section start
        {
            m->Lock();
            c->Wait();
            completeThreadNum++;
            m->Unlock();
        }
        // Critical section end
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
        condition_m = new Mutex();
        c = new Condition(*condition_m);
        sum = 0;
        memset(arr, 0, sizeof(arr));
        arr_i = 0;
        completeThreadNum = 0;
    }

    virtual void TearDown() { 
        delete m;
        delete condition_m;
        delete c;
    };
};

Mutex* MutexEnvironment::m;
Mutex* MutexEnvironment::condition_m;
Condition* MutexEnvironment::c;
int MutexEnvironment::completeThreadNum;

int MutexEnvironment::sum;
int MutexEnvironment::arr[40];
int MutexEnvironment::arr_i;

void CreateThread(long rank, ThreadEntryFunc func)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_t newPthread;
    pthread_create(&newPthread, &attr, func, (void*)rank);
    pthread_attr_destroy(&attr);
}

// Use 4 threads.
// Test 1: Mutex
TEST(MutexTest, testMutex1)
{
    long threadNum = 4;
    pthread_t* thread_handles = (pthread_t*)malloc(threadNum * sizeof(pthread_t));
    long thread;

    for (thread = 0; thread < threadNum; thread++)
        pthread_create(&thread_handles[thread], NULL, MutexEnvironment::mutexTestFunction1, (void *)thread);

    for (thread = 0; thread < threadNum; thread++)
        pthread_join(thread_handles[thread], NULL);

    free(thread_handles);
}

// Test 2: AutoMutex
TEST(MutexTest, testMutex2)
{
    long threadNum = 4;
    pthread_t* thread_handles = (pthread_t*)malloc(threadNum * sizeof(pthread_t));
    long thread;

    for (thread = 0; thread < threadNum; thread++)
        pthread_create(&thread_handles[thread], NULL, MutexEnvironment::mutexTestFunction2, (void *)thread);

    for (thread = 0; thread < threadNum; thread++)
        pthread_join(thread_handles[thread], NULL);

    free(thread_handles);
}

// Test 3: Parallel Computing 1: Calculate the sum of thread id.
TEST(MutexTest, testMutex3)
{
    long threadNum = 4;
    pthread_t* thread_handles = (pthread_t*)malloc(threadNum * sizeof(pthread_t));
    long thread;

    for (thread = 0; thread < threadNum; thread++)
        pthread_create(&thread_handles[thread], NULL, MutexEnvironment::mutexTestFunction3, (void *)thread);

    for (thread = 0; thread < threadNum; thread++)
        pthread_join(thread_handles[thread], NULL);

    EXPECT_EQ(MutexEnvironment::sum, 6);

    free(thread_handles);
}

// Test 4: Parallel Computing 2: Calculate the 40th Fibonacci number.
TEST(MutexTest, testMutex4)
{
    long threadNum = 4;
    pthread_t* thread_handles = (pthread_t*)malloc(threadNum * sizeof(pthread_t));
    long thread;

    for (thread = 0; thread < threadNum; thread++)
        pthread_create(&thread_handles[thread], NULL, MutexEnvironment::mutexTestFunction4, (void *)thread);

    for (thread = 0; thread < threadNum; thread++)
        pthread_join(thread_handles[thread], NULL);

    EXPECT_EQ(MutexEnvironment::arr[39], 102334155);

    free(thread_handles);
}

// Test 5: Condition
/*
TEST(MutexTest, testMutex5)
{
    long threadNum = 4;
    pthread_t* thread_handles = (pthread_t*)malloc(threadNum * sizeof(pthread_t));
    long thread;

    // 0.0s, 0.2s, 0.4s, 0.6s: Create 4 threads.
    for (thread = 0; thread < 4; thread++) {
        CreateThread(thread, MutexEnvironment::conditionWait);
        sleep(0.2);
    }

    // 2.0s: Wake a thread.
    sleep(1.2);
    CreateThread(thread++, MutexEnvironment::conditionSignal);
    EXPECT_EQ(MutexEnvironment::completeThreadNum, 1);

    // 3.0s: Wake a thread again.
    sleep(1);
    CreateThread(thread++, MutexEnvironment::conditionSignal);
    EXPECT_EQ(MutexEnvironment::completeThreadNum, 2);
    
    // 4.0s: Wake all threads.
    sleep(1);
    CreateThread(thread++, MutexEnvironment::conditionSignalAll);
    EXPECT_EQ(MutexEnvironment::completeThreadNum, threadNum);

    free(thread_handles);
}
*/

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new MutexEnvironment());
    return RUN_ALL_TESTS();
}