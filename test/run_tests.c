
/*==============================================================================
 run_tests.c -- test aggregator and results reporting
 
 Copyright 2018 Laurence Lundblade
 
 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:
 
 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 
 (This is the MIT license)
 ==============================================================================*/
//  Created by Laurence Lundblade on 9/30/18.


#include "run_tests.h"
#include "UsefulBuf.h"
#include <stdbool.h>

#include "half_precision_test.h"
#include "basic_test.h"
#include "bstrwrap_tests.h"
#include "mempool_test.h"
#include "qcbor_decode_tests.h"

// Used to test the test runner
int fail_test()
{
    return -44;
}


/*
 Convert a number up to 999999999 to a string. This is so sprintf doesn't
 have to be linked in so as to minimized dependencies even in test code.
 
 This function does pointer math. TODO: test this.
 */
const char *NumToString(int32_t nNum, UsefulBuf StringMem)
{
    const uint32_t uMax = 1000000000;
    
    UsefulOutBuf OutBuf;
    UsefulOutBuf_Init(&OutBuf, StringMem);
    
    if(nNum < 0) {
        UsefulOutBuf_AppendByte(&OutBuf, '-');
        nNum = -nNum;
    }
    if(nNum > uMax-1) {
        return "XXX";
    }
    
    bool bDidSomeOutput = false;
    for(int n = uMax; n > 0; n/=10) {
        int x = nNum/n;
        if(x || bDidSomeOutput){
            bDidSomeOutput = true;
            UsefulOutBuf_AppendByte(&OutBuf, '0' + x);
            nNum -= x * n;
        }
    }
    if(!bDidSomeOutput){
        UsefulOutBuf_AppendByte(&OutBuf, '0');
    }
    UsefulOutBuf_AppendByte(&OutBuf, '\0');
    
    return UsefulOutBuf_GetError(&OutBuf) ? "" : StringMem.ptr;
}



typedef int (test_fun_t)(void);

#define TEST_ENTRY(test_name)  {#test_name, test_name}
typedef struct {
    const char *szTestName;
    test_fun_t  *test_fun;
} test_entry;

test_entry s_tests[] = {
    TEST_ENTRY(ParseTooDeepArrayTest),
    TEST_ENTRY(ComprehensiveInputTest),
    TEST_ENTRY(ParseMapTest),
    TEST_ENTRY(indefinite_length_decode_test),
    TEST_ENTRY(basic_test_one),
    TEST_ENTRY(NestedMapTest),
    TEST_ENTRY(BignumParseTest),
    TEST_ENTRY(OptTagParseTest),
    TEST_ENTRY(DateParseTest),
    TEST_ENTRY(ParseSimpleTest),
    TEST_ENTRY(ShortBufferParseTest2),
    TEST_ENTRY(ShortBufferParseTest),
    TEST_ENTRY(ParseDeepArrayTest),
    TEST_ENTRY(SimpleArrayTest),
    TEST_ENTRY(IntegerValuesParseTest),
    TEST_ENTRY(mempool_test),
    TEST_ENTRY(indefinite_length_decode_string_test),
    TEST_ENTRY(half_precision_encode_basic),
    TEST_ENTRY(half_precision_decode_basic),
    TEST_ENTRY(half_precision_to_float_transitive_test),
    TEST_ENTRY(double_as_smallest_encode_basic),
    TEST_ENTRY(half_precision_to_float_vs_rfc_test),
    TEST_ENTRY(bstrwraptest),
    TEST_ENTRY(bstr_wrap_error_test),
    TEST_ENTRY(bstr_wrap_nest_test),
    TEST_ENTRY(cose_sign1_tbs_test),
    //TEST_ENTRY(fail_test),
};


int run_tests(outputstring output, void *poutCtx, int *pNumTestsRun)
{
    int nTestsFailed = 0;
    int nTestsRun = 0;
    UsefulBuf_MakeStackUB(StringStorage, 5);
    
    test_entry *t;
    const test_entry *s_tests_end = s_tests + sizeof(s_tests)/sizeof(test_entry);
    
    for(t = s_tests; t < s_tests_end; t++) {
        int x = (t->test_fun)();
        nTestsRun++;
        if(output) {
            (*output)(t->szTestName, poutCtx);
        }
        
        if(x) {
            if(output) {
                (*output)(" FAILED (returned ", poutCtx);
                (*output)(NumToString(x, StringStorage), poutCtx);
                (*output)(")\n", poutCtx);
            }
            nTestsFailed++;
        } else {
            if(output) {
                (*output)( " PASSED\n", poutCtx);
            }
        }
    }
    
    if(pNumTestsRun) {
        *pNumTestsRun = nTestsRun;
    }
    
    if(output) {
        (*output)( "SUMMARY: ", poutCtx);
        (*output)( NumToString(nTestsRun, StringStorage), poutCtx);
        (*output)( " tests run; ", poutCtx);
        (*output)( NumToString(nTestsFailed, StringStorage), poutCtx);
        (*output)( " tests failed\n", poutCtx);
    }
    
    return nTestsFailed;
}