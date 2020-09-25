import { flatbuffers } from 'flatbuffers';
import { Product } from './Product_generated';

const low = 0xf000_0000;
const high = 0x1;

const fbb = new flatbuffers.Builder();
Product.startProduct(fbb);
Product.addU32(fbb, low);
Product.addU64(fbb, new flatbuffers.Long(low, high));
const offProd = Product.endProduct(fbb);
fbb.finish(offProd);
const u8arr = fbb.asUint8Array();


const bufProd = new flatbuffers.ByteBuffer(u8arr);
const prod = Product.getRootAsProduct(bufProd);
const u32 = prod.U32();
const u64 = prod.U64();

console.log('1. Serialize and deserialize from TS');
console.log(`Buffer from TS:  ${u8arr}`);
console.log('Expect U32 to equal U64.low.');
console.log(`Actual result: U32 == U64.low is ${u32 == u64.low}`);


const c_u8arr = new Uint8Array(
    [12, 0, 0, 0, 8, 0, 20, 0, 16, 0, 4, 0, 8, 0, 0, 0, 0, 0, 0, 240, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 240]
    );
const c_bufProd = new flatbuffers.ByteBuffer(c_u8arr);
const c_prod = Product.getRootAsProduct(c_bufProd);
const c_u32 = c_prod.U32();
const c_u64 = c_prod.U64();

console.log('\n2. Deserialize buffer from CPP output');
console.log(`Buffer from CPP: ${c_u8arr}`);
console.log('Expect U32 to equal U64.low.');
console.log(`Actual result: U32 == U64.low is ${c_u32 == c_u64.low}`);
