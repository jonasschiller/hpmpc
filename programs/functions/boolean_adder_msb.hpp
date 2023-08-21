#pragma once
#include "../../protocols/Protocols.h"
#include "../../datatypes/k_bitset.hpp"
#include <cstring>
#include <iostream>

template<typename Share>
class BooleanAdder_MSB {
    using Bitset = sbitset_t<Share>;
private:
    int r;
    Bitset &x;
    Bitset &y;
    Share &z;
    Share carry_last;
    Share carry_this;
   
public:
//constructor

BooleanAdder_MSB()
    {
        r = BITLENGTH;
    }

BooleanAdder_MSB(Bitset &x0, Bitset &x1, Share &y0) : x(x0), y(x1), z(y0) 
    {
        r = BITLENGTH;
    }

void set_values(Bitset &x0, Bitset &x1, Share &y0) 
    {
        x = x0;
        y = x1;
        z = y0;
    }

int get_rounds() {
    return r;
}

int get_total_rounds() {
    return BITLENGTH;
}

bool is_done() {
    return r == 0;
}

void step() 
{
r-=1;
switch(r)
{
    case BITLENGTH-1: //special case for lsbs
        carry_last = x[BITLENGTH-1] & y[BITLENGTH-1];
        break;
case BITLENGTH-2:
        carry_last.complete_and(); // get carry from lsb
        prepare_carry();
      break;
    case 0:
      complete_carry();
      update_z(); // final value, no need to prepare another carry
      break;
    default:
      complete_carry(); // get carry from previous round
      prepare_carry(); // prepare carry for next round
        break;
}
}

void prepare_carry()
{
    carry_this = (carry_last ^ x[r]) & (carry_last ^ y[r]);
}

void complete_carry()
{
    carry_this.complete_and();
    carry_this = carry_this ^ carry_last;
    carry_last = carry_this;

}

void update_z()
{
    z = x[0] ^ y[0] ^ carry_last;

}


};
