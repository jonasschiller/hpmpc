#pragma once
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <cstring>
#include <pthread.h>
#include <random>
#include <bitset>
#include <new>
#include <memory>
#include "arch/DATATYPE.h"
#include "arch/SSE.h"
#include "programs/functions/debug.hpp"
#include "protocols/generic_share.hpp"
#include "protocols/init_protocol_base.hpp"
#include "protocols/live_protocol_base.hpp"

#if FUNCTION_IDENTIFIER == 0
#include "programs/search_init.hpp"
#elif FUNCTION_IDENTIFIER == 1
#include "programs/xor_not_and.hpp"
#elif FUNCTION_IDENTIFIER == 2 || FUNCTION_IDENTIFIER == 3 || FUNCTION_IDENTIFIER == 5 || FUNCTION_IDENTIFIER == 6 || FUNCTION_IDENTIFIER == 8 || FUNCTION_IDENTIFIER == 9 || FUNCTION_IDENTIFIER == 10
#include "programs/mult_init.hpp"
#elif FUNCTION_IDENTIFIER == 4 || FUNCTION_IDENTIFIER == 7
#include "programs/debug_init.hpp"
#endif

#include "utils/xorshift.h"

#include "config.h"
#include "protocols/Protocols.h"

#include "utils/randomizer.hpp"
#include "utils/verifier.hpp"
#include "utils/mutexes.hpp"
#include "utils/timing.hpp"
#include "utils/print.hpp"
#include "networking/client.hpp"
#include "networking/server.hpp"

#include "networking/sockethelper.h"
#include "networking/buffers.h"
#if LIVE == 1 && INIT == 0 && NO_INI == 0
#include "protocols/CircuitInfo.hpp"
#endif
template <typename Live_Share, typename Live_Protocol, typename Result_Type,  typename Datatype_bool, typename Datatype_arithmetic>
class ProtocolExecuter
{
private:
Live_Function live_func;
sender_args sending_args[num_players];
receiver_args receiving_args[num_players];

Randomizer randomizer;

#if PRE == 1
sender_args sending_args_pre[num_players];
receiver_args receiving_args_pre[num_players];
#endif
#if MAL == 1
Verifier verifier;
#endif
Mutexes mutexes;
Live_Protocol p_live;

ProtocolExecuter(Function func)
{
this->func = func;
}
struct timespec i1, p1, p2, l1, l2;

int modulo(int x,int N){
    return (x % N + N) %N;
}









#if LIVE == 1
void live_circuit()
{
pthread_t sending_Threads[num_players-1];
pthread_t receiving_threads[num_players-1];
int ret;

//TODO check, recently commented
    for(int t=0;t<(num_players-1);t++) {
        ret = pthread_create(&receiving_threads[t], NULL, receiver, &receiving_args[t]);
        if (ret){
            print("ERROR; return code from pthread_create() is %d\n", ret);
            exit(-1);
            }
    }

    /// Creating sending threads



    for(int t=0;t<(num_players-1);t++) {
        ret = pthread_create(&sending_Threads[t], NULL, sender, &sending_args[t]);
        if (ret){
            print("ERROR; return code from pthread_create() is %d\n", ret);
            exit(-1);
            }
    }



    // waiting until all threads connected
    mutexes.wait_for_connection();
    print("All parties connected sucessfully, starting protocol and timer! \n");
    clock_gettime(CLOCK_REALTIME, &l1);




    /// Processing Inputs ///
    /* Sharemind protocol = Sharemind(); */
    clock_t time_function_start = clock ();
    std::chrono::high_resolution_clock::time_point c1 =
            std::chrono::high_resolution_clock::now();
    
    p_live = Live_Protocol(OPT_SHARE);
    auto result = new RESULTTYPE;
    func<Live_Protocol,Live_Share>(p_live,result);
    #if MAL==1
        p_live.compare_views();
        /* p_live.communicate(); */
    #endif
    
    
    
    for(int t=0;t<(num_players-1);t++) {
        pthread_join(receiving_threads[t],NULL);
        pthread_join(sending_Threads[t],NULL);
        /* sending_args[t].elements_to_send.clear(); */

    }

    double time = std::chrono::duration_cast<std::chrono::microseconds>(
                         std::chrono::high_resolution_clock::now() - c1)
                         .count();
    /* searchComm__<Sharemind,DATATYPE>(protocol,*found); */
    clock_gettime(CLOCK_REALTIME, &l2);
    double accum = ( l2.tv_sec - l1.tv_sec )
    + (double)( l2.tv_nsec - l1.tv_nsec ) / (double) 1000000000L;
    #if PRINT == 1
    print_result(result); //different for other functions
    #endif
    clock_t time_function_finished = clock ();

    double init_time = ( l1.tv_sec - i1.tv_sec )
    + (double)( l1.tv_nsec - i1.tv_nsec ) / (double) 1000000000L;
    #if PRE == 1
    double accum_pre = ( p2.tv_sec - p1.tv_sec )
    + (double)( p2.tv_nsec - p1.tv_nsec ) / (double) 1000000000L;
    init_time = init_time - accum_pre;
    #endif
    print("Time measured to initialize program: %fs \n", init_time);
    /* printf("Time measured to read and receive inputs: %fs \n", double((time_data_received - time_application_start)) / CLOCKS_PER_SEC); */
    print("Time measured to perform computation clock: %fs \n", double((time_function_finished - time_function_start)) / CLOCKS_PER_SEC);
    print("Time measured to perform computation getTime: %fs \n", accum);
    print("Time measured to perform computation chrono: %fs \n", time / 1000000);
    // Join threads to ensure closing of sockets

}
#endif

void executeLive(int argc, char *argv[], int process_id, int process_num)
{

Randomizer randomizer = Randomizer();
#if MAL == 1
Verifier verifier = Verifier();
#endif


/// Connecting to other Players
std::string ips[num_players-1];

//char* hostnames[num_players-1];
for(int i=0; i < num_players -1; i++)
{
    if(i < argc - 1 )
        ips[i] = std::string(argv[i+1]);
    else
    {
        ips[i] = "127.0.0.1";
    }
}


mutexes = Mutexes();

generateElements();

init_circuit(ips);

#if PRE == 1
    preprocess_circuit(ips);
#endif

#if PRE == 1 && LIVE == 0
double dummy_time = 0.00;
    print("Time measured to initialize program: %fs \n", dummy_time);
    print("Time measured to perform computation clock: %fs \n", dummy_time);
    print("Time measured to perform computation getTime: %fs \n", dummy_time);
    print("Time measured to perform computation chrono: %fs \n", dummy_time);
#endif

#if LIVE == 1
    live_circuit();
#endif

}

};

void execute(int argc, char *argv[], int process_id, int process_num)
{
#ifndef PROTOCOL_PRE
    #define PROTOCOL_PRE int
#endif
#ifndef PRE_SHARE
    #define PRE_SHARE int
#endif
    ProtocolExecuter<INIT_SHARE, SHARE, PRE_SHARE, PROTOCOL_INIT, PROTOCOL_LIVE, PROTOCOL_PRE, search<PROTOCOL_INIT,INIT_SHARE>, search<PROTOCOL_LIVE,SHARE>, search<PROTOCOL_PRE,PRE_SHARE>, RESULTTYPE, DATATYPE, DATATYPE>
}

