# README
Demonstrate flatbuffers v1.12 TS returns negative values for uint64.

## Tool Versions

| Tool                  | Version   |
|-----------------------|-----------|
| flatc                 | 1.12.0    |
| flatbuffers           | 1.12.0    |
| @types/flatbuffers    | 1.10.0    |
| node                  | 14.4.0    |
| ts-node               | 9.0.0     |
| typescript            | 4.0.3     |

## Issue
`flatbuffers.Long` mistakenly converts uint32 values to signed value with `low | 0` or `high | 0`.

## Steps to Reproduce

### Demo
This repo provides a demo script, just run `index.ts`.

``` bash
1. Serialize and deserialize from TS
Buffer from TS:  12,0,0,0,8,0,20,0,16,0,4,0,8,0,0,0,0,0,0,240,1,0,0,0,0,0,0,0,0,0,0,240
Expect U32 to equal U64.low.
Actual result: U32 == U64.low is false

2. Deserialize buffer from CPP output
Buffer from CPP: 12,0,0,0,8,0,20,0,16,0,4,0,8,0,0,0,0,0,0,240,1,0,0,0,0,0,0,0,0,0,0,240
Expect U32 to equal U64.low.
Actual result: U32 == U64.low is false
```

### Step-by-Step
 1. Create a table that contains `uint32` and `uint64` fields.
    For example,
    ```
    table Product {
        U32: uint32;
        U64: uint64;
    }
    ```

 2. Generate TypeScript and C++ code with `flatc` utility.
    ```
    flatc -o . --ts --no-fb-import --no-ts-reexport fbs/Product.fbs
    flatc -o . --cpp --no-fb-import fbs/Product.fbs
    ```

 3. Serialize `0x0000'0001'f000'0000` in CPP.
    ``` cpp
    uint32_t low = 0xf000'0000;
    uint32_t high = 0x1;
    uint64_t u64 = (static_cast<uint64_t>(high) << 32) | low;

    flatbuffers::FlatBufferBuilder fbb;
    ProductBuilder fbbProd{ fbb };
    fbbProd.add_U32(low);
    fbbProd.add_U64(u64);
    auto offProd = fbbProd.Finish();
    fbb.Finish(offProd);
    ```

 4. Deserialize the buffer in TypeScript.
    ``` ts
    const c_u8arr = new Uint8Array(/*uint8_t array from CPP*/);
    const c_bufProd = new flatbuffers.ByteBuffer(c_u8arr);
    const c_prod = Product.getRootAsProduct(c_bufProd);
    const c_u32 = c_prod.U32();
    const c_u64 = c_prod.U64();
    ```

 5. Expect `c_u32 == c_u64.low`.


## Actual Result
`c_u64.low` becomes negative, and thus differs from `c_u32`.



# Code Analysis

`flatbuffers.Long` seems to cause this issue.

The `low` and `high` parameters are `|`-ed with 0 (_seemed to provide default values_) and cause some unsigned values becoming negative.

> Ref of [`Long` constructor](https://github.com/google/flatbuffers/blob/v1.12.0/js/flatbuffers.js#L96) implementation of v1.12.0:
> ``` ts
> flatbuffers.Long = function(low, high) {
>   /**
>    * @type {number}
>    * @const
>    */
>   this.low = low | 0;
>
>   /**
>    * @type {number}
>    * @const
>    */
>   this.high = high | 0;
> };
> ```
>
> Ref of a more recent [`Long` constructor](https://github.com/google/flatbuffers/pull/6095/files#diff-02ed618d88d609c5ab89c3839d6c36fdR9) implementation (google/flatbuffers#6095):
> ``` ts
> constructor(low: number, high: number) {
>   this.low = low | 0;
>   this.high = high | 0;
> }
> ```


Therefore, even though `readUint64` created `Long` with two `readUint32`s, the `low` and `high` are still signed value.

> Ref of [`readUint64`](https://github.com/google/flatbuffers/blob/v1.12.0/js/flatbuffers.js#L965) implementation of v1.12.0:
> ``` ts
> flatbuffers.ByteBuffer.prototype.readUint64 = function(offset) {
>   return new flatbuffers.Long(this.readUint32(offset), this.readUint32(offset + 4));
> };
> ```
>
> Ref of a more recent [`readUint64`](https://github.com/google/flatbuffers/pull/6095/files#diff-ceab2b22408a49c392f73320db3ed073R82) implementation (google/flatbuffers#6095):
> ``` ts
> readUint64(offset: number): Long {
>   return new Long(this.readUint32(offset), this.readUint32(offset + 4));
> }
> ```