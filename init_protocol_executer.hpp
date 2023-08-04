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

#include "networking/sockethelper.h"
#include "networking/buffers.h"
#if LIVE == 1 && INIT == 0 && NO_INI == 0
#include "protocols/CircuitInfo.hpp"
#endif

template <typename Init_Share, typename Init_Protocol, typename Init_Function, typename Input_Type, typename Result_Type>
class INIT_PROTOCOL_EXECUTER
{
private:
Init_Function init_func;
sender_args sending_args[num_players];
receiver_args receiving_args[num_players];


#if PRE == 1
sender_args sending_args_pre[num_players];
receiver_args receiving_args_pre[num_players];
#endif
#if MAL == 1
Verifier verifier;
#endif
Mutexes mutexes;
Init_Protocol p_init;

struct timespec i1, p1, p2, l1, l2;

int modulo(int x,int N){
    return (x % N + N) %N;
}

public:
INIT_PROTOCOL_EXECUTER(Init_Function init_func) : init_func(init_func) {
}


void init_circuit(std::string ips[])
{

/* clock_t time_init_start = clock (); */
/* std::chrono::high_resolution_clock::time_point t_init = std::chrono::high_resolution_clock::now(); */
#if PRINT == 1
print("Initializing circuit ...\n");
#endif

    p_init = Init_Protocol(OPT_SHARE);
    
    #if INIT == 1 && NO_INI == 0
    auto result_garbage = new Result_Type;
    auto input_garbage = new Input_Type;

    
    init_func(p_init,input_garbage,result_garbage);

    #if PRE == 1
    p_init.finalize(ips,receiving_args_pre,sending_args_pre);
    #else
    p_init.finalize(ips); //TODO change to new version
    #endif
#endif



}


void executeProgram(int argc, char *argv[], int process_id, int process_num)
{
clock_gettime(CLOCK_REALTIME, &i1);


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


/* generateElements(); */

init_circuit(ips);



}

};

void execute(int argc, char *argv[], int process_id, int process_num)
{
    INIT_PROTOCOL_EXECUTER<INIT_SHARE, PROTOCOL_INIT, FUNCTION<PROTOCOL_INIT,INIT_SHARE>, INPUTTYPE, RESULTTYPE> executer(FUNCTION<PROTOCOL_INIT,INIT_SHARE>{});
    executer.executeProgram(argc, argv, process_id, process_num);
}

