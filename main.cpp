#include <cstdio>
#include "Product_generated.h"

/*
$ g++ main.cpp -o main -std=c++14 -I.
$ ./main
[12, 0, 0, 0, 8, 0, 20, 0, 16, 0, 4, 0, 8, 0, 0, 0, 0, 0, 0, 240, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 240]

Expect U32 to equal U64's lower 32 bits.
Actual result: U32 == U64 & 0xffff'ffffUL is true
*/

int main()
{
    uint32_t low = 0xf000'0000;
    uint32_t high = 0x1;
    uint64_t u64 = (static_cast<uint64_t>(high) << 32) | low;

    flatbuffers::FlatBufferBuilder fbb;
    ProductBuilder fbbProd{ fbb };
    fbbProd.add_U32(low);
    fbbProd.add_U64(u64);
    auto offProd = fbbProd.Finish();
    fbb.Finish(offProd);

    auto cbBuf = fbb.GetSize();
    auto pBuf = fbb.GetBufferPointer();

    if (cbBuf <= 0) {
        return 1;
    }

    printf("[%d", *pBuf);
    for (size_t idx = 1; idx < cbBuf; ++idx) {
        printf(", %d", pBuf[idx]);
    }
    printf("]\n");

    const Product* pProd = flatbuffers::GetRoot<Product>(pBuf);
    if (pProd == nullptr) {
        return 2;
    }

    uint32_t out_low = pProd->U32();
    uint64_t out_u64 = pProd->U64();
    printf("\nExpect U32 to equal U64's lower 32 bits.\n");
    printf("Actual result: U32 == U64 & 0xffff'ffffUL is %s\n",
        out_low == (out_u64 & 0xffff'ffffUL) ? "true" : "false");

    return 0;
}
