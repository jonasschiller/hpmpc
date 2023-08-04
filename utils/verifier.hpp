#pragma once
#include "../arch/DATATYPE.h"
#include <cstdint>
#ifdef __SHA__
#include "../crypto/sha/SHA_256_x86.h"
#elif ARM == 1
#include "../cryptosha/SHA_256_arm.h"
#else
#include "../crypto/sha/SHA_256.h"
#endif

class Verifier {
private:
DATATYPE* verify_buffer[num_players*2]; // Verify buffer for each player
uint64_t verify_buffer_index[num_players*2] = {0};

alignas(sizeof(DATATYPE)) uint32_t hash_val[num_players*2][8]; // Hash value for each player
uint64_t elements_to_compare[num_players*2] = {0};
public:

Verifier()
{
    // Ensure all players have the same initial state
    /* initial state */
    uint32_t state[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    for(int i = 0; i < num_players-1; i++)
    {
        for(int j = 0; j < 8; j++)
            hash_val[i][j] = state[j];
    }


}

void add_element_to_compare(int link_id)
{
    elements_to_compare[link_id]++;
}

uint64_t get_elements_to_compare(int link_id)
{
    return elements_to_compare[link_id];
}

void perform_compare_view(int player_id)
{
    if (verify_buffer_index[player_id] == 0)
        return;
    #ifdef __SHA__
    sha256_process_x86(hash_val[player_id], (uint8_t*) verify_buffer[player_id],sizeof(DATATYPE)*verify_buffer_index[player_id]);
    #elif ARM == 1
    sha256_process_arm(hash_val[player_id], (uint8_t*) verify_buffer[player_id],sizeof(DATATYPE)*verify_buffer_index[player_id]);
    #else
    sha256_process(hash_val[player_id], (uint8_t*) verify_buffer[player_id],sizeof(DATATYPE)*verify_buffer_index[player_id]);
    #endif
    verify_buffer_index[player_id] = 0;
}


void store_compare_view(int player_id, DATATYPE element_to_compare)
{
#if VERIFY_BUFFER > 0
    #if VERIFY_BUFFER == 1
    #ifdef __SHA__
    sha256_process_x86(hash_val[player_id], (uint8_t*) &element_to_compare, sizeof(DATATYPE));
    #else
    sha256_process(hash_val[player_id], (uint8_t*) &element_to_compare, sizeof(DATATYPE));
    #endif
        return;
        #endif
if(verify_buffer_index[player_id] == VERIFY_BUFFER)
    perform_compare_view(player_id);
#endif
verify_buffer[player_id][verify_buffer_index[player_id]] = element_to_compare;
verify_buffer_index[player_id] +=1;
}
    
void get_hashes_to_send(DATATYPE* val_to_send[num_players*2], int hash_chunks_to_send)
{
for (int player_id = 0; player_id < num_players*2; player_id++) {
        if (elements_to_compare[player_id] > 0) {
            perform_compare_view(player_id);
            // exchange 1 sha256 hash. Do to DATATYPE constraints it may need to be
            // split up to multiple chunks
            int index_slice = 0;
            for (int i = 0; i < hash_chunks_to_send; i++) {
#if DATTYPE < 64
              uint8_t *addr_to_send =
                  ((uint8_t *)(hash_val[player_id])) + index_slice;
              index_slice +=
                  sizeof(DATATYPE); // hash is stored in 4 byte chunks -> need
                                    // smaller slices for small DATATYPE
#elif DATTYPE >= 256
              uint32_t values_to_send[8 * sizeof(DATATYPE) / (sizeof(uint32_t) * 8)]{0};
              for (int j = 0; j < 8; j++)
                values_to_send[j] = hash_val[player_id][j];
              uint32_t *addr_to_send = values_to_send;
#else
              uint32_t *addr_to_send = hash_val[player_id] + index_slice;
              index_slice +=
                  sizeof(DATATYPE) / 4; // hash is stored in 4 byte chunks -> move
                                        // up index by multiplier
#endif
              val_to_send[player_id][i] = *((DATATYPE *)addr_to_send);

}
};
