#pragma once

#if RANDOM_ALGORITHM == 0
#include "../utils/xorshift.h"
#elif RANDOM_ALGORITHM == 1
#include "../arch/AES_BS_SHORT.h"
#endif
#include "../networking/buffers.h"
#include <stdint.h>

#if RANDOM_ALGORITHM == 2
#if defined(__VAES__) || defined(__SSE2__)
#include "../crypto/aes/AES.h"
#include <x86intrin.h>
#include <immintrin.h>
#include <memory>
    #if defined(__VAES__) && defined(__AVX512F__) && defined (__AVX512VL__)
#define BUF 512
#define MM_XOR _mm512_xor_si512
#define MM_AES_ENC _mm512_aesenc_epi128
#define MM_AES_DEC _mm512_aesdec_epi128
#define MM_AES_ENC_LAST _mm512_aesenclast_epi128
#define MM_AES_DEC_LAST _mm512_aesdeclast_epi128
#define COUNT_TYPE __m512i
/* #define load _mm512_load_epi64 */
    #elif defined(__VAES__) && defined(__AVX2__)
#define BUF 256
#define MM_XOR _mm256_xor_si256
#define MM_AES_ENC _mm256_aesenc_epi128
#define MM_AES_DEC _mm256_aesdec_epi128
#define MM_AES_ENC_LAST _mm256_aesenclast_epi128
#define MM_AES_DEC_LAST _mm256_aesdeclast_epi128
#define COUNT_TYPE __m256i
/* #define load _mm256_load_epi64 */
    #elif defined __SSE2__
#define BUF 128
#define MM_XOR _mm_xor_si128
#define MM_AES_ENC _mm_aesenc_si128
#define MM_AES_DEC _mm_aesdec_si128
#define MM_AES_ENC_LAST _mm_aesenclast_si128
#define MM_AES_DEC_LAST _mm_aesdeclast_si128
#define COUNT_TYPE __m128i
    #endif
#else
#define USE_SSL_AES 1
#include "../crypto/aes/AES_SSL.h"
#define COUNT_TYPE uint64_t
#endif

#endif

class Randomizer
{
    private:
#if defined(__VAES__) || defined(__SSE2__)
COUNT_TYPE key[num_players*multiplier][11]{0};
#else
EVP_CIPHER_CTX* key[num_players*multiplier];
#endif

#if RANDOM_ALGORITHM == 0
DATATYPE srng[num_players*multiplier][64]{0};
#elif RANDOM_ALGORITHM == 1
DATATYPE counter[num_players*multiplier][128]{0};
DATATYPE cipher[num_players*multiplier][128]{0};
DATATYPE key[num_players*multiplier][11][128]{0};
#elif RANDOM_ALGORITHM == 2
#if DATTYPE == BUF
DATATYPE counter[num_players*multiplier]{0};
/* DATATYPE key[num_players*multiplier][11]{0}; */
#else
#define BUFFER_SIZE BUF/DATTYPE 
#if USE_SSL_AES == 1
uint64_t counter[num_players*multiplier][2] = {0};
#else
DATATYPE counter[num_players*multiplier][BUFFER_SIZE] = {0};
#endif
#endif
#endif
int num_generated[num_players*multiplier] = {0};
    public:
    




void init_buffers(int link_id)
{
#if RANDOM_ALGORITHM == 0
    num_generated[link_id] = 64;
#elif RANDOM_ALGORITHM == 1
    num_generated[link_id] = 128;
#elif RANDOM_ALGORITHM == 2
    #if DATTYPE <= 64
    num_generated[link_id] = BUFFER_SIZE;
    #endif
#endif
}

DATATYPE getRandomVal(int link_id)
{
/* if(link_id > 3) */
/*     return SET_ALL_ZERO(); */
#if RANDOM_ALGORITHM == 0
   if(num_generated[link_id] > 63)
   {
       num_generated[link_id] = 0;
       xor_shift(srng[link_id]);
   }
   num_generated[link_id] += 1;
   return srng[link_id][num_generated[link_id] -1];
#elif RANDOM_ALGORITHM == 1
    if(num_generated[link_id] > 127)
    {
        num_generated[link_id] = 0;
        AES__(counter[link_id], key[link_id], cipher[link_id]);
        for (int i = 0; i < 128; i++) {
           counter[link_id][i] += 1;
        }
    }
    num_generated[link_id] += 1;
    return cipher[link_id][num_generated[link_id] -1];
#elif RANDOM_ALGORITHM == 2
    #if USE_SSL_AES == 1
    if(num_generated[link_id] > BUFFER_SIZE - 1)
    {
        num_generated[link_id] = 0;
        counter[link_id][0] += 1;
        DO_ENC_BLOCK( (unsigned char*)counter[link_id], key[link_id]);
    }
        num_generated[link_id] += 1;
    return ((DATATYPE*) (counter[link_id]))[num_generated[link_id] -1];
    
    #else 

    #if DATTYPE == BUF
    DO_ENC_BLOCK(counter[link_id], key[link_id]);
    counter[link_id] += 1;
    return counter[link_id];
    #else
    
    if(num_generated[link_id] > BUFFER_SIZE - 1)
    {
        num_generated[link_id] = 0;
        COUNT_TYPE vectorized_counter{0};
        for (int i = 0; i < BUFFER_SIZE; i++) {
            counter[link_id][i] += 1+BUFFER_SIZE;
        }
        memcpy(&vectorized_counter, counter[link_id], sizeof(COUNT_TYPE));
        DO_ENC_BLOCK(vectorized_counter, key[link_id]);
    }
    num_generated[link_id] += 1;
    return counter[link_id][num_generated[link_id] -1];
    #endif
#endif
#endif
}

DATATYPE getRandomVal(int link_id1, int link_id2)
{
    return getRandomVal( num_players * (link_id1+1) + link_id2);
}

void init_srng(uint64_t link_id, uint64_t link_seed)
{
    #if RANDOM_ALGORITHM == 0
    UINT_TYPE gen_seeds[DATTYPE];
   for (int i = 0; i < DATTYPE; i++) {
      gen_seeds[i] = link_seed * (i+1); // replace with independant seeds in the future
   }
/* int incr = (DATTYPE -1) / 64 + 1; */
/* for (int i = 0; i < 64; i+=incr) { */
/* orthogonalize(gen_seeds+i, srng[link_id]+i); */
orthogonalize_boolean(gen_seeds, srng[link_id]);

#elif RANDOM_ALGORITHM == 1
    UINT_TYPE gen_keys[11][DATTYPE*128/BITLENGTH];
    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < DATTYPE*128/BITLENGTH; j++) {
            gen_keys[i][j] = link_seed * ((i+1)*j); // replace with independant seeds in the future
        }
    }
    for (int i = 0; i < 11; i++) {
            for(int j = 0; j < DATTYPE*128/BITLENGTH; j++)
                orthogonalize_boolean(gen_keys[i]+j, key[link_id][i]+j);
            }
    
#elif RANDOM_ALGORITHM == 2




#if DATTYPE >= 128
    int incr = (DATTYPE -1) / 64 + 1;
#else 
int incr = (sizeof(COUNT_TYPE)*8 - 1) /64 +1;
#endif
    uint64_t gen_keys[11][incr];
    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < incr; j++) {
            gen_keys[i][j] = link_seed * ((i+1)*j); // replace with independant seeds in the future
        }
    }
#if USE_SSL_AES
    for (int i = 0; i < 11; i++) {
    key[link_id] = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(key[link_id], EVP_aes_128_cbc(), NULL, (unsigned char*) gen_keys[i], (unsigned char*) gen_keys[i]);
    }
#else
    for (int i = 0; i < 11; i++) {
    memcpy(&key[link_id][i], gen_keys[i], incr*sizeof(uint64_t));
    }

    /* for (int j = 0; j < DATTYPE*2; j++) { */
        /*     gen_keys[j] = link_seed * (j+1); // replace with independant seeds in the future */
        
    /* } */
        /*     orthogonalize(gen_keys, key[link_id]); */
        /*     orthogonalize(gen_keys+64, key[link_id]+1); */


#endif
#endif

    for (int i = 0; i < 11; i++) {
        init_buffers(link_id);;
    }


}

void init_srngs()
{
#if num_players == 3
/* init_srng(PARTY, PARTY + 5000); */
/* init_srng( (PARTY + 1) % 3 , 6000); */
/* init_srng( (PARTY + 2) % 3 , 6000); */
init_srng(PPREV, modulo((PARTY - 1),  num_players) + 5000);
init_srng(PNEXT,PARTY + 5000);
init_srng(PSELF, PARTY+6000); // used for sharing inputs
#elif num_players == 4
//new logic
/* init_srng(P0, (player_id+1) * 1 + 5000); // */ 
/* init_srng(P1, (player_id+1) * 2 + 5000); // 1*2 */
/* init_srng(P2, (player_id+1) * 3 + 5000); // 1*3 */
/* init_srng(P3, (player_id+1) * 4 + 5000); // 1*4 */
init_srng(0,0);
init_srng(1,0);
init_srng(2,0);
init_srng(3,0);
init_srng(4,0);
init_srng(5,0);
init_srng(6,0);
init_srng(7,0);
#endif

}

};
