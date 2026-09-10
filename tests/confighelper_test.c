// Proof-of-plumbing test for the new CTest wiring — not an attempt at
// comprehensive coverage. Exercises strtodevsubsubtype (helper/confighelper.c),
// a pure string->enum mapping with no I/O/hardware, including the R8/R3
// aliasing onto SIMDEVSUBTYPE_MOZAR5 and the fallback for unrecognized input.
#include <stdio.h>
#include <string.h>

#include "../src/monocoque/helper/confighelper.h"

static int failures = 0;

static void check(const char* input, DeviceSubSubType expected)
{
    DeviceSettings ds = {0};
    strtodevsubsubtype(input, &ds);
    if (ds.dev_subsubtype != expected)
    {
        fprintf(stderr, "FAIL: strtodevsubsubtype(\"%s\") = %d, expected %d\n",
                input, ds.dev_subsubtype, expected);
        failures++;
    }
}

int main(void)
{
    check("MozaR5", SIMDEVSUBTYPE_MOZAR5);
    check("MozaR8", SIMDEVSUBTYPE_MOZAR5);   // aliased onto the same protocol as R5
    check("MozaR3", SIMDEVSUBTYPE_MOZAR5);   // same
    check("MozaNew", SIMDEVSUBTYPE_MOZA_NEW);
    check("MozaKSProWheel", SIMDEVSUBTYPE_MOZA_KS_PRO_WHEEL);
    check("CammusC5", SIMDEVSUBTYPE_CAMMUSC5);
    check("CammusC12", SIMDEVSUBTYPE_CAMMUSC12);
    check("LogitechG29", SIMDEVSUBTYPE_LOGITECH_G29);
    check("not-a-real-subtype", SIMDEVSUBTYPE_UNKNOWN);
    check("", SIMDEVSUBTYPE_UNKNOWN);

    if (failures > 0)
    {
        fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }

    printf("confighelper_test: all checks passed\n");
    return 0;
}
