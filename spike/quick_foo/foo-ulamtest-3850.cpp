#include <assert.h>

int main()
  {
    const long j = - 0xDDDe; //0xffffffff ffff2222
    int i;
    int n;

    assert(j < 0);
    long k = -j; //0x0, 0xddde
    assert(k > 0);

    n = -j; //found assert in NodeUnaryOp; explicit cast in ulam **********
    i = j; //explicit cast in ulam *************************************
    //s.print(n);
    //s.print(i);
    //s.print(i + n);
    assert((i + n) == 0);

    long mixsum64 = n + j; //j64 + n32;
    assert(mixsum64 == 0);
    int mixsum32 = j + n; //explicit cast reqd in ulam ********
    assert(mixsum32 == 0);

    long mixdiff64 = j - n;
    assert(mixdiff64 == 2 * j);
    int mixdiff32 = j - n; //explicit cast reqd in ulam ***************
    //s.print(mixdiff32); //-113596
    //s.print(2 * i); //overflows to saturate at S32_MAX
    //s.print(n); //56798
    assert(mixdiff32 == 2 * i); //< in ulam XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
    assert(mixdiff32 == 2 * j); //but this is true!
    assert(mixdiff64 == mixdiff32); //qed


    long mixprod64 = 2 * j;
    int mixprod32 = j * 2; //explicit cast req'd in ulam ************
    assert(mixprod64 == mixprod32);

    long mixdiv64 = j / 2;
    int mixdiv32 =  j / 2; //explicit cast req'd in ulam **************
    //s.print(mixdiv32); // -28399
    assert(mixdiv64 == mixdiv32);

    //s.print(mixprod32/mixdiv32);
    assert(mixprod32 / mixdiv64 == 4);
    assert(mixprod64 / mixdiv32 == 4);
    assert(mixprod64 / mixdiv32 == 4);
    assert(mixprod32 / mixdiv64 == 4);


    long mixlshift64 = j << 1;
    int mixlshift32 = j << 1; //explicit cast req'd in ulam *************
    //s.print(mixlshift32); //-113596
    assert(mixlshift32 == mixprod32);
    assert(mixlshift64 == mixlshift32);

    long mixrshift64 = ((unsigned long) j) >> 1; //note: logical shift, use unsigned long!!!
    int mixrshift32 =  (int) (((unsigned long) j) >> 1); //explicit cast req'd in ulam ***********
    //s.print(mixrshift32); //2147483647
    //s.print(mixprod32); // -113596
    //s.print(mixdiv32); // -28399
    //s.print((int) (i >> 1)); //2147455249

    assert(mixrshift64 != mixrshift32); //!= in ulam

    return (i + n);
  }
