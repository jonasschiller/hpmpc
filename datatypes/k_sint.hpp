#include <array>
#include <stdexcept>
#include "../arch/DATATYPE.h"

template<typename Pr>
class sint_t {
private:
    std::array<DATATYPE, BITLENGTH> arr;
    size_t k = BITLENGTH;
    Pr P; //protocol
    sint_t(UINT_TYPE value, Pr protocol) {
        P = protocol;
        UINT_TYPE* ptr = (UINT_TYPE*) arr;
        for(std::size_t i = 0; i < k*sizeof(DATATYPE)/sizeof(UINT_TYPE); ++i) {
            ptr[i] = value;
        }
            orthogonalize_arithmetic(ptr, arr.data());
            
    }

    void to_uint(UINT_TYPE* result) {
        unorthogonalize_arithmetic(arr.data(), result);
    }


    DATATYPE& operator[](std::size_t idx) {
        if (idx >= k) {
            throw std::out_of_range("Index out of range");
        }
        return arr[idx];
    }

    const DATATYPE& operator[](std::size_t idx) const {
        if (idx >= k) {
            throw std::out_of_range("Index out of range");
        }
        return arr[idx];
    }

    
    sint_t operator+(const sint_t& other) const {
        sint_t result;
        for(std::size_t i = 0; i < k; ++i) {
            result[i] = P.Add(arr[i] + other[i]);
        }
        return result;
    }

    sint_t operator-(const sint_t & other) const {
        sint_t result;
        for(std::size_t i = 0; i < k; ++i) {
            result[i] = P.Sub(arr[i] - other[i]);
        }
        return result;
    }

        sint_t operator*(const sint_t & other) const {
        sint_t result;
        for(std::size_t i = 0; i < k; ++i) {
            P.prepareMult(arr[i], other[i],result[i]);
        }
        return result;
    }

        void completeMult() {
        for(std::size_t i = 0; i < k; ++i) {
            P.completeMult(arr[i]);
        }
        }
        


    // ... You can define other operations similarly

};

