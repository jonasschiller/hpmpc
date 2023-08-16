#pragma once
#include "functions/share_conversion.hpp"
void generateElements()
{
        UINT_TYPE input[DATTYPE];
        player_input = new DATATYPE[BITLENGTH];
        for(int i = 0; i < DATTYPE; ++i) {
            if (i % 2 == 0)
                input[i] = 15;
            else
                input[i] = -15;
        }
        std::cout << PARTY << " input: ";
    for(int i = 0; i < DATTYPE; i++)
    {    std::cout << std::bitset<sizeof(uint64_t)*8>(input[i]);
    std::cout << std::endl;
    }
        
    orthogonalize_boolean(input, player_input);
    /* orthogonalize_arithmetic(input, player_input); */
}

void print_result(DATATYPE* var) 
{
}

