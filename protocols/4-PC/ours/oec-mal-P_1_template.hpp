#pragma once
#include "../../generic_share.hpp"
template <typename Datatype>
class OEC_MAL1_Share
{

private:
    Datatype v;
    Datatype r;
#if PROTOCOL == 11
    Datatype m;
#endif

public:
    
OEC_MAL1_Share() {}
OEC_MAL1_Share(Datatype v, Datatype r) : v(v), r(r) {}
OEC_MAL1_Share(Datatype v) : v(v) {}


OEC_MAL1_Share public_val(Datatype a)
{
    return OEC_MAL1_Share(a,SET_ALL_ZERO());
}

OEC_MAL1_Share Not() const
{
   return OEC_MAL1_Share(NOT(v),r);
}

template <typename func_add>
OEC_MAL1_Share Add(OEC_MAL1_Share b, func_add ADD) const
{
   return OEC_MAL1_Share(ADD(v,b.v),ADD(r,b.r));
}



template <typename func_add, typename func_sub, typename func_mul>
    OEC_MAL1_Share prepare_mult(OEC_MAL1_Share b, func_add ADD, func_sub SUB, func_mul MULT) const
{
/* Datatype cr = XOR(getRandomVal(P_013),getRandomVal(P_123)); */
/* c.r = SUB(getRandomVal(P_013),getRandomVal(P_123)); */
OEC_MAL1_Share c;
c.r = getRandomVal(P_013);
Datatype r124 = getRandomVal(P_013);
/* Datatype r234 = getRandomVal(P_123); //used for veryfying m3' sent by P_3 -> probably not needed -> for verification needed */
c.v = ADD( ADD(MULT(v,b.r), MULT(b.v,r))  , r124);  
/* Datatype m_2 = XOR(c.v, c.r); */
send_to_live(P_2,c.v);

/* Datatype m3_prime = XOR( XOR(r234,cr) , AND( XOR(a.v,a.r) ,XOR(b.v,b.r))); //computationally wise more efficient to verify ab instead of m_3 prime */

/* store_compare_view(P_0,m3_prime); */
/* c.m = ADD(c.v,getRandomVal(P_123)); */
Datatype a1b1 = MULT(v,b.v);
#if PROTOCOL == 10 || PROTOCOL == 12
store_compare_view(P_0,ADD(a1b1,getRandomVal(P_123_2))); // compare a1b1 + r123_2 with P_0
#endif
/* c.v = XOR( AND(      XOR(a.v,a.r) , XOR(b.v,b.r) ) , c.v); */
#if PROTOCOL == 11
c.m = ADD(c.v,getRandomVal(P_123_2)); // m_2 + r234_2 store to compareview later
#endif

c.v = SUB( a1b1,c.v);
return c;
}

template <typename func_add, typename func_sub, typename func_mul>
OEC_MAL1_Share prepare_dot(const OEC_MAL1_Share b, func_add ADD, func_sub SUB, func_mul MULT) const
{
OEC_MAL1_Share c;
c.r = ADD(MULT(v,b.r), MULT(b.v,r)); // a_0 y_1 + b_0 x_1
c.v = MULT(v, b.v); // a0b0
return c;
}
    
    template <typename func_add, typename func_sub, typename func_trunc>
void mask_and_send_dot_with_trunc(func_add ADD, func_sub SUB, func_trunc TRUNC)
{
m = SUB(r, getRandomVal(P_013));// a_0 y_1 + b_0 x_1 - r_0,1,3   
r = getRandomVal(P_013); // z_1
send_to_live(P_2, m); 

}

    template <typename func_add, typename func_sub, typename func_trunc>
void complete_mult_with_trunc(func_add ADD, func_sub SUB, func_trunc TRUNC)
{
m = ADD(m, receive_from_live(P_2)); // v^1,2 = m^1 + m^2
v = TRUNC(SUB(v, m)) // [a_0 b_0 - v^1,2]^t
#if PROTOCOL == 11
store_compare_view(P_0, ADD(m,getRandomVal(P_123)); // compare m1 + m2 + r123 with P_0
#else
store_compare_view(P_012,ADD(m, getRandomVal(P_123))); // v^1,2 + r_1,2,3
#endif
store_compare_view(P_0,ADD(v, getRandomVal(P_123))); // c_0 + w
}

template <typename func_add, typename func_sub>
void mask_and_send_dot(func_add ADD, func_sub SUB)
{
Datatype cr = getRandomVal(P_013);
Datatype r124 = getRandomVal(P_013);
/* Datatype r234 = getRandomVal(P_123); //used for veryfying m3' sent by P_3 -> probably not needed -> for verification needed */
v = ADD( v  , r124);  
/* Datatype m_2 = XOR(c.v, c.r); */
send_to_live(P_2,v);

#if PROTOCOL == 10 || PROTOCOL == 12
store_compare_view(P_0,ADD(r,getRandomVal(P_123_2))); // compare a1b1 + r123_2 with P_0
#endif
#if PROTOCOL == 11
m = ADD(v,getRandomVal(P_123_2)); // m_2 + r234_2 store to compareview later
#endif

v = SUB( r,v);
r = cr;

}
template <typename func_add, typename func_sub>
void complete_mult(func_add ADD, func_sub SUB)
{
Datatype m_3 = receive_from_live(P_2);
v = SUB(v, m_3);

/* c.m = XOR(c.m,m_3); */
/* Datatype cm = XOR(c.m,m_3); */

#if PROTOCOL == 11
store_compare_view(P_0,ADD(m,m_3)); // compare m_2 + m_3 + r234_2
store_compare_view(P_0,ADD(v,getRandomVal(P_123))); //compare ab + c1 + r234_1
#else
store_compare_view(P_012,ADD(v,getRandomVal(P_123))); //compare ab + c1 + r234_1
#endif
}


void prepare_reveal_to_all()
{
return;
}    

template <typename func_add, typename func_sub>
Datatype complete_Reveal(func_add ADD, func_sub SUB)
{
Datatype r = receive_from_live(P_0);
Datatype result = SUB(v, r);
store_compare_view(P_123, r);
store_compare_view(P_0123, result);
return result;
}



template <int id, typename func_add, typename func_sub>
void prepare_receive_from(func_add ADD, func_sub SUB)
{
if constexpr(id == PSELF)
{
    
    Datatype x_0 = getRandomVal(P_013);
    Datatype u = getRandomVal(P_123);
    r = x_0; //  = x_1, x_2 = 0
    v = ADD(get_input_live(),x_0);
    send_to_live(P_0,ADD(v,u));
    send_to_live(P_2,ADD(v,u));

}
else if constexpr(id == P_0)
{
    r = getRandomVal(P_013);
    v = SET_ALL_ZERO();
    // u = 0

}
else if constexpr(id == P_2)
{
    r = SET_ALL_ZERO();
    v = getRandomVal(P_123); //u
    
  

}
else if constexpr(id == P_3)
{
    r = getRandomVal(P_013); //x1
    v = getRandomVal(P_123); //u

    

}
}

template <int id, typename func_add, typename func_sub>
void complete_receive_from(func_add ADD, func_sub SUB)
{
if constexpr(id != PSELF)
{
            #if PRE == 1
        Datatype val;
        if(id == P_3)
            val = pre_receive_from_live(P_3);
        else
            val = receive_from_live(id);
    #else
    Datatype val = receive_from_live(id);
    #endif

    if constexpr(id != P_0)
            store_compare_view(P_0,val);
    if constexpr(id != P_2)
            store_compare_view(P_2,val);
    v = SUB(val,v); // convert locally to a + x_0
}
}

static void send()
{
    send_live();
}

static void receive()
{
    receive_live();
}

static void communicate()
{
    communicate_live();
}

static void prepare_A2B_S1(OEC_MAL1_Share in[], OEC_MAL1_Share out[])
{
    for(int i = 0; i < BITLENGTH; i++)
    {
        out[i].r = SET_ALL_ZERO(); // set share to 0
        Datatype temp[BITLENGTH];
        for (int j = 0; j < BITLENGTH; j++)
        {
            out[j].v = in[j].v; //a0 
        }
    }
    unorthogonalize_arithmetic(out, (UINT_TYPE*) out);
    orthogonalize_boolean((UINT_TYPE*) out, out);
    for (int j = 0; j < BITLENGTH; j++)
    {
        store_compare_view(P_0, FUNC_XOR(out[j].v), getRandomVal(P_123)); 
    }

}


static void prepare_A2B_S2(OEC_MAL1_Share in[], OEC_MAL1_Share out[])
{
    for(int i = 0; i < BITLENGTH; i++)
    {
        out[i].r = getRandomVal(P_013);
        out[i].v = SET_ALL_ZERO();
    }
}

static void complete_A2B_S1(OEC_MAL1_Share out[])
{
}

static void complete_A2B_S2(OEC_MAL1_Share out[])
{

}

void prepare_bit_injection_S1(OEC_MAL1_Share out[])
{
    Datatype temp[BITLENGTH]{0};
    temp[BITLENGTH - 1] = v;
    unorthogonalize_boolean(temp, (UINT_TYPE*) temp);
    orthogonalize_arithmetic((UINT_TYPE*) temp, out);
    for (int j = 0; j < BITLENGTH; j++)
    {
        out[i].r = SET_ALL_ZERO(); // set share to 0
        store_compare_view(P_0, OP_ADD(out[j].v), getRandomVal(P_123)); 
    }
}

void prepare_bit_injection_S2(OEC_MAL1_Share out[])
{
    for(int i = 0; i < BITLENGTH; i++)
    {
        out[i].r = getRandomVal(P_013);
        out[i].v = SET_ALL_ZERO();
    }
}

static void complete_bit_injection_S1(OEC_MAL1_Share out[])
{
    
}

static void complete_bit_injection_S2(OEC_MAL1_Share out[])
{
}

};

