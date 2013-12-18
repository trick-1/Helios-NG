#ifdef __C40
#pragma no_stack_checks
#pragma few_modules
#endif

#ifndef __ARM
#define	FASTRAM 1
#endif

static char *rcsid = "$Header: /dsl/Helios.ARM/cmds/private/RCS/dhry.c,v 1.6 1993/08/10 14:47:36 nickc Exp $";

#include <stdio.h>
#include <task.h>
#include <memory.h>
/*
 *      The following program contains statements of a high-level programming
 *      language (C) in a distribution considered representative:
 *
 *      assignments                     53%
 *      control statements              32%
 *      procedure, function calls       15%
 *
 *      100 statements are dynamically executed.  The program is balanced with
 *      respect to the three aspects:
 *              - statement type
 *              - operand type (for simple data types)
 *              - operand access
 *                      operand global, local, parameter, or constant.
 *
 *      The combination of these three aspects is balanced only approximately.
 *
 *      The program does not compute anything meaningfull, but it is
 *      syntactically and semantically correct.
 *
 */
/* Accuracy of timings and human fatigue controlled by next two lines */
#define LOOPS   50000           /* Use this for slow or 16 bit machines */
/*#define LOOPS 500000          /* Use this for faster machines */
/* Compiler dependent options */
#undef  NOENUM                  /* Define if compiler has no enum's */
#undef  NOSTRUCTASSIGN          /* Define if compiler can't assign structures */
#ifdef NOSTRUCTASSIGN
#define memcpy xcopy
#endif
/* define only one of the next two defines */
/*#define TIMES*/                       /* Use times(2) time function */
/*#define TIME*/                        /* Use time(2) time function */
#define CLOCK                           /* use clock() time function */
/* define the granularity of your times(2) function (when used) */
#define HZ      60              /* times(2) returns 1/60 second (most) */
/*#define HZ    100             /* times(2) returns 1/100 second (WECo) */
/* for compatibility with goofed up version */
/*#define GOOF */                  /* Define if you want the goofed up version */
#ifdef GOOF
char    Version[] = "1.0";
#else
char    Version[] = "1.1";
#endif
#ifdef  NOSTRUCTASSIGN
#define structassign(d, s)      memcpy(&(d), &(s), sizeof(d))
#else
#define structassign(d, s)      d = s
#endif
#ifdef  NOENUM
#define Ident1  1
#define Ident2  2
#define Ident3  3
#define Ident4  4
#define Ident5  5
typedef int     Enumeration;
#else
typedef enum    {Ident1, Ident2, Ident3, Ident4, Ident5} Enumeration;
#endif
typedef int     OneToThirty;
typedef int     OneToFifty;
typedef char    CapitalLetter;
typedef char    String30[31];
typedef int     Array1Dim[51];
typedef int     Array2Dim[51][51];
struct  Record
{
        struct Record           *PtrComp;
        Enumeration             Discr;
        Enumeration             EnumComp;
        OneToFifty              IntComp;
        String30                StringComp;
};
typedef struct Record   RecordType;
typedef RecordType *    RecordPtr;
typedef int             boolean;
#define NULL            0
#ifndef TRUE
# define TRUE            1
# define FALSE           0
#endif
#ifndef REG
#define REG
#endif
extern Enumeration      Func1();
extern boolean          Func2();
#ifdef TIMES
#include <sys/types.h>
#include <sys/times.h>
#endif
#ifdef CLOCK
/* #include <time.h> */
typedef int clock_t;
#endif
char lbuf[80];
int dhrystones = 0;
int ischild = 0;
main(int argc, char **argv)
{
	int loops;
#if FASTRAM
	void Accelerate(Carrier *, ...);
	extern Task *MyTask;
	Carrier *carrier;
#endif
	void Proc0(int loops);
	(void) setvbuf(stdout, lbuf, _IOLBF, 80);
	
	loops = argc<2?LOOPS:atoi(argv[1]);

	ischild = argc>=2;

#if FASTRAM
	carrier = AllocFast(1500,&MyTask->MemPool);

	if( carrier == NULL ) exit(0);

        Accelerate(carrier,Proc0,4,loops);
#else
	Proc0(loops);
#endif
        exit(dhrystones);
}
/*
 * Package 1
 */
static int             IntGlob;
static boolean         BoolGlob;
static char            Char1Glob;
static char            Char2Glob;
static RecordPtr       PtrGlb;
static RecordPtr       PtrGlbNext;
static Array1Dim       Array1Glob;
static Array2Dim       Array2Glob;
Proc0(int loops)
{
        OneToFifty              IntLoc1;
        REG OneToFifty          IntLoc2;
        OneToFifty              IntLoc3;
        REG char                CharLoc;
        REG char                CharIndex;
        Enumeration             EnumLoc;
        String30                String1Loc;
        String30                String2Loc;
        extern char             *Malloc();
#ifdef TIME
        long                    time();
        long                    starttime;
        long                    benchtime;
        long                    nulltime;
        register unsigned int   i;
        starttime = time( (long *) 0);
        for (i = 0; i < loops; ++i);
        nulltime = time( (long *) 0) - starttime; /* Computes o'head of loop */
#endif
#ifdef TIMES
        time_t                  starttime;
        time_t                  benchtime;
        time_t                  nulltime;
        struct tms              tms;
        register unsigned int   i;
        times(&tms); starttime = tms.tms_utime;
        for (i = 0; i < loops; ++i);
        times(&tms);
        nulltime = tms.tms_utime - starttime; /* Computes overhead of looping */
#endif
#ifdef CLOCK
        clock_t                 starttime;
        clock_t                 benchtime;
        clock_t                 nulltime;
        register unsigned int   i;
        starttime = clock();
        for (i = 0; i < loops; ++i);
        nulltime = clock() - starttime; /* Computes overhead of looping */
#endif
#ifndef CLOCK
register unsigned int i;
#endif
        PtrGlbNext = (RecordPtr) Malloc(sizeof(RecordType));
        PtrGlb = (RecordPtr) Malloc(sizeof(RecordType));
        PtrGlb->PtrComp = PtrGlbNext;
        PtrGlb->Discr = Ident1;
        PtrGlb->EnumComp = Ident3;
        PtrGlb->IntComp = 40;
        strcpy(PtrGlb->StringComp, "DHRYSTONE PROGRAM, SOME STRING");
#ifndef GOOF
        strcpy(String1Loc, "DHRYSTONE PROGRAM, 1'ST STRING");   /*GOOF*/
#endif
        Array2Glob[8][7] = 10;  /* Was missing in published program */

/*****************
-- Start Timer --
*****************/
#ifdef TIME
        starttime = time( (long *) 0);
#endif
#ifdef TIMES
        times(&tms); starttime = tms.tms_utime;
#endif
#ifdef CLOCK
        starttime = clock();
#endif
        for (i = 0; i < loops; ++i)
        {
                Proc5();
                Proc4();
                IntLoc1 = 2;
                IntLoc2 = 3;
                strcpy(String2Loc, "DHRYSTONE PROGRAM, 2'ND STRING");
                EnumLoc = Ident2;
                BoolGlob = ! Func2(String1Loc, String2Loc);
                while (IntLoc1 < IntLoc2)
                {
                        IntLoc3 = 5 * IntLoc1 - IntLoc2;
                        Proc7(IntLoc1, IntLoc2, &IntLoc3);
                        ++IntLoc1;
                }
                Proc8(Array1Glob, Array2Glob, IntLoc1, IntLoc3);
                Proc1(PtrGlb);
                for (CharIndex = 'A'; CharIndex <= Char2Glob; ++CharIndex)
                        if (EnumLoc == Func1(CharIndex, 'C'))
                                Proc6(Ident1, &EnumLoc);
                IntLoc3 = IntLoc2 * IntLoc1;
                IntLoc2 = IntLoc3 / IntLoc1;
                IntLoc2 = 7 * (IntLoc3 - IntLoc2) - IntLoc1;
                Proc2(&IntLoc1);
        }
/*****************
-- Stop Timer --
*****************/
#ifdef TIME
        benchtime = time( (long *) 0) - starttime - nulltime;
        printf("Dhrystone(%s) time for %ld passes = %ld\n",
                Version,
                (long) loops, benchtime);
        printf("This machine benchmarks at %ld dhrystones/second\n",
                ((long) loops) / benchtime);
#endif
#ifdef TIMES
        times(&tms);
        benchtime = tms.tms_utime - starttime - nulltime;
        printf("Dhrystone(%s) time for %ld passes = %ld\n",
                Version,
                (long) loops, benchtime/HZ);
        printf("This machine benchmarks at %ld dhrystones/second\n",
                ((long) loops) * HZ / benchtime);
#endif
#ifdef CLOCK
        benchtime = clock() - starttime - nulltime;
	if( ischild )
	{
		dhrystones = ((long) loops) * 100 / benchtime;
		return;
	}
        printf("Dhrystone(%s) time for %ld passes = %ld\n",
                Version,
                (long) loops, benchtime / 100);
        printf("This machine benchmarks at %ld dhrystones/second\n",
                ((long) loops) * 100 / benchtime);
#endif
}
Proc1(PtrParIn)
REG RecordPtr   PtrParIn;
{
#define NextRecord      (*(PtrParIn->PtrComp))
        structassign(NextRecord, *PtrGlb);
        PtrParIn->IntComp = 5;
        NextRecord.IntComp = PtrParIn->IntComp;
        NextRecord.PtrComp = PtrParIn->PtrComp;
        Proc3(NextRecord.PtrComp);
        if (NextRecord.Discr == Ident1)
        {
                NextRecord.IntComp = 6;
                Proc6(PtrParIn->EnumComp, &NextRecord.EnumComp);
                NextRecord.PtrComp = PtrGlb->PtrComp;
                Proc7(NextRecord.IntComp, 10, &NextRecord.IntComp);
        }
        else
                structassign(*PtrParIn, NextRecord);
#undef  NextRecord
}
Proc2(IntParIO)
OneToFifty      *IntParIO;
{
        REG OneToFifty          IntLoc;
        REG Enumeration         EnumLoc;
        IntLoc = *IntParIO + 10;
        for(;;)
        {
                if (Char1Glob == 'A')
                {
                        --IntLoc;
                        *IntParIO = IntLoc - IntGlob;
                        EnumLoc = Ident1;
                }
                if (EnumLoc == Ident1)
                        break;
        }
}
Proc3(PtrParOut)
RecordPtr       *PtrParOut;
{
        if (PtrGlb != NULL)
                *PtrParOut = PtrGlb->PtrComp;
        else
                IntGlob = 100;
        Proc7(10, IntGlob, &PtrGlb->IntComp);
}
Proc4()
{
        REG boolean     BoolLoc;
        BoolLoc = Char1Glob == 'A';
        BoolLoc |= BoolGlob;
        Char2Glob = 'B';
}
Proc5()
{
        Char1Glob = 'A';
        BoolGlob = FALSE;
}
extern boolean Func3();
Proc6(EnumParIn, EnumParOut)
REG Enumeration EnumParIn;
REG Enumeration *EnumParOut;
{
        *EnumParOut = EnumParIn;
        if (! Func3(EnumParIn) )
                *EnumParOut = Ident4;
        switch (EnumParIn)
        {
        case Ident1:    *EnumParOut = Ident1; break;
        case Ident2:    if (IntGlob > 100) *EnumParOut = Ident1;
                        else *EnumParOut = Ident4;
                        break;
        case Ident3:    *EnumParOut = Ident2; break;
        case Ident4:    break;
        case Ident5:    *EnumParOut = Ident3;
        }
}
Proc7(IntParI1, IntParI2, IntParOut)
OneToFifty      IntParI1;
OneToFifty      IntParI2;
OneToFifty      *IntParOut;
{
        REG OneToFifty  IntLoc;
        IntLoc = IntParI1 + 2;
        *IntParOut = IntParI2 + IntLoc;
}
Proc8(Array1Par, Array2Par, IntParI1, IntParI2)
Array1Dim       Array1Par;
Array2Dim       Array2Par;
OneToFifty      IntParI1;
OneToFifty      IntParI2;
{
        REG OneToFifty  IntLoc;
        REG OneToFifty  IntIndex;
        IntLoc = IntParI1 + 5;
        Array1Par[IntLoc] = IntParI2;
        Array1Par[IntLoc+1] = Array1Par[IntLoc];
        Array1Par[IntLoc+30] = IntLoc;
        for (IntIndex = IntLoc; IntIndex <= (IntLoc+1); ++IntIndex)
                Array2Par[IntLoc][IntIndex] = IntLoc;
        ++Array2Par[IntLoc][IntLoc-1];
        Array2Par[IntLoc+20][IntLoc] = Array1Par[IntLoc];
        IntGlob = 5;
}
Enumeration Func1(CharPar1, CharPar2)
CapitalLetter   CharPar1;
CapitalLetter   CharPar2;
{
        REG CapitalLetter       CharLoc1;
        REG CapitalLetter       CharLoc2;
        CharLoc1 = CharPar1;
        CharLoc2 = CharLoc1;
        if (CharLoc2 != CharPar2)
                return (Ident1);
        else
                return (Ident2);
}
boolean Func2(StrParI1, StrParI2)
String30        StrParI1;
String30        StrParI2;
{
        REG OneToThirty         IntLoc;
        REG CapitalLetter       CharLoc;
        IntLoc = 1;
        while (IntLoc <= 1)
                if (Func1(StrParI1[IntLoc], StrParI2[IntLoc+1]) == Ident1)
                {
                        CharLoc = 'A';
                        ++IntLoc;
                }
        if (CharLoc >= 'W' && CharLoc <= 'Z')
                IntLoc = 7;
        if (CharLoc == 'X')
                return(TRUE);
        else
        {
                if (strcmp(StrParI1, StrParI2) > 0)
                {
                        IntLoc += 7;
                        return (TRUE);
                }
                else
                        return (FALSE);
        }
}
boolean Func3(EnumParIn)
REG Enumeration EnumParIn;
{
        REG Enumeration EnumLoc;
        EnumLoc = EnumParIn;
        if (EnumLoc == Ident3) return (TRUE);
        return (FALSE);
}
#ifdef  NOSTRUCTASSIGN
memcpy(d, s, l)
register char   *d;
register char   *s;
register int    l;
{
        while (l--) *d++ = *s++;
}
#endif
#ifdef  MAC
#define TIMELOC 0x16A
long time(parm)
long parm;
{
register        long *p;
                        p = TIMELOC;
                        return(*p/60L);
}
#endif
