#include "parameters.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libconfig.h>

#include <argtable2.h>
#include <regex.h>

ConfigError getParameters(int argc, char** argv, Parameters* p)
{

    ConfigError exitcode = E_SOMETHING_BAD;

    // set return structure defaults
    p->program_action      = 0;
    p->max_revs            = 0;
    p->verbosity_count     = 0;

    // setup argument handling structures
    const char* progname = "monocoque";

    struct arg_lit* arg_verbosity1   = arg_litn("v","verbose", 0, 2, "increase logging verbosity");
    struct arg_lit* arg_verbosity2   = arg_litn("v","verbose", 0, 2, "increase logging verbosity");

    struct arg_rex* cmd1             = arg_rex1(NULL, NULL, "play", NULL, REG_ICASE, NULL);
    struct arg_str* arg_sim          = arg_strn("s", "sim", "<gamename>", 0, 1, NULL);
    struct arg_lit* help             = arg_litn(NULL,"help", 0, 1, "print this help and exit");
    struct arg_lit* vers             = arg_litn(NULL,"version", 0, 1, "print version information and exit");
    struct arg_end* end1             = arg_end(20);
    void* argtable1[]                = {cmd1,arg_sim,arg_verbosity1,help,vers,end1};
    int nerrors1;

    struct arg_rex* cmd2a            = arg_rex1(NULL, NULL, "config", NULL, REG_ICASE, NULL);
    struct arg_rex* cmd2b            = arg_rex1(NULL, NULL, "tachometer", NULL, REG_ICASE, NULL);
    struct arg_int* arg_max_revs     = arg_int1("m", "max_revs",NULL,"specify max revs of tachometer");
    struct arg_int* arg_granularity  = arg_int0("g", "granularity",NULL,"1 every 1000 revs, 2 every 500 revs, 4 every 250 revs, default 1");
    struct arg_file* arg_save        = arg_filen("s", "savefile", "<savefile>", 1, 1, NULL);
    struct arg_lit* help2            = arg_litn(NULL,"help", 0, 1, "print this help and exit");
    struct arg_lit* vers2            = arg_litn(NULL,"version", 0, 1, "print version information and exit");
    struct arg_end* end2             = arg_end(20);
    void* argtable2[]                = {cmd2a,cmd2b,arg_max_revs,arg_granularity,arg_save,arg_verbosity2,help2,vers2,end2};
    int nerrors2;

    struct arg_lit*  help0           = arg_lit0(NULL,"help",     "print this help and exit");
    struct arg_lit*  version0        = arg_lit0(NULL,"version",  "print version information and exit");
    struct arg_end*  end0            = arg_end(20);
    void* argtable0[]                = {help0,version0,end0};
    int nerrors0;

    if (arg_nullcheck(argtable0) != 0)
    {
        printf("%s: insufficient memory\n",progname);
        goto cleanup;
    }
    if (arg_nullcheck(argtable1) != 0)
    {
        printf("%s: insufficient memory\n",progname);
        goto cleanup;
    }
    if (arg_nullcheck(argtable2) != 0)
    {
        printf("%s: insufficient memory\n",progname);
        goto cleanup;
    }

    arg_granularity->ival[0] = 1;

    nerrors0 = arg_parse(argc,argv,argtable0);
    nerrors1 = arg_parse(argc,argv,argtable1);
    nerrors2 = arg_parse(argc,argv,argtable2);

    if (nerrors1==0)
    {
        p->program_action = A_PLAY;
        p->sim_string = arg_sim->sval[0];
        p->verbosity_count = arg_verbosity1->count;
        exitcode = E_SUCCESS_AND_DO;
    }
    else
        if (nerrors2==0)
        {
            p->program_action = A_CONFIG_TACH;
            p->max_revs = arg_max_revs->ival[0];
            p->granularity = 1;
            if (arg_granularity->ival[0] > 0 && arg_granularity->ival[0] < 5 && arg_granularity->ival[0] != 3)
            {
                p->granularity=arg_granularity->ival[0];
            }
            p->save_file = *arg_save->filename;
            p->verbosity_count = arg_verbosity2->count;
            exitcode = E_SUCCESS_AND_DO;
        }
        else
        {
            if (cmd1->count > 0)
            {
                arg_print_errors(stdout,end1,progname);
                printf("Usage: %s ", progname);
                arg_print_syntax(stdout,argtable1,"\n");
            }
            else
                if (cmd2a->count > 0)
                {
                    arg_print_errors(stdout,end2,progname);
                    printf("Usage: %s ", progname);
                    arg_print_syntax(stdout,argtable2,"\n");
                }
                else
                {
                    if (help->count==0 && vers->count==0)
                    {
                        printf("%s: missing <play|config> command.\n",progname);
                        printf("Usage 1: %s ", progname);
                        arg_print_syntax(stdout,argtable1,"\n");
                        printf("Usage 2: %s ", progname);
                        arg_print_syntax(stdout,argtable2,"\n");

                    }
                }
            exitcode = E_SUCCESS_AND_EXIT;
            goto cleanup;
        }

    // interpret some special cases before we go through trouble of reading the config file
    if (help->count > 0)
    {
        printf("Usage: %s\n", progname);
        printf("Usage 1: %s ", progname);
        arg_print_syntax(stdout,argtable1,"\n");
        printf("Usage 2: %s ", progname);
        arg_print_syntax(stdout,argtable2,"\n");
        printf("\nReport bugs on the github github.com/spacefreak18/monocoque.\n");
        exitcode = E_SUCCESS_AND_EXIT;
        goto cleanup;
    }

    if (vers->count > 0)
    {
        printf("%s Simulator Hardware Manager\n",progname);
        printf("October 2022, Paul Dino Jones\n");
        exitcode = E_SUCCESS_AND_EXIT;
        goto cleanup;
    }

cleanup:
    arg_freetable(argtable0,sizeof(argtable0)/sizeof(argtable0[0]));
    arg_freetable(argtable1,sizeof(argtable1)/sizeof(argtable1[0]));
    arg_freetable(argtable2,sizeof(argtable2)/sizeof(argtable2[0]));
    return exitcode;

}
