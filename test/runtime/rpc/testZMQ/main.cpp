//=========================================================================
// Copyright (C) 2022 The C++ Component Model(COMO) Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=========================================================================

#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>
#include "zmq.h"
#include "RPCTestUnit.h"
#include <comoapi.h>
#include <comosp.h>
#include <ServiceManager.h>
#include <cstdio>
#include <unistd.h>
#include "ComoConfig.h"
#include "comorpc.h"
#include "CZMQUtils.h"

using como::test::rpc::CService;
using como::test::rpc::IService;
using como::test::rpc::IID_IService;
using jing::ServiceManager;

int mainClient(void);
int mainServer(void);

int main(int argc, char** argv)
{
    if (argc > 1) {
        switch(atoi(argv[1])) {
            case 1:
                mainClient();
                break;
            case 2:
                mainServer();
        }
    }

    return 0;
}

//#define __TEST_SPEED__  1

int mainClient(void)
{
    void *context = zmq_ctx_new();

    //  Socket to talk to server
    void *requester = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester, "tcp://127.0.0.1:5555");

    printf("In mainClient ......\n");

    struct timespec curTime;
    clock_gettime(CLOCK_REALTIME, &curTime);

    for (int i = 0;  i < 10000;  i++) {
        char strDst[256] = {0};
        snprintf(strDst, 256, "Hello %d",i);

        zmq_send(requester, strDst, strlen(strDst)+1, 0);

        char string[256];
        zmq_recv(requester, string, sizeof(string), 0);
#ifndef __TEST_SPEED__
        printf("Received reply %d [%s]\n", i, string);
#endif
    }

    struct timespec endTime;
    clock_gettime(CLOCK_REALTIME, &endTime);

    printf("Req/Rep, 100000 times, elapsed %ld s\n", (endTime.tv_sec - curTime.tv_sec));

    zmq_close(requester);
    zmq_ctx_destroy(context);
    return 0;
}

static void *worker_routine(void *context);

int CZMQUtils_CzmqProxy()
{
    void *context = zmq_ctx_new();

    // Socket to talk to clients
    void *clients = zmq_socket(context, ZMQ_ROUTER);
    zmq_bind(clients, "tcp://127.0.0.1:5555");

    // Socket to talk to workers
    void *workers = zmq_socket(context, ZMQ_DEALER);
    zmq_bind(workers, "inproc://workers");

    //  Launch pool of worker threads
    for (int i = 0;  i < 5;  i++) {
        pthread_t worker;
        pthread_create (&worker, nullptr, worker_routine, context);
    }

    // Connect Worker threads to client threads via a queue proxy
    zmq_proxy(clients, workers, nullptr);

    // We never get here, but clean up anyhow
    zmq_close(clients);
    zmq_close(workers);
    zmq_ctx_destroy(context);

    return 0;
}

#define gettidv1() syscall(__NR_gettid)

static void *worker_routine(void *context)
{
    // Socket to talk to workers
    void *receiver = zmq_socket(context, ZMQ_REP);
    zmq_connect(receiver, "inproc://workers");

    while (1) {
        char string[256];
        zmq_recv(receiver, string, sizeof(string), 0);
#ifndef __TEST_SPEED__
        printf("[%ld]Received request: [%s]\n", (long int)gettidv1(), string);

        // Do some 'work'
        sleep(1);
#endif
        // Send reply back to client
        zmq_send(receiver, "World", 6, 0);
    }
    zmq_close(receiver);
    return nullptr;
}

int mainServer(void)
{
    printf("In mainServer ......\n");
    CZMQUtils_CzmqProxy();

    return 0;
}

