#pragma once
#include "../../generic_share.hpp"
#define SHARE OECL_Share
struct OECL_Share{
    DATATYPE p1;
    DATATYPE p2;
OECL_Share(){}
OECL_Share(DATATYPE s1, DATATYPE s2)
{
    p1 = s1;
    p2 = s2;
}
};