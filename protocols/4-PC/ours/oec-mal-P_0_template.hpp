#pragma once
#include "../../generic_share.hpp"
#define PRE_SHARE OEC_MAL0_Share
template <typename Datatype>
class OEC_MAL0_Share
{
private:
    Datatype v;
    Datatype r;
public:
    
OEC_MAL0_Share() {}
OEC_MAL0_Share(Datatype v, Datatype r) : v(v), r(r) {}
OEC_MAL0_Share(Datatype v) : v(v) {}


    template <typename func_mul, typename func_add, typename func_sub, typename func_trunc>
OEC_MAL0_Share prepare_mult_public_fixed(const Datatype b, func_mul MULT, func_add ADD, func_sub SUB, func_trunc TRUNC) const
{
#if TRUNC_THEN_MULT == 1
    auto result = MULT(TRUNC(r),b);
#else
    auto result = TRUNC(MULT(r,b));
#endif
    auto rand_val = getRandomVal(P_013);
    auto val = SUB(result,rand_val);
#if PROTOCOL == 12
    store_compare_view(P_2, val);
#else
#if PRE == 1
    pre_send_to_live(P_2, val);
#else
    send_to_live(P_2, val);
#endif
#endif
    return OEC_MAL0_Share(SET_ALL_ZERO(),result);
} 
    
template <typename func_add, typename func_sub>
void complete_public_mult_fixed( func_add ADD, func_sub SUB)
{
    v = receive_from_live(P_2);
    store_compare_view(P_1, v);
    /* store_compare_view(P_012, v); */
    v = SUB(v,r);
    /* v = ADD(v,val); */
}


    

static OEC_MAL0_Share public_val(Datatype a)
{
    return OEC_MAL0_Share(a,SET_ALL_ZERO());
}

template <typename func_mul>
OEC_MAL0_Share mult_public(const Datatype b, func_mul MULT) const
{
    return OEC_MAL0_Share(MULT(v,b),MULT(r,b));
}

OEC_MAL0_Share Not() const
{
   return OEC_MAL0_Share(NOT(v),r);
}

template <typename func_add>
OEC_MAL0_Share Add(OEC_MAL0_Share b, func_add ADD) const
{
   return OEC_MAL0_Share(ADD(v,b.v),ADD(r,b.r));
}

template <typename func_add, typename func_sub, typename func_mul>
    OEC_MAL0_Share prepare_mult(OEC_MAL0_Share b, func_add ADD, func_sub SUB, func_mul MULT) const
{
OEC_MAL0_Share c;
c.r = ADD(getRandomVal(P_013),getRandomVal(P_023)); // calculate c_1
/* Datatype r124 = getRandomVal(P_013); */
/* Datatype o1 = XOR( x1y1, r124); */
Datatype o1 = ADD(c.r, ADD(MULT(r, b.r), getRandomVal(P_013)));

#if PROTOCOL == 11
c.v = SUB(ADD( MULT(v,b.r), MULT(b.v,r)),c.r);
#else
c.v = ADD( MULT(v,b.r), MULT(b.v,r));
#endif

/* Datatype m3_flat = AND(a.v,b.v); */

/* c.m = XOR(x1y1, XOR( XOR(AND(a.v,b.v), AND( XOR(a.v, a.r), XOR(b.v, b.r))), c.r)); */
#if PROTOCOL == 12
store_compare_view(P_2,o1);
#else
    #if PRE == 1
        pre_send_to_live(P_2, o1);
    #else
        send_to_live(P_2, o1);
    #endif
#endif

return c;
}

template <typename func_add, typename func_sub, typename func_mul>
OEC_MAL0_Share prepare_dot(const OEC_MAL0_Share b, func_add ADD, func_sub SUB, func_mul MULT) const
{
OEC_MAL0_Share c;
/* #if FRACTIONAL == 0 */
c.r = MULT(r, b.r); //x0y0
c.v = ADD( MULT(v,b.r), MULT(b.v,r)); // au y_0 + bv x_0
/* #else */
/* c.r = MULT(r, b.r); //x0y0 */
/* c.v = ADD(ADD(MULT(v, b.r), MULT(b.v,r)), c.r); //v^1,2 = a_u y_0 + b_v x_0 + x_0 y_0 --> later + m^3 */ 
/* #endif */
return c;
}
    
    template <typename func_add, typename func_sub, typename func_trunc>
void mask_and_send_dot_with_trunc(func_add ADD, func_sub SUB, func_trunc TRUNC)
{
v = ADD(v,r);
r = TRUNC(SUB(ADD(getRandomVal(P_013), getRandomVal(P_023)), r)); // z_0 = [r_0,1,3 + r_0,2,3 - x_0 y_0]^t
#if PROTOCOL == 12
store_compare_view(P_2, SUB(r, getRandomVal(P_013))); // z_0 - z_1
#else
#if PRE == 1
pre_send_to_live(P_2, SUB(r, getRandomVal(P_013))); // z_0 - z_1
#else
send_to_live(P_2, SUB(r, getRandomVal(P_013))); // z_0 - z_1
#endif

#endif
}
template <typename func_add, typename func_sub>
void mask_and_send_dot(func_add ADD, func_sub SUB)
{
Datatype cr = ADD(getRandomVal(P_013),getRandomVal(P_023)); // z_0 = r_0,1,3 + r_0,2,3
/* Datatype r124 = getRandomVal(P_013); */
/* Datatype o1 = XOR( x1y1, r124); */
Datatype o1 = ADD(cr,ADD( r, getRandomVal(P_013))); // x0 y0 + z_0 + r_0,1,3_2

#if PROTOCOL == 11
v = SUB(v,cr); // au y_0 + bv x_0 - z_0
#endif
r = cr; //zo
#if PROTOCOL == 12
store_compare_view(P_2,o1);
#else
    #if PRE == 1
        pre_send_to_live(P_2, o1);
    #else
        send_to_live(P_2, o1);
    #endif
#endif

}

    template <typename func_add, typename func_sub, typename func_trunc>
void complete_mult_with_trunc(func_add ADD, func_sub SUB, func_trunc TRUNC)
{
#if PROTOCOL == 11
Datatype m1m2 = receive_from_live(P_2); // m1 + m2 + r123
store_compare_view(P_1,m1m2); 
store_compare_view(P_3,SUB(m1m2,v)); // m2,2 - cw' 
#else
#if PRE == 1
Datatype m3 = pre_receive_from_live(P_3); // (e + r0,1 + r0,2)^T - r_0,1
#else
Datatype m3 = receive_from_live(P_3); // (e + r0,1 + r0,2)^T - r_0,1
#endif
store_compare_view(P_012,ADD(v,m3)); // v^1,2 = a_u y_0 + b_v x_0 + x_0 y_0 + m^3 
#endif
Datatype c0w = receive_from_live(P_2);
store_compare_view(P_1,c0w); // v^1,2 = a_u y_0 + b_v x_0 + x_0 y_0 + m^3 
v = SUB(c0w,r); // c_0,w - z_0
}


template <typename func_add, typename func_sub>
void complete_mult(func_add ADD, func_sub SUB)
{
#if PROTOCOL == 10 || PROTOCOL == 12
#if PRE == 1
Datatype o_4 = pre_receive_from_live(P_3);
#else
Datatype o_4 = receive_from_live(P_3);
#endif
#elif PROTOCOL == 11
Datatype m_2XORm_3 = receive_from_live(P_2);
store_compare_view(P_1, m_2XORm_3); // Verify if P_2 sent correct message m_2 XOR m_3
store_compare_view(P_3, SUB(m_2XORm_3,v)); // x1 y1 - x1 y3 - x 3 y1 - r234 should remain
v = receive_from_live(P_2); // receive ab + c1 + r_234_1 from P_2 , need to convert to ab+ r234_1 (maybe not? and only for verify?)
store_compare_view(P_1, v); // Verify if P_2 sent correct message of ab
v = SUB(v,r); // convert to ab + r234_1 (maybe not needed)
#endif

#if PROTOCOL == 10 || PROTOCOL == 12
/* Datatype m3_prime = receive_from_live(P_2); */
v = ADD(v, o_4);

/* c.m = XOR(c.m, o_4); */
Datatype m3_prime = receive_from_live(P_2);
v = SUB(m3_prime,v);
store_compare_view(P_012,ADD(v, r)); // compare ab + r_234_1 + c_1 with P_2,P_3
store_compare_view(P_1, m3_prime); // compare m_3 prime with P_2
#endif
}


void prepare_reveal_to_all() const
{
send_to_live(P_1, r);
send_to_live(P_2, r);

send_to_live(P_3, v);
}    

template <typename func_add, typename func_sub>
Datatype complete_Reveal(func_add ADD, func_sub SUB) const
{
#if PRE == 1
Datatype result = SUB(v, pre_receive_from_live(P_3));
/* send_to_live(P_3, result); */
#else
Datatype result = SUB(v, receive_from_live(P_3));
#endif
store_compare_view(P_0123, result); 
// Problem, P_3 sends all the values -> send in circle


return result;
}




template <int id, typename func_add, typename func_sub>
void prepare_receive_from(Datatype val, func_add ADD, func_sub SUB)
{
if constexpr(id == PSELF)
{
    v = val;
    Datatype x_1 = getRandomVal(P_013);
    Datatype x_2 = getRandomVal(P_023);
    r = ADD(x_1, x_2);
    
    send_to_live(P_1,ADD(v, r));
    send_to_live(P_2,ADD( v, r));
     
}
else if constexpr(id == P_1)
{
    r = getRandomVal(P_013); // x_0
}
else if constexpr(id == P_2)
{
    r = getRandomVal(P_023); // x_0
}
else if constexpr(id == P_3)
{
    Datatype x_1 = getRandomVal(P_013);
    Datatype x_2 = getRandomVal(P_023);
    r = ADD(x_1, x_2);
}
}

    template <int id,typename func_add, typename func_sub>
void prepare_receive_from(func_add ADD, func_sub SUB)
{
    if constexpr(id == PSELF)
        prepare_receive_from<id>(get_input_live(), ADD, SUB);
    else
        prepare_receive_from<id>(SET_ALL_ZERO(), ADD, SUB);
}

template <int id, typename func_add, typename func_sub>
void complete_receive_from(func_add ADD, func_sub SUB)
{
if constexpr(id != PSELF)
{

    #if PRE == 1
    if(id == P_3)
        v = pre_receive_from_live(id);
    else
        v = receive_from_live(id);
    #else
    v = receive_from_live(id);
    #endif


    if constexpr(id != P_1)
            store_compare_view(P_1,v);
    if constexpr(id != P_2)
            store_compare_view(P_2,v);


    v = SUB(v,r); // convert locally to a + u
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
/* #if PRE == 0 */
    communicate_live();
/* #endif */
}

static void prepare_A2B_S1(int m, int k, OEC_MAL0_Share in[], OEC_MAL0_Share out[])
{
    for(int i = m; i < k; i++)
    {
        out[i-m].r = SET_ALL_ZERO(); // set share to 0
    }
}


static void prepare_A2B_S2(int m, int k, OEC_MAL0_Share in[], OEC_MAL0_Share out[])
{
    //convert share  (- x0) to boolean
    Datatype temp[BITLENGTH];
        for (int j = 0; j < BITLENGTH; j++)
        {
            temp[j] = OP_SUB(SET_ALL_ZERO(), in[j].r); // set share to -x0
        }
    alignas(sizeof(Datatype)) UINT_TYPE temp2[DATTYPE];
    unorthogonalize_arithmetic(temp, temp2);
    orthogonalize_boolean(temp2, temp);
    /* unorthogonalize_arithmetic(temp, (UINT_TYPE*) temp); */
    /* orthogonalize_boolean((UINT_TYPE*) temp, temp); */

    for(int i = m; i < k; i++)
    {
            out[i-m].r = temp[i]; 
            out[i-m].v = temp[i];  // set both shares to -x0
        #if PROTOCOL == 12
            store_compare_view(P_2, FUNC_XOR(temp[i],getRandomVal(P_013))); //  - x_0 + r013
        #else
            #if PRE == 1
                pre_send_to_live(P_2, FUNC_XOR(temp[i], getRandomVal(P_013))); // -x0 xor r0,1 to P_2
            #else
                send_to_live(P_2, FUNC_XOR(temp[i], getRandomVal(P_013))); // -x0 xor r0,1 to P_2
            #endif
        #endif
    } 
            /* out[0].p1 = FUNC_NOT(out[0].p1);// change sign bit -> -x0 xor r0,1 to x0 xor r0,1 */
}

static void complete_A2B_S1(int k, OEC_MAL0_Share out[])
{
    for(int i = 0; i < k; i++)
    {
        out[i].v = receive_from_live(P_2);  // receive a_0 xor r123
        store_compare_view(P_1, out[i].v);
    }
}

static void complete_A2B_S2(int k, OEC_MAL0_Share out[])
{

}

void prepare_opt_bit_injection(OEC_MAL0_Share x[], OEC_MAL0_Share out[])
{
    Datatype y0[BITLENGTH]{0};
    y0[BITLENGTH - 1] = r; //convert y0 to an arithemtic value
    alignas (sizeof(Datatype)) UINT_TYPE temp2[DATTYPE];
    unorthogonalize_boolean(y0, temp2);
    orthogonalize_arithmetic(temp2, y0);
    Datatype b0v[BITLENGTH]{0};
    b0v[BITLENGTH - 1] = FUNC_XOR(v,r); //convert b0v to an arithemtic value
    unorthogonalize_boolean(b0v, temp2);
    orthogonalize_arithmetic(temp2, b0v);
    for(int i = 0; i < BITLENGTH; i++)
    {
        Datatype r013 = getRandomVal(P_013);
        Datatype r013_2 = getRandomVal(P_013);
        Datatype m00 = OP_SUB(y0[i], r013);
        Datatype m01 = OP_SUB(OP_MULT(x[i].r, y0[i]), r013_2);
#if PROTOCOL != 12
#if PRE == 1
        pre_send_to_live(P_2, m00);
        pre_send_to_live(P_2, m01);
#else
        send_to_live(P_2, m00);
        send_to_live(P_2, m01);
#endif
#else
        store_compare_view(P_2, m00);
        store_compare_view(P_2, m01);
#endif

#if PRE == 1
        Datatype m30 = pre_receive_from_live(P_3);
        Datatype m31 = pre_receive_from_live(P_3);
#else
        Datatype m30 = receive_from_live(P_3);
        Datatype m31 = receive_from_live(P_3);
#endif 
        Datatype a0u = OP_ADD(x[i].v, x[i].r); // set share to a_0 + u
        Datatype c0w = OP_MULT(a0u, b0v[i]);
        Datatype tmp = OP_SUB(OP_ADD(b0v[i], b0v[i]), PROMOTE(1));
        tmp = OP_MULT(tmp, OP_SUB(m31, OP_MULT(a0u, m30)));
        tmp = OP_SUB(tmp, OP_MULT(b0v[i], x[i].r));
        c0w = OP_ADD(c0w, tmp);
        out[i].v = c0w;
        out[i].r = OP_ADD(getRandomVal(P_013), getRandomVal(P_023));
    }
}

void complete_opt_bit_injection()
{
        Datatype m20 = receive_from_live(P_2);
        store_compare_view(P_1, m20);
        v = OP_ADD(v, m20);
        store_compare_view(P_012, OP_ADD(v, r));
        /* v = OP_SUB(v, r); */
}

void prepare_bit2a(OEC_MAL0_Share out[])
{
    Datatype y0[BITLENGTH]{0};
    y0[BITLENGTH - 1] = r; //convert y0 to an arithemtic value
    alignas (sizeof(Datatype)) UINT_TYPE temp2[DATTYPE];
    unorthogonalize_boolean(y0, temp2);
    orthogonalize_arithmetic(temp2, y0);
    Datatype b0v[BITLENGTH]{0};
    b0v[BITLENGTH - 1] = FUNC_XOR(v,r); //convert b0v to an arithemtic value
    unorthogonalize_boolean(b0v, temp2);
    orthogonalize_arithmetic(temp2, b0v);
    for(int i = 0; i < BITLENGTH; i++)
    {
        Datatype r013 = getRandomVal(P_013);
        Datatype m0 = OP_SUB(y0[i], r013);
#if PROTOCOL != 12
#if PRE == 1
        pre_send_to_live(P_2, m0);
#else
        send_to_live(P_2, m0);
#endif
#else
        store_compare_view(P_2, m0);
#endif

#if PRE == 1
        Datatype m3 = pre_receive_from_live(P_3);
#else
        Datatype m3 = receive_from_live(P_3);
#endif 
        out[i].v = OP_SUB( OP_ADD(b0v[i], m3), OP_MULT(OP_ADD(b0v[i], b0v[i]), m3));
        out[i].r = OP_ADD(getRandomVal(P_013), getRandomVal(P_023));
    }

}

void complete_bit2a()
{
    Datatype m20 = receive_from_live(P_2);
    store_compare_view(P_1, m20);
    v = OP_ADD(v, m20);
    store_compare_view(P_012, OP_ADD(v, r));
}


void prepare_bit_injection_S1(OEC_MAL0_Share out[])
{
    for(int i = 0; i < BITLENGTH; i++)
    {
        out[i].r = SET_ALL_ZERO(); // set share to 0
    }
}

void prepare_bit_injection_S2(OEC_MAL0_Share out[])
{
    Datatype temp[BITLENGTH]{0};
    temp[BITLENGTH - 1] = r;
    /* unorthogonalize_boolean(temp,(UINT_TYPE*)temp); */
    /* orthogonalize_arithmetic((UINT_TYPE*) temp,  temp); */
    alignas(sizeof(Datatype)) UINT_TYPE temp2[DATTYPE];
    unorthogonalize_boolean(temp, temp2);
    orthogonalize_arithmetic(temp2, temp);
    for(int i = 0; i < BITLENGTH; i++)
    {
        out[i].v = temp[i]; //c_w = x_0
        out[i].r = OP_SUB(SET_ALL_ZERO(), temp[i]) ; // z_0 = - x_0
        #if PROTOCOL == 12
            store_compare_view(P_2, OP_ADD(temp[i],getRandomVal(P_013))); //  - x_0 + r013
        #else
        #if PRE == 1
            pre_send_to_live(P_2, OP_ADD(temp[i],getRandomVal(P_013))); //  - x_0 + r013
        #else
            send_to_live(P_2, OP_ADD(temp[i],getRandomVal(P_013))); //  - x_0 + r013
        #endif
        #endif
        
    }
}

static void complete_bit_injection_S1(OEC_MAL0_Share out[])
{
    for(int i = 0; i < BITLENGTH; i++)
    {
        out[i].v = receive_from_live(P_2);  // receive a_0 + r123
        store_compare_view(P_1, out[i].v);
    }
    
}

static void complete_bit_injection_S2(OEC_MAL0_Share out[])
{
}

#if MULTI_INPUT == 1

template <typename func_add, typename func_sub, typename func_mul>
    OEC_MAL0_Share prepare_dot3(const OEC_MAL0_Share b, const OEC_MAL0_Share c, func_add ADD, func_sub SUB, func_mul MULT) const
{
Datatype mxy = SUB(MULT(r,b.r),getRandomVal(P_013));
Datatype mxz = SUB(MULT(r,c.r),getRandomVal(P_013));
Datatype myz = SUB(MULT(b.r,c.r),getRandomVal(P_013));
Datatype mxyz = MULT(MULT(r,b.r),c.r);
mxyz = SUB( SET_ALL_ZERO(), mxyz); // trick to be compatible with dot2
#if PRE == 1 && PROTOCOL == 12
Datatype rxy = pre_receive_from_live(P_3);
Datatype rxz = pre_receive_from_live(P_3);
Datatype ryz = pre_receive_from_live(P_3);
#else
Datatype rxy = receive_from_live(P_3);
Datatype rxz = receive_from_live(P_3);
Datatype ryz = receive_from_live(P_3);
/* pre_send_to_live(P_2, mxy); */
/* pre_send_to_live(P_2, mxz); */
/* pre_send_to_live(P_2, myz); */
/* pre_send_to_live(P_2, mxyz); */
#endif
#if PROTOCOL == 12
store_compare_view(P_2, mxy);
store_compare_view(P_2, mxz);
store_compare_view(P_2, myz);
#else
send_to_live(P_2, mxy);
send_to_live(P_2, mxz);
send_to_live(P_2, myz);
#endif
Datatype a0u = ADD(r,v);
Datatype b0v = ADD(b.r,b.v);
Datatype c0w = ADD(c.r,c.v);
OEC_MAL0_Share d;
d.v = ADD(
        ADD( MULT(a0u,ADD(MULT(b0v,SUB(c0w,c.r)),ryz))
            ,(MULT(b0v,SUB(rxz, MULT(c0w,r)))))
        ,MULT(c0w,SUB(rxy, MULT(a0u,b.r)))); // a0(b0(c0 + ryz-z1) + b0(rxz- c0 x1) + c0(rxy- a0 y1)) - rxyz
d.r = mxyz;
d.v = SUB(SET_ALL_ZERO(), d.v); // trick to be compatible with dot2
return d;
}

template <typename func_add, typename func_sub, typename func_mul>
    OEC_MAL0_Share prepare_mult3(const OEC_MAL0_Share b, const OEC_MAL0_Share c, func_add ADD, func_sub SUB, func_mul MULT) const
{
Datatype mxy = SUB(MULT(r,b.r),getRandomVal(P_013));
Datatype mxz = SUB(MULT(r,c.r),getRandomVal(P_013));
Datatype myz = SUB(MULT(b.r,c.r),getRandomVal(P_013));
Datatype mxyz = SUB(MULT(MULT(r,b.r),c.r),getRandomVal(P_013));
#if PRE == 1 && PROTOCOL == 12
Datatype rxy = pre_receive_from_live(P_3);
Datatype rxz = pre_receive_from_live(P_3);
Datatype ryz = pre_receive_from_live(P_3);
Datatype rxyz = pre_receive_from_live(P_3);
#else
Datatype rxy = receive_from_live(P_3);
Datatype rxz = receive_from_live(P_3);
Datatype ryz = receive_from_live(P_3);
Datatype rxyz = receive_from_live(P_3);
/* pre_send_to_live(P_2, mxy); */
/* pre_send_to_live(P_2, mxz); */
/* pre_send_to_live(P_2, myz); */
/* pre_send_to_live(P_2, mxyz); */
#endif
#if PROTOCOL == 12
store_compare_view(P_2, mxy);
store_compare_view(P_2, mxz);
store_compare_view(P_2, myz);
store_compare_view(P_2, mxyz);
#else
send_to_live(P_2, mxy);
send_to_live(P_2, mxz);
send_to_live(P_2, myz);
send_to_live(P_2, mxyz);
#endif
Datatype a0u = ADD(r,v);
Datatype b0v = ADD(b.r,b.v);
Datatype c0w = ADD(c.r,c.v);
OEC_MAL0_Share d;
d.v = SUB(ADD(
        ADD( MULT(a0u,ADD(MULT(b0v,SUB(c0w,c.r)),ryz))
            ,(MULT(b0v,SUB(rxz, MULT(c0w,r)))))
        ,MULT(c0w,SUB(rxy, MULT(a0u,b.r)))), rxyz); // a0(b0(c0 + ryz-z1) + b0(rxz- c0 x1) + c0(rxy- a0 y1)) - rxyz
d.r = ADD(getRandomVal(P_013),getRandomVal(P_023));
return d;
}

template <typename func_add, typename func_sub>
void complete_mult3(func_add ADD, func_sub SUB){
Datatype m20 = receive_from_live(P_2);
store_compare_view(P_1, m20);
v = ADD(v,m20);
store_compare_view(P_012, ADD(v,r));
}

template <typename func_add, typename func_sub, typename func_mul>
    OEC_MAL0_Share prepare_dot4(const OEC_MAL0_Share b, const OEC_MAL0_Share c, const OEC_MAL0_Share d, func_add ADD, func_sub SUB, func_mul MULT) const
{
Datatype mxy = SUB(MULT(r,b.r),getRandomVal(P_013));
Datatype mxz = SUB(MULT(r,c.r),getRandomVal(P_013));
Datatype mxw = SUB(MULT(r,d.r),getRandomVal(P_013));
Datatype myz = SUB(MULT(b.r,c.r),getRandomVal(P_013));
Datatype myw = SUB(MULT(b.r,d.r),getRandomVal(P_013));
Datatype mzw = SUB(MULT(c.r,d.r),getRandomVal(P_013));
Datatype mxyz = SUB(MULT(MULT(r,b.r),c.r),getRandomVal(P_013));
Datatype mxyw = SUB(MULT(MULT(r,b.r),d.r),getRandomVal(P_013));
Datatype mxzw = SUB(MULT(MULT(r,c.r),d.r),getRandomVal(P_013));
Datatype myzw = SUB(MULT(MULT(b.r,c.r),d.r),getRandomVal(P_013));
Datatype mxyzw = MULT(MULT(r,b.r),MULT(c.r,d.r));
#if PRE == 1 && PROTOCOL == 12
/* pre_send_to_live(P_2, mxy); */
/* pre_send_to_live(P_2, mxz); */
/* pre_send_to_live(P_2, mxw); */
/* pre_send_to_live(P_2, myz); */
/* pre_send_to_live(P_2, myw); */
/* pre_send_to_live(P_2, mzw); */
/* pre_send_to_live(P_2, mxyz); */
/* pre_send_to_live(P_2, mxyw); */
/* pre_send_to_live(P_2, mxzw); */
/* pre_send_to_live(P_2, myzw); */
/* pre_send_to_live(P_2, mxyzw); */
Datatype rxy = pre_receive_from_live(P_3);
Datatype rxz = pre_receive_from_live(P_3);
Datatype rxw = pre_receive_from_live(P_3);
Datatype ryz = pre_receive_from_live(P_3);
Datatype ryw = pre_receive_from_live(P_3);
Datatype rzw = pre_receive_from_live(P_3);
Datatype rxyz = pre_receive_from_live(P_3);
Datatype rxyw = pre_receive_from_live(P_3);
Datatype rxzw = pre_receive_from_live(P_3);
Datatype ryzw = pre_receive_from_live(P_3);
#else
Datatype rxy = receive_from_live(P_3);
Datatype rxz = receive_from_live(P_3);
Datatype rxw = receive_from_live(P_3);
Datatype ryz = receive_from_live(P_3);
Datatype ryw = receive_from_live(P_3);
Datatype rzw = receive_from_live(P_3);
Datatype rxyz = receive_from_live(P_3);
Datatype rxyw = receive_from_live(P_3);
Datatype rxzw = receive_from_live(P_3);
Datatype ryzw = receive_from_live(P_3);
#endif
#if PROTOCOL == 12
store_compare_view(P_2, mxy);
store_compare_view(P_2, mxz);
store_compare_view(P_2, mxw);
store_compare_view(P_2, myz);
store_compare_view(P_2, myw);
store_compare_view(P_2, mzw);
store_compare_view(P_2, mxyz);
store_compare_view(P_2, mxyw);
store_compare_view(P_2, mxzw);
store_compare_view(P_2, myzw);
#else
send_to_live(P_2, mxy);
send_to_live(P_2, mxz);
send_to_live(P_2, mxw);
send_to_live(P_2, myz);
send_to_live(P_2, myw);
send_to_live(P_2, mzw);
send_to_live(P_2, mxyz);
send_to_live(P_2, mxyw);
send_to_live(P_2, mxzw);
send_to_live(P_2, myzw);
#endif
Datatype a0 = ADD(r,v);
Datatype b0 = ADD(b.r,b.v);
Datatype c0 = ADD(c.r,c.v);
Datatype d0 = ADD(d.r,d.v);
OEC_MAL0_Share e;
e.v =       
                ADD(
                    ADD(
                        MULT(a0, SUB( MULT(d0, ADD(MULT(b0,SUB(c0,c.r)),ryz )), ryzw))
                        ,
                        MULT(b0, ADD( MULT(a0, SUB(rzw, MULT(c0,d.r))), 
                            SUB( MULT(c0, rxw), rxzw)))
                        )
                    ,
                    ADD(
                        MULT(c0, SUB( MULT(a0, SUB(ryw, MULT(d0,b.r))), rxyw))
                        ,
                        MULT(d0, ADD( MULT(b0, SUB(rxz, MULT(c0,r))),
                            SUB( MULT(c0, rxy), rxyz)))
                    )
        ); // a0(d0(b0(c0 - z1) + ryz) - ryzw) + b0(a0(rzw-c0w1) + c0rxy - rxzw) + c0(a0(ryw-d0y1) - rxyw) + d0(b0(rxz-c0x1) + c0rxy - rxyz) + rxyzw
e.r = mxyzw;
e.v = SUB(SET_ALL_ZERO(), e.v); // trick to be compatible with dot2
return e;
}


template <typename func_add, typename func_sub, typename func_mul>
    OEC_MAL0_Share prepare_mult4(const OEC_MAL0_Share b, const OEC_MAL0_Share c, const OEC_MAL0_Share d, func_add ADD, func_sub SUB, func_mul MULT) const
{
Datatype mxy = SUB(MULT(r,b.r),getRandomVal(P_013));
Datatype mxz = SUB(MULT(r,c.r),getRandomVal(P_013));
Datatype mxw = SUB(MULT(r,d.r),getRandomVal(P_013));
Datatype myz = SUB(MULT(b.r,c.r),getRandomVal(P_013));
Datatype myw = SUB(MULT(b.r,d.r),getRandomVal(P_013));
Datatype mzw = SUB(MULT(c.r,d.r),getRandomVal(P_013));
Datatype mxyz = SUB(MULT(MULT(r,b.r),c.r),getRandomVal(P_013));
Datatype mxyw = SUB(MULT(MULT(r,b.r),d.r),getRandomVal(P_013));
Datatype mxzw = SUB(MULT(MULT(r,c.r),d.r),getRandomVal(P_013));
Datatype myzw = SUB(MULT(MULT(b.r,c.r),d.r),getRandomVal(P_013));
Datatype mxyzw = SUB(MULT(MULT(r,b.r),MULT(c.r,d.r)),getRandomVal(P_013));
#if PRE == 1 && PROTOCOL == 12
/* pre_send_to_live(P_2, mxy); */
/* pre_send_to_live(P_2, mxz); */
/* pre_send_to_live(P_2, mxw); */
/* pre_send_to_live(P_2, myz); */
/* pre_send_to_live(P_2, myw); */
/* pre_send_to_live(P_2, mzw); */
/* pre_send_to_live(P_2, mxyz); */
/* pre_send_to_live(P_2, mxyw); */
/* pre_send_to_live(P_2, mxzw); */
/* pre_send_to_live(P_2, myzw); */
/* pre_send_to_live(P_2, mxyzw); */
Datatype rxy = pre_receive_from_live(P_3);
Datatype rxz = pre_receive_from_live(P_3);
Datatype rxw = pre_receive_from_live(P_3);
Datatype ryz = pre_receive_from_live(P_3);
Datatype ryw = pre_receive_from_live(P_3);
Datatype rzw = pre_receive_from_live(P_3);
Datatype rxyz = pre_receive_from_live(P_3);
Datatype rxyw = pre_receive_from_live(P_3);
Datatype rxzw = pre_receive_from_live(P_3);
Datatype ryzw = pre_receive_from_live(P_3);
Datatype rxyzw = pre_receive_from_live(P_3);
#else
Datatype rxy = receive_from_live(P_3);
Datatype rxz = receive_from_live(P_3);
Datatype rxw = receive_from_live(P_3);
Datatype ryz = receive_from_live(P_3);
Datatype ryw = receive_from_live(P_3);
Datatype rzw = receive_from_live(P_3);
Datatype rxyz = receive_from_live(P_3);
Datatype rxyw = receive_from_live(P_3);
Datatype rxzw = receive_from_live(P_3);
Datatype ryzw = receive_from_live(P_3);
Datatype rxyzw = receive_from_live(P_3);
#endif
#if PROTOCOL == 12
store_compare_view(P_2, mxy);
store_compare_view(P_2, mxz);
store_compare_view(P_2, mxw);
store_compare_view(P_2, myz);
store_compare_view(P_2, myw);
store_compare_view(P_2, mzw);
store_compare_view(P_2, mxyz);
store_compare_view(P_2, mxyw);
store_compare_view(P_2, mxzw);
store_compare_view(P_2, myzw);
store_compare_view(P_2, mxyzw);
#else
send_to_live(P_2, mxy);
send_to_live(P_2, mxz);
send_to_live(P_2, mxw);
send_to_live(P_2, myz);
send_to_live(P_2, myw);
send_to_live(P_2, mzw);
send_to_live(P_2, mxyz);
send_to_live(P_2, mxyw);
send_to_live(P_2, mxzw);
send_to_live(P_2, myzw);
send_to_live(P_2, mxyzw);
#endif
Datatype a0 = ADD(r,v);
Datatype b0 = ADD(b.r,b.v);
Datatype c0 = ADD(c.r,c.v);
Datatype d0 = ADD(d.r,d.v);
OEC_MAL0_Share e;
e.v =       
                ADD(
                    ADD(
                        MULT(a0, SUB( MULT(d0, ADD(MULT(b0,SUB(c0,c.r)),ryz )), ryzw))
                        ,
                        MULT(b0, ADD( MULT(a0, SUB(rzw, MULT(c0,d.r))), 
                            SUB( MULT(c0, rxw), rxzw)))
                        )
                    ,
                    ADD(
                        ADD(rxyzw, MULT(c0, SUB( MULT(a0, SUB(ryw, MULT(d0,b.r))), rxyw)))
                        ,
                        MULT(d0, ADD( MULT(b0, SUB(rxz, MULT(c0,r))),
                            SUB( MULT(c0, rxy), rxyz)))
                    )
        ); // a0(d0(b0(c0 - z1) + ryz) - ryzw) + b0(a0(rzw-c0w1) + c0rxy - rxzw) + c0(a0(ryw-d0y1) - rxyw) + d0(b0(rxz-c0x1) + c0rxy - rxyz) + rxyzw
e.r = ADD(getRandomVal(P_013),getRandomVal(P_023));
return e;
}

template <typename func_add, typename func_sub>
void complete_mult4(func_add ADD, func_sub SUB){
Datatype m20 = receive_from_live(P_2);
store_compare_view(P_1, m20);
v = ADD(v,m20);
store_compare_view(P_012, ADD(v,r));
}

#endif

template <typename func_add, typename func_sub, typename func_xor, typename func_and, typename func_trunc>
void prepare_trunc_2k_inputs(func_add ADD, func_sub SUB, func_xor XOR, func_and AND, func_trunc tr, OEC_MAL0_Share& r_mk2, OEC_MAL0_Share& r_msb, OEC_MAL0_Share& c, OEC_MAL0_Share& c_prime) const{
    Datatype rmk2 = OP_SHIFT_LOG_RIGHT<FRACTIONAL+1>( OP_SHIFT_LEFT<1>(r) );
    Datatype rmsb = OP_SHIFT_LOG_RIGHT<BITLENGTH-1>(r);

    r_mk2.v = rmk2;
    r_mk2.r = SUB(SET_ALL_ZERO(), rmk2);
    r_msb.v = rmsb;
    r_msb.r = SUB(SET_ALL_ZERO(), rmsb);
#if PROTOCOL == 12
    store_compare_view(P_2, SUB(r_mk2.r, getRandomVal(P_013)));
    store_compare_view(P_2, SUB(r_msb.r, getRandomVal(P_013)));
#else
#if PRE == 0
    send_to_live(P_2, SUB(r_mk2.r, getRandomVal(P_013)));
    send_to_live(P_2, SUB(r_msb.r, getRandomVal(P_013)));
#else
    pre_send_to_live(P_2, SUB(r_mk2.r, getRandomVal(P_013)));
    pre_send_to_live(P_2, SUB(r_msb.r, getRandomVal(P_013)));
#endif
#endif

    c.r = SET_ALL_ZERO();
    c_prime.r = SET_ALL_ZERO();
}

template <typename func_add, typename func_sub, typename func_xor, typename func_and, typename func_trunc>
void complete_trunc_2k_inputs(func_add ADD, func_sub SUB, func_xor XOR, func_and AND, func_trunc tr, OEC_MAL0_Share& r_mk2, OEC_MAL0_Share& r_msb, OEC_MAL0_Share& c, OEC_MAL0_Share& c_prime) const{
    c.v = receive_from_live(P_2);
    c_prime.v = receive_from_live(P_2);
    store_compare_view(P_1, c.v);
    store_compare_view(P_1, c_prime.v);
}


};
