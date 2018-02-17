//
// Created by Bas du Pré on 17-02-18.
//

#ifndef WICKEDC_FRONTEND_H
#define WICKEDC_FRONTEND_H

extern int test_testlib_assert;

extern "C" {

#include "funkyvm/funkyvm.h"

CPU_State compile_and_run(const char *wckd_code);

};

#endif //WICKEDC_FRONTEND_H
