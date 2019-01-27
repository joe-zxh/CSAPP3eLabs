/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 *   
 *   (1)   (2)          (3)   (4)     (5)       (6)
 *                XOR   ~(1)  ~(2)  (2)&(3)   (1)&(4)   (5)|(6)  
 *    0     0     0     1     1       0          0         0
 *    0     1     1     1     0       1          0         1
 *    1     0     1     0     1       0          1         1
 *    1     1     0     0     0       0          0         0
 */
int bitXor(int x, int y) {
  int t1, t2, t3;
  t1 = ~x & y;
  t2 = x & ~y;

  t3 = ~t1 & ~t2;

  return (~t3);//8 ops
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 *    
 *  10000000 00000000 00000000 00000000
 */
int tmin(void) {
  int a;

  a=1;
  a = a<<31;

  return a;//2 ops
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 * 
 * when bit=4, Tmax=0111
 * Tmax+1=1000
 * Tmax+1+Tmax=1111
 * Tmax+1+Tmax+1=0000
 * 
 * but -1 also has this property.
 * 
 * if x==-1, then ~x=0000, !(~x)==false
 * !(~Tmax)=true
 * 
 * 
 */
int isTmax(int x) {
  int a,b;
  
  a = x+x+2;

  b=!(~x);

  return !(a | b);//6 ops
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 * 
 *   construct a=   01010101 01010101 01010101 01010101
 *             a|x= x1x1x1x1 x1x1x1x1 x1x1x1x1 x1x1x1x1
 *   if allOddBits, then
 *             a|x= 11111111 11111111 11111111 11111111
 */
int allOddBits(int x) {
  int answer,a;
  
  a = 0x55;//01010101
  a = (a<<8)+a;
  a = (a<<16)+a;//01010101 01010101 01010101 01010101

  answer = (a|x);
  return !(~answer); //7 ops
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 *  
 */
int negate(int x) {
  return (~x)+1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 *  
 * 0x30   0011 0000
 * 0x31   0011 0001
 * 0x32   0011 0010
 * 0x33   0011 0011
 * 0x34   0011 0100
 * 0x35   0011 0101
 * 0x36   0011 0110
 * 0x37   0011 0111
 *all are 0011 0xxx 
 * 
 * 0x38   0011 1000
 * 0x39   0011 1001
 * 
 */
int isAsciiDigit(int x) {

  int a = 0x6;
  int b = x>>3;

  int c = a^(b);//if a==b，then c=0

  int d = x^(0x38);
  int e = x^(0x39);

  //c==0 || d==0 || e==0

  return (!(c | 0)) | (!(d | 0)) | (!(e | 0)) ; //12 ops
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 * 
 *   conditional means if(x！=0),output y；if(x==0),output z
 * 
 *    thought：use x to generate k, where k=11...1 or 00...0
 *    final result is (k & y) | (~k & z)
 *    if k=11...1, then output y
 *    if k=00...0, then output z
 * 
 *                 x    0
 * initial       xxxx 0000    (1)
 * (1)&1111      x1xx 0000    (2)
 * !(2)          0000 0001    (3)
 * (3)+1111      1111 0000    k
 *  
 */
int conditional(int x, int y, int z) {
  int a,k;
  
  a = ~0;//11111111 11111111 11111111 11111111

  k = !(x&a)+a;

  return (k & y) | (~k & z); //8 ops
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 * 
 * output true, if:
 * x==y or
 * x-y<0 && (x-y) not overflow
 * x<0 && y>0
 * 
 * 
 */
int isLessOrEqual(int x, int y) {

  int a,b,c,d,e,f;

  a = x+(~y+1); //x-y

  b = (a^0); //when(x==y), a==0 b==0.

  c = a>>31; //check whether the sign bit is -1
  e = ((y>>31)+1)|(x>>31);
  //x is positive, y is negative may overflow. e is 0 in this situation.
  f = 2+(c&(~(!(!e)))); //if sign bit == -1 and e!=0，then f=0
  
  d = ((x>>31)+1)|(y>>31); //if x<0 && y>0, then d=0
  
  return ((!b) | (!f) | (!d));//23ops
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4
 * 
 *  ops        positive      0       negative
 *  initial    0010         0000      1xxx             (1)
 *  >>31       0000         0000      1111             (2)
 *  (1)+0111   1xxx         0111      xxxx(unkown)     (3)
 *  (3)>>31    1111         0000      1111 or 0000     (4)
 *  (2)|(4)    1111         0000      1111             k
 */
int logicalNeg(int x) {

  int a = x>>31;

  int k = (~(1<<31));

  int b = ((x+k)>>31);

  return (a|b)+1; //7ops
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 * 
 * First convert the positive number to negative by shifting all the bits. 
 * (because they share the same answer)
 * 
 * then find the first "10" pattern.
 *  
 * I cannot figure out this puzzle in 90 opts.
 */
int howManyBits(int x) {
  int tf, sum;

  x=~(x^(x>>31));//get negative.
    
  sum = 0;

  tf = ((!(x&(1<<30)))<<31)>>31;//100000000 00000000  00000000  00000000
  sum += (tf)&32&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<29)))<<31)>>31;//010000000 00000000  00000000  00000000
  sum += (tf)&31&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<28)))<<31)>>31;//001000000 00000000  00000000  00000000
  sum += (tf)&30&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<27)))<<31)>>31;//000100000 00000000  00000000  00000000
  sum += (tf)&29&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<26)))<<31)>>31;//000010000 00000000  00000000  00000000
  sum += (tf)&28&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<25)))<<31)>>31;
  sum += (tf)&27&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<24)))<<31)>>31;
  sum += (tf)&26&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<23)))<<31)>>31;
  sum += (tf)&25&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<22)))<<31)>>31;
  sum += (tf)&24&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<21)))<<31)>>31;
  sum += (tf)&23&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<20)))<<31)>>31;
  sum += (tf)&22&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<19)))<<31)>>31;
  sum += (tf)&21&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<18)))<<31)>>31;
  sum += (tf)&20&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<17)))<<31)>>31;
  sum += (tf)&19&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<16)))<<31)>>31;
  sum += (tf)&18&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<15)))<<31)>>31;
  sum += (tf)&17&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<14)))<<31)>>31;
  sum += (tf)&16&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<13)))<<31)>>31;
  sum += (tf)&15&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<12)))<<31)>>31;
  sum += (tf)&14&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<11)))<<31)>>31;
  sum += (tf)&13&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<10)))<<31)>>31;
  sum += (tf)&12&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<9)))<<31)>>31;
  sum += (tf)&11&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<8)))<<31)>>31;
  sum += (tf)&10&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<7)))<<31)>>31;
  sum += (tf)&9&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<6)))<<31)>>31;
  sum += (tf)&8&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<5)))<<31)>>31;
  sum += (tf)&7&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<4)))<<31)>>31;
  sum += (tf)&6&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<3)))<<31)>>31;
  sum += (tf)&5&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<2)))<<31)>>31;
  sum += (tf)&4&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<1)))<<31)>>31;
  sum += (tf)&3&(((!sum)<<31)>>31);

  tf = ((!(x&(1<<0)))<<31)>>31;
  sum += (tf)&2&(((!sum)<<31)>>31);

  sum += (!sum);//the output of "0" is 1

  return sum; //346 ops.
}

// A solution that is satisfied the 90 ops constrain. But it is copied from others.
// int howManyBits(int x) {
//     int temp=x^(x>>31);//get positive of x;
//     int isZero=!temp;
//     //notZeroMask is 0xffffffff
//     int notZeroMask=(!(!temp)<<31)>>31;
//     int bit_16,bit_8,bit_4,bit_2,bit_1;
//     bit_16=!(!(temp>>16))<<4;
//     //see if the high 16bits have value,if have,then we need at least 16 bits
//     //if the highest 16 bits have value,then rightshift 16 to see the exact place of  
//     //if not means they are all zero,right shift nothing and we should only consider the low 16 bits
//     temp=temp>>bit_16;
//     bit_8=!(!(temp>>8))<<3;
//     temp=temp>>bit_8;
//     bit_4=!(!(temp>>4))<<2;
//     temp=temp>>bit_4;
//     bit_2=!(!(temp>>2))<<1;
//     temp=temp>>bit_2;
//     bit_1=!(!(temp>>1));
//     temp=bit_16+bit_8+bit_4+bit_2+bit_1+2;//at least we need one bit for 1 to tmax,
//     //and we need another bit for sign
//     return isZero|(temp&notZeroMask);
// }


//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  // 1 sign bit, 8 exp bits, 23 frac bits.
  
  int exp,frac;
  
  exp = ((uf & 0x7F800000)>>23);// 0x7F800000=01111111 10000000 00000000 00000000
  frac = ((uf & 0x7FFFFF));//      0x7FFFFF  =00000000 01111111 11111111 11111111

  if(exp==0){//denormalized values
    //uf = uf*2, but need to keep the sign.
    //draw a table to see this rule.
    return ((uf>>31)<<31) | (uf*2);
  }else if(exp==0xFF){//11111111 special values
    return uf;
  }else{//normal cases
    //frac unchange，exp+1
    exp+=1;
    return ((uf>>31)<<31)|(exp<<23)|frac;
  }
  //15 ops
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 *  
 */
int floatFloat2Int(unsigned uf) {
  // 1 sign bit, 8 exp bits, 23 frac bits.
  int E,f,bias,sign,exp,frac,s,i;
  
  bias = 127;

  sign = (uf>>31);//-1 is negative，0 is positive
  if (sign==0){
    sign = 1;
  }else{
    sign=-1;
  }
  exp = ((uf & 0x7F800000)>>23);// 0x7F800000=01111111 10000000 00000000 00000000
  frac = ((uf & 0x7FFFFF));//      0x7FFFFF  =00000000 01111111 11111111 11111111

  if(exp==0){//denormalized values
    return 0;//these values are near 0
  }else if(exp==0xFF){//11111111 special values
    return 0x80000000u;
  }else{//normal cases
    f=1;
    E=exp-bias;

    if(E>=23){ //all the frac are valid
      f = frac|0x800000;//10000000 00000000 00000000
      //left shift the frac by 23
      s = f*sign;
      for (i = 0;i<E-23;i++){ //the residue 2 needs to mulitiply
        //0x40000000 = 01000000 00000000 00000000 00000000
        if(s&0x40000000){ //when left second bit is 1, it would overflow when the next *2.
          return 0x80000000u;
        }
        s*=2;
      }
      return (s);
    }else if(E>=0){ //some frac bits should drop
      f = (frac>>23)|(1<<E);// drop the bits
      s = f*sign;
      for (i = 0;i<E;i++){
        s*=2;
      }
      return (s);
    }else{
      return 0;
    }
  }
  //23 ops
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 *  
 *  this puzzle is simple...
 */
unsigned floatPower2(int x) {
  // 1 sign bit, 8 exp bits, 23 frac bits.
  int bias,exp;

  bias = 127;
  exp = x+bias;
  if (exp<0){ //too small to be represented
    return 0;
  }else if(exp>254){ //too large
    return 2139095040;
  }
  else{
    return (exp<<23);
  }  
  //4 ops
}
