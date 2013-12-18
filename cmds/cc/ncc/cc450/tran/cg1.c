/*{{{  Includes */
#ifdef __old
#include "cchdr.h"
#include "AEops.h"
#include "util.h"
#include "xpuops.h"
#include "cg.h"
#else
#include "globals.h"
#include "builtin.h"
#include "store.h"
#include "util.h"
#include "aeops.h"
#include "xpuops.h"
#include "cg.h"
#endif
/*}}}*/
/*{{{  Externs and defines */

extern int maxssp;
extern int ssp;
extern int maxcall;
#if 0
/* Replaced by pragma check library_module.  Tony 23/1/95 */
extern int nomodule;
extern int libfile;
#endif

static int incall;		/* counts number of nested calls */

extern Tcode *curcode;
extern int deadcode;

/* forward references */
void cg_cond();
void cg_count();

#define isextfnptr(e) 				\
((h0_(e) == s_addrof) && (arg1_(e) != NULL) && 	\
(h0_(arg1_(e)) == s_binder) && 			\
((((Binder *)(arg1_(e)))->bindstg & (bitofstg_(s_extern)|b_fnconst)) == (bitofstg_(s_extern)|b_fnconst)))
/*}}}*/
/*{{{  cg_test1 */

/* valuep is true iff we need the real boolean value at the end */
static void cg_test1(x,branchtrue,dest,valuep)
Expr *x; bool branchtrue; LabelNumber *dest; bool valuep;
{
    Expr *x1, *x2;

    push_trace("cg_test");
    
    if(debugging(DEBUG_CG)) 
	trace("Cg_test: %x %d %x %x %x",x,h0_(x),branchtrue,dest,valuep);

    if (x == 0) { syserr("missing expr"); { pop_trace(); return; } }

    verify_integer(x);
    if (integer_constant(x))
    {   if ((result2==0 && !branchtrue) ||
            (result2!=0 && branchtrue)) 
        { /* This is necessary so that a proper boolean arrives 
             at the target label in all cases */
            if (valuep) 
            {
                pushInt();
                emit(p_j, dest); popInt();
            }
            else emit(f_j, dest);
        }
        { pop_trace(); return; }
    }
    else switch (h0_(x))
    {

default:    /* Maybe this needs eqc 0 eqc 0 if valuep ? */
            cg_expr(x);
    	    if( branchtrue ) emit( f_eqc, 0 );
    	    emit( f_cj, dest ); popInt();
	    { pop_trace(); return; }


case s_string: 
            if (branchtrue) 
	    {
                if (valuep) 
                {
                    pushInt();
                    emit(p_j, dest);popInt();
                }
                else emit(f_j, dest);
	    }
            { pop_trace(); return; }

case s_andand:
            if (branchtrue)
            {   LabelNumber *l = nextlabel();
                cg_test1(arg1_(x), FALSE, l, valuep);
                cg_count();
                cg_test1(arg2_(x), TRUE, dest, valuep);
                start_new_basic_block(l);
                { pop_trace(); return; }
            }
            else
            {   cg_test1(arg1_(x), FALSE, dest, valuep);
                cg_count();
                cg_test1(arg2_(x), FALSE, dest,valuep);
                { pop_trace(); return; }
            }

case s_oror:
            if (!branchtrue)
            {   LabelNumber *l = nextlabel();
                cg_test1(arg1_(x), TRUE, l, valuep);
                cg_count();
                cg_test1(arg2_(x), FALSE, dest, valuep);
                start_new_basic_block(l);
                { pop_trace(); return; }
            }
            else
            {   cg_test1(arg1_(x), TRUE, dest,valuep);
                cg_count();
                cg_test1(arg2_(x), TRUE, dest,valuep);
                { pop_trace(); return; }
            }

case s_boolnot:
            cg_test1(arg1_(x), !branchtrue, dest,valuep);
            { pop_trace(); return; }

case s_comma:
            cg_exprvoid(arg1_(x));
            cg_count();
            cg_test1(arg2_(x), branchtrue, dest,valuep);
            { pop_trace(); return; }

case s_monplus:
/* Monadic plus does not have to generate any code                       */
            cg_test1(arg1_(x), branchtrue, dest,valuep);
            { pop_trace(); return; }

case s_notequal:
	    branchtrue = !branchtrue;
case s_equalequal:
	    /* only optimise for eqc if the value is small */
	    if( integer_constant(arg2_(x)) && smallint(result2) )
	    {
		int r2 = result2;
		cg_expr(arg1_(x));	    
	    	if( valuep || r2 != 0 ) emit( f_eqc, r2 );
		else branchtrue = !branchtrue;
	    }
	    else if( integer_constant(arg1_(x)) && smallint(result2) )
	    {
		int r2 = result2;
		cg_expr(arg2_(x));
	    	if( valuep || r2 != 0 ) emit( f_eqc, r2 );
		else branchtrue = !branchtrue;
	    }
#ifdef never
	    else if( isextfnptr(arg1_(x)) )
	    {
	    	cg_expr(arg2_(x));
	    	emit( p_ldx, arg1_(arg1_(x)) );
		emit( f_opr, op_diff );
		if (valuep) emit( f_eqc, 0);
		else branchtrue = !branchtrue;
	    }
	    else if( isextfnptr(arg2_(x)) )
	    {
	    	cg_expr(arg1_(x));
	    	emit( p_ldx, arg1_(arg2_(x)) );
	    	emit( f_opr, op_diff );
		if (valuep) emit( f_eqc, 0);
		else branchtrue = !branchtrue;
	    }
#endif
	    else
            {
		int rep = mcrepofexpr( arg1_(x) );
		RegSort rsort = (rep&0xff000000)!=0x02000000 ? INTREG :
			      ( (rep&0x00ffffff)==4 ? FLTREG : DBLREG );

		if( rsort != INTREG )
		{
		    if( !floatingHardware && rsort == DBLREG )
		    {
			cg_doublecmp( s_equalequal, arg1_(x), arg2_(x));
		    }
		    else cg_binary( s_equalequal, arg1_(x), arg2_(x), TRUE, rsort );
		}
		else {
			cg_binary( s_diff, arg1_(x), arg2_(x), TRUE, rsort );
			if (valuep) emit( f_eqc,0);
			else branchtrue = !branchtrue;
		}
	    }
            break;

case s_lessequal:
	    branchtrue = !branchtrue;
case s_greater:
	    x1 = arg1_(x);
	    x2 = arg2_(x);
	    goto gengt;

case s_greaterequal:
	    branchtrue = !branchtrue;
case s_less:
	    x1 = arg2_(x);
	    x2 = arg1_(x);
gengt:
	    {
		int rep = mcrepofexpr( arg1_(x) );
		RegSort rsort = (rep&0xff000000)!=0x02000000 ? INTREG :
			      ( (rep&0x00ffffff)==4 ? FLTREG : DBLREG );

		if( !floatingHardware && rsort == DBLREG )
		{
			cg_doublecmp( s_greater, x1, x2 );
		}
	    	else cg_binary( s_greater,x1, x2, 0, rsort );
	    }
            break;

    }
    if( branchtrue ) emit( f_eqc, 0 );
    emit( f_cj, dest );
    if( !valuep ) popInt();
    pop_trace();
}
/*}}}*/
/*{{{  cg_test */

void cg_test(x,branchtrue,dest)
Expr *x; int branchtrue; LabelNumber *dest;
{
    cg_count();
    cg_test1(x, branchtrue, dest, FALSE);
}
/*}}}*/
/*{{{  cg_loop */
void cg_loop(init,pretest,step,body,posttest)
Expr *init; Expr *pretest; Expr *step; Cmd *body;
Expr *posttest;
{
/* Here I deal with all loops. Many messy things are going on!           */
    LabelNumber *l1 = nextlabel(), *l2 = 0;
    struct LoopInfo oloopinfo;
    int oanylabels;
    Block *blkhead;
    BlockList *othisloop;
    int oinloop;
    int unwind = FALSE;
    
    oloopinfo = loopinfo;
/* A large amount of status belongs with loop constructs, and gets saved */
/* here so that it can be restored at the end of compiling the loop.     */
    loopinfo.breaklab = loopinfo.contlab = 0;
    loopinfo.binders = active_binders;

    if(init!=0) cg_exprvoid(init);  /* the initialiser (if any)          */

    oanylabels = anylabels;
    anylabels = loopseen = FALSE;

/* Decide whether this loop is a candidate for being unwound. We only	*/
/* unwind loops which have a (simple) pretest and a simple or no	*/
/* step expression. Also the body should be relatively simple.		*/
/* @@@ develop some sort of metric for this...				*/

#if 0
	unwind = TRUE; /* for now unwind all loops			*/
#else
	unwind = FALSE;
#endif

/* @@@@@ THIS NEEDS REVISING BECAUSE SHOWCODE may well optimise these    */
/* branches away again ...                                               */
/* The transputer only timeslices on unconditional jumps, so ALL loops	 */
/* must contain one. This means that the posttest must be moved to the   */
/* top of the loop with an unconditonal jump at the end. Here I put out  */
/* a jump round the posttest first time through.			 */
/* The noop2 forces this block to appear to be non-empty, otherwise the	 */
/* optimiser may well try to eliminate it with funny results.		 */

	if( posttest != 0 )
	{
		emit( p_noop2 , 0 );
		emit( f_j, l2=nextlabel() );
		pretest = posttest;
	}

#ifdef never
/* Establish a block (which will be left empty for now) that marks the   */
/* entry to this loop. This block may be extended later to set up values */
/* of loop invariants.                                                   */
    blkhead = start_new_basic_block(nextlabel());
#endif

    loopinfo.breaklab=nextlabel();

    start_new_basic_block(l1);

    align_block();		/* cause loop head label to be aligned */
    
/* When cg_loop is called there can NEVER be both a pretest and a post-  */
/* test specified. Here I put out the conditional branch to              */
/* the end of the loop.                                                  */

    if (pretest!=0) cg_test(pretest, FALSE, loopinfo.breaklab );

    othisloop = this_loop;
    oinloop = in_loop;
    this_loop = NULL;
    in_loop = TRUE;

    if( l2 ) start_new_basic_block(l2);

    cg_count();
    cg_cmd(body);

    /* if we are unwinding, we generate the test, body and step	twice	*/
    /* more. This is based on the observation that loops are executed	*/
    /* three times on average.						*/
    
    if( unwind )
    {
    	if(step!=0) cg_exprvoid(step);
    	if( pretest ) cg_test(pretest, FALSE, loopinfo.breaklab );
    	cg_cmd(body);
    	if(step!=0) cg_exprvoid(step);
    	if( pretest ) cg_test(pretest, FALSE, loopinfo.breaklab );
    	cg_cmd(body);
    }

    if (loopinfo.contlab) start_new_basic_block(loopinfo.contlab);
    if(step!=0) cg_exprvoid(step);


    emit(f_j, l1);

#ifdef never
/* I count this as a 'proper' loop, (a candidate for loop invariant      */
/* analysis) if there are no case labels or ordinary labels within it.   */
/* (note that case labels within an enclosed switch() do not count)      */
/* I also record whether there are any loops (clean or otherwise) inside */
/* this one.                                                             */

    if (!anylabels)
    {   blkflags_(blkhead) |= BLKLOOP;
        if (!loopseen) blkflags_(blkhead) |= BLKINNER;
        all_loops = mkLoopList(all_loops, this_loop, blkhead);
        if (oinloop)
        {   for (; this_loop!=NULL; this_loop = this_loop -> blklstcdr)
                othisloop = mkBlockList(othisloop, this_loop -> blklstcar);
            this_loop = othisloop;
        }
        else in_loop = FALSE;
    }
/* If this was not a kosher loop the next one out still may be           */
    else if (oinloop)
    {   while (othisloop!=NULL)
        {   BlockList *p = othisloop;
            othisloop = p->blklstcdr;
            p->blklstcdr = this_loop;
            this_loop = p;
        }
    }
    else
    {   while (this_loop!=NULL)
            this_loop = (BlockList *)discard2((List *)this_loop);
        this_loop = othisloop;
        in_loop = FALSE;
    }
#endif
/* End of the loop, I guess. Restore all context.                        */
    if (loopinfo.breaklab) start_new_basic_block(loopinfo.breaklab);
    anylabels |= oanylabels;
    loopseen = TRUE;
    loopinfo = oloopinfo;
    in_loop = oinloop;
    this_loop = othisloop;
    cg_count();
}
/*}}}*/
/*{{{  take_address */
Expr *take_address(e)
Expr *e;
{
    if (h0_(e)==s_content) return arg1_(e);     /*   & * x   --->  x     */
    else if (h0_(e)==s_dot)                     /*   &(x.y)  ---> (&x)+y */
        e =     mk_expr2(s_plus,
                        ptrtotype_(type_(e)),
                        take_address(arg1_(e)),
                        mkintconst(te_int, exprdotoff_(e), 0));
    else if (h0_(e)==s_cast) return take_address(arg1_(e));  /* ignore casts */
    else e =     mk_expr1(s_addrof,
                         ptrtotype_(typeofexpr(e)),
                         e);

    pp_expr(e,1);
    return e;
}
/*}}}*/
/*{{{  cg_fnap */

/* The following typedef is local to cg_fnargs() and cg_fnap() */
typedef struct ArgInfo { Expr *expr; int info, rep; } ArgInfo;

void
cg_fnap(x,rsort,valneeded)
Expr *x; RegSort rsort; bool valneeded;
{
/* Compile a call to a function - being tidied!                          */
    int argwords;
    BindList *save_binders = active_binders;
    int d = current_stackdepth;
    int savessp = ssp;
    Expr *fn = arg1_(x);
    Binder *b;
    int ispv = 	!(h0_(fn)==s_addrof && 
		(b = (Binder *)arg1_(fn))!=0 && 
		h0_(b)==s_binder);
    Binder *proc;

	push_trace("cg_fnap");
	
	if( debugging(DEBUG_CG) )
		trace("cg_fnap %s",ispv?"<proc var>":_symname(bindsym_(b)));
		
	/* if this is a procedure variable call, evaluate the proc ptr	*/
	/* first and put it on top of the vector stack.			*/
    	if( ispv ) 
	{
		TypeExpr *t = typeofexpr(fn);
		proc = gentempvar(t,mcrepofexpr(fn));
		cg_exprvoid( mk_expr2(s_assign, t, (Expr *)proc, fn));
	}

	incall++;
/* Work out what the args to the fn are.				  */
/* For functions of 10 args or less, C stack is used, not SynAlloc space: */
#define STACKFNARGS 10  /* n.b. this is *not* a hard limit on no. of args */

    {   ArgInfo v[STACKFNARGS];
        ExprList *a = exprfnargs_(x);
        int n = length(a);
        ArgInfo *p = n > STACKFNARGS ? 
                     (ArgInfo *)SynAlloc(n*sizeof(ArgInfo)) : v;
        argwords = cg_fnargs(a, p, n, ispv);
    }

/* Now the arguments are all in the right places - call the function     */

        if (!ispv)
	{ /* a direct call to the function	*/
		emit( p_call, bindsym_(b) );
 	}             
        else switch (mcrepofexpr(fn))
        {
        case 0x00000004:
        case 0x01000004:
            {
		emit(p_callpv,proc);
                break;
            }
        default:
                syserr("invalid expression used as function");
        }

	if( argwords ) emit( p_fnstack, -argwords );
        current_stackdepth = d;
	ssp = savessp;
        active_binders = save_binders;
	setInt(FullDepth); setFloat(FullDepth);
	if( valneeded )
	{
		if( rsort == INTREG || !floatingHardware ) setInt(2); 
		else setFloat(2); 
	}
	incall--;
	
	pop_trace();
}
/*}}}*/
/*{{{  cg_fnargs */

int cg_fnargs(a, arg, n, ispv)
ExprList *a; ArgInfo *arg; const int n;
const int ispv;
{
	int narg = 0;
	int argwords = 0;
	int savessp = ssp;
	int savemax;
	int argpos, argregs = 0;	
	int fndrop, fnraise;
/* Following line to fix bug to stop opd.value being patched if deadcode */
	int dcode;
	Tcode *t, *t1;

        for (; a != NULL && narg<n; a = cdr_(a), narg++)
        {   int rep = mcrepofexpr(exprcar_(a));
            arg[narg].info = depth(exprcar_(a));
            arg[narg].expr = exprcar_(a);
            arg[narg].rep = rep;
            argwords += pad_to_word(rep & 0x00ffffff)/4;
            if( debugging(DEBUG_CG) ) trace("arg%d depth %d rep %x",narg,arg[narg].info,rep);
        }
        if (n != narg || a != 0) syserr("arg count confused");

/* First see if there are any args which contain function calls. If so	*/
/* do these first and save the result into a temporary.			*/
/* This also applies to structure returning functions.			*/

	for( narg = n; narg ; )
	{
		narg--;
		if( contains_fnap(arg[narg].expr) )
		{
			switch( arg[narg].rep )
			{
			case 0x00000001:
	                case 0x00000002:
			case 0x00000004:
                	case 0x01000001:
                	case 0x01000002:
                	case 0x01000004:	/* integer-like things	*/
			case 0x02000004:	/* single length floats	*/
			case 0x02000008:	/* double length floats	*/ 
			{
				TypeExpr *t = typeofexpr(arg[narg].expr);
				Binder *b = gentempvar(t,arg[narg].rep);
				cg_exprvoid( mk_expr2(s_assign, t, (Expr *)b, 
				             arg[narg].expr));
                                arg[narg].expr = (Expr *)b; 
                                break;
			}

			default:
			/* functions returning structures		*/
			{
				TypeExpr *t = typeofexpr(arg[narg].expr);
				Binder *b = gentempvar(t,arg[narg].rep);
				bindstg_(b) |= s_addrof;
				cg_exprvoid(mk_expr2(s_assign, t, (Expr *)b,
					arg[narg].expr));
                                arg[narg].expr = (Expr *)b; 
				break;
			}
			}
		}
	}
/* Here we have only simple expressions to deal with			*/

	emit( p_fnstack, 0 );		/* to be plugged later		*/
	t = curcode;

	savemax = maxssp;
	maxssp = ssp;
	
	/* see whether any args will go directly into registers, note that */
	/* we only do the second if the first will go			*/
	/* only integer style args can be passed this way	        */
	if( n > 0 && ( arg[0].rep & 0xff000000) < 0x02000000 &&
		 (arg[0].rep & 0x00ffffff) <= 4 )
	{
		argregs++;
		if(  n > 1 && ( arg[1].rep & 0xff000000) < 0x02000000 &&
			(arg[1].rep & 0x00ffffff) <= 4 ) argregs++;
	}

	argpos = argwords-argregs;

   	for( narg = n; narg>argregs ; )
	{
		argpos -= pad_to_word(arg[--narg].rep & 0x00ffffff)/4;

		switch( arg[narg].rep )
		{
		case 0x00000001:
		case 0x00000002:
		case 0x00000004:
		case 0x01000001:
		case 0x01000002:
		case 0x01000004:
			/* integer-like things		*/
			cg_expr(arg[narg].expr);
			emit( f_stl, argpos );
			popInt();
			break;
		
		case 0x02000004:
		        /* floats                      */ 
		        /* we should never actually do this since the	*/
		        /* FE will have cast any floats to double for us*/

			cg_expr(arg[narg].expr);
			if( floatingHardware )
			{
				emit( f_ldlp, argpos );
				emit(f_opr, op_fpstnlsn );
				popFloat();
				break;
			}
			else
			{
				emit( f_stl, argpos );
				popInt();
				break;
			}
	
		case 0x02000008:
			/* doubles		      */
			if( floatingHardware )
			{
				cg_expr(arg[narg].expr);
				emit( f_ldlp, argpos );
				emit(f_opr, op_fpstnldb );
				popFloat();
				break;
			}
			else
			{
				VLocal *olddd = doubledest;
				TypeExpr *t = typeofexpr(arg[narg].expr);
				Binder *b = genfnargbinder(t,argpos,2,8);
				Expr *x;

				x = mk_expr2(s_assign,t,(Expr *)b,arg[narg].expr);

				pp_expr(x,0);
				cg_exprvoid(x);
				doubledest = olddd;
				break;
			}

		default:
		{ /* Structure arguments passed by value		*/
			int structsize = arg[narg].rep & 0x00ffffff;
			if((arg[narg].rep & 0xff000003) != 0x03000000)
			syserr("cg_fnarg(arg%d odd rep %x)",narg,arg[narg].rep);
			cg_expr(take_address(arg[narg].expr));
			emit( f_ldlp, argpos );
			pushInt();
			emit( f_ldc, structsize );
			pushInt();
			emit(f_opr, op_move );
			setInt(FullDepth);
			break;
		}
		}
	}

	switch( argregs )
	{
	case 0:		
	/* neither would go directly, load them now 	*/
		if( argwords > 1 ) { emit( f_ldl, 1 ); pushInt(); }
		if( argwords > 0 ) { emit( f_ldl, 0 ); pushInt(); }
		break;
	
	case 1:		
	/* only arg 1 would go directly load arg2, then eval arg 1 */
		if( argwords > 1 ) { emit( f_ldl, 0 ); pushInt(); }
		cg_expr(arg[0].expr);
                break; 

	case 2:
	/* both will load directly, use cg_binary to get it right */
		cg_binary(s_nothing, arg[1].expr, arg[0].expr, 0, INTREG );
		break;

	}

/* Following line to fix bug to stop opd.value being patched if deadcode */
	dcode = deadcode;
	emit( p_fnstack, -(argwords==0?0:argwords==1?1-argregs:2-argregs) );
	t1 = curcode;

	if( savemax < maxssp ) savemax = maxssp;

	if(debugging(DEBUG_CG))
		trace("cg_fnargs0 argwords %d maxssp %d",argwords,maxssp);

/* Now calculate how much space is needed to make the call.		*/
/* The following calculations have been arrived at after lots of tries	*/
/* fool with this at your peril.					*/
/* NB that dump_info is #pragma -i (for info).				*/
	{
		int unused = savemax-maxssp;
		int fiddle = deadcode ? 0 : - t1->opd.value;
		int maxcallmargin = dump_info ? big_callmargin : def_callmargin;

    		if(debugging(DEBUG_CG)) 
			trace("cg_fnargs1 unused %d fiddle %d savemax %d",
				unused,fiddle,savemax);

		if( unused < maxcallmargin && argwords > 2 )
		{
			int sspextra;
			if( unused < 0 ) unused = 0;
			sspextra = min(argwords-2,maxcallmargin)-unused;
			savemax += max(0,sspextra);
			unused = savemax-maxssp;
		}

		fndrop = max(fiddle,argwords - unused - argregs);
		if( fndrop < 0 ) fndrop = 0;

		fnraise = fndrop - fiddle;
		if( fnraise < 0 ) fnraise = 0;

		if(debugging(DEBUG_CG)) 
			trace("cg_fnargs2 unused %d savemax %d fndrop %d fnraise %d",
				unused,savemax,fndrop,fnraise);

/* Following line to fix bug to stop opd.value being patched if deadcode */
	if(!dcode)
		t->opd.value = fndrop;
	}

	ssp = savessp;
	maxssp = savemax;

	return fnraise;
}
/*}}}*/
/*{{{  gentempvar */
Binder *gentempvar(t,rep)
TypeExpr *t;
int rep;
{
	Binder *b = gentempbinder(t);
	set_VLocal(b, rep>>24, rep&0x00ffffff, 0);
	b->bindmcrep=rep;
	return b;
}
/*}}}*/
/*{{{  contains_fnap */
bool contains_fnap(x)
Expr *x;
{
	switch(h0_(x))
	{
case s_integer:
case s_floatcon:
case s_binder:
case s_string:
	return FALSE;
case s_fnap:
        return TRUE;
case s_cond:
	return contains_fnap(arg1_(x)) ||
		contains_fnap(arg2_(x)) ||
		contains_fnap(arg3_(x));
case s_cast:
	if( !floatingHardware )
	{
		int mcmode = mcrepofexpr(x);
		int argmode = mcrepofexpr(arg1_(x));

		if ( (mcmode>>24)==2 || (argmode>>24)==2) return TRUE;
	}
case s_dot:
case s_content:
case s_bitnot:
case s_boolnot:
case s_monplus:
case s_addrof:
	        return contains_fnap(arg1_(x));
case s_neg:
	if( !floatingHardware ) 
	{
		int mcmode = mcrepofexpr(x);
		if( (mcmode>>24)==2 ) return TRUE;
	}
        return contains_fnap(arg1_(x));
case s_div:
case s_plus:
case s_times:
case s_minus:
case s_equalequal:
case s_notequal:
case s_greater:
case s_greaterequal:
case s_less:
case s_lessequal:
	if( !floatingHardware )
	{
		int mcmode = mcrepofexpr(x);
		if( (mcmode>>24)==2 ) return TRUE;
	}
case s_displace:
case s_assign:
case s_comma:
case s_leftshift:
case s_rightshift:
case s_rem:
case s_andand:
case s_oror:
case s_and:
case s_xor:
case s_or:
		return contains_fnap(arg1_(x)) || 
                	contains_fnap(arg2_(x)) ;
case s_let:
       	 	return contains_fnap(arg2_(x));
default:
		return FALSE;
	}
}
/*}}}*/
/*{{{  structure_function_value */

bool structure_function_value(x)
Expr *x;
{
/* x is an expression (known to be of a structure type). Return true if  */
/* the expression is the result of evaluating a structure-yielding       */
/* function. This information is needed in the case of selections such   */
/* as     f().field     etc.                                             */
    switch (h0_(x))
    {
case s_comma:
        return structure_function_value(arg2_(x));
case s_dot:
        return structure_function_value(arg1_(x));
case s_fnap:
        return TRUE;
case s_cond:
        return structure_function_value(arg2_(x)) ||
               structure_function_value(arg3_(x));
default:
        return FALSE;
    }
}
/*}}}*/
/*{{{  cg_addr */

void cg_addr(sv,valneeded)
Expr *sv; int valneeded;
{
    Expr *e;
    for (;;) switch (h0_(sv))
    {
case s_dot:	/*	&(x.y)	     can occur during fpsim code      */
	sv = cg_content_for_dot(sv);

case s_content: /* 	&(*x) -> x   can occur when doing float casts */
	cg_addrexpr(arg1_(sv));
	return;

case s_comma:   /* can occur in structure manipulation code */
        cg_exprvoid(arg1_(sv));
        sv = arg2_(sv);
        continue;

case s_cast:
        /*  & ((cast)x)    --->   & x               */
        sv = arg1_(sv);
        continue;

case s_cond:
        /*  & (p ? x : y)  --->   p ? (&x) : (&y)   */
        arg2_(sv) = take_address(arg2_(sv));
        arg3_(sv) = take_address(arg3_(sv));
        type_(sv) = typeofexpr(arg2_(sv));
        cg_expr1(sv, valneeded); return;

case s_assign:
/* this can happen with structure assignments - it is essentially an     */
/* artefact of transformations made here in the codegenerator but seems  */
/* fairly convenient.                                                    */
/* In particular this is used in the case of                             */
/*      (a = b) . c                                                      */
/* which in effect turns into                                            */
/*      ((a = b), a . c)                                                 */
/* but with need for special care if a is an expression that might have  */
/* side effects when being evaluated.                                    */
        e = arg1_(sv);
ell:    switch (h0_(e))
        {
    default:
            syserr("p nasty in '&(p=q)'");
    case s_dot:
/*  &((p.q) =r )   =>   &(*(&p+q') = r)                                  */
            e = cg_content_for_dot(e);
	    pp_expr(e,1);
            goto ell;
    case s_content:
           {    TypeExpr *t = typeofexpr(arg1_(e));
                Binder *gen = gentempbinder(t);
/*     & (*p=q)                                                          */
/*                turns into                                             */
/*     (let g;  g = p,  *g = q,  g)                                      */
/*  where the temp var is to ensure that p gets evaluated just once.     */
                sv = mk_expr2(s_let,
/* @@@ The semantic analyser should export such a function ... */
                              t,
                              (Expr *)mkBindList(0, gen),
                              mk_expr2(s_comma,
                                       t,
                                       mk_expr2(s_assign,
                                                t,
                                                (Expr *)gen,
                                                arg1_(e)),
                                       mk_expr2(s_comma,
                                                t,
                                                mk_expr2(s_assign,
                                                         typeofexpr(arg2_(sv)),
                                                         mk_expr1(s_content,
                                                                  t,
                                                                  (Expr *)gen),
                                                arg2_(sv)),
                                       (Expr *)gen)));
                if (debugging(DEBUG_CG))
                {   eprintf("& p = q  ---> ");
                    pr_expr(sv);
                    eprintf("\n");
                }
            }
	    pp_expr(sv, valneeded);
            cg_addrexpr(sv); return;
    case s_binder:
            cg_exprvoid(sv);  /* get the assignment done, voiding the result */
            sv = e;           /* then take address of the lhs variable       */
        }
        /* drop through */
case s_binder:
      { Binder *b = (Binder *)sv;
        switch (bindstg_(b) & PRINCSTGBITS)
        {
    default:
            syserr("Funny storage class %d\n", bindstg_(b));
    case bitofstg_(s_auto):
            if (valneeded)
            {   emit( p_ldvlp, bindxx_(b));
		pushInt();
                return;
            }
            else return;

    case bitofstg_(s_static):
	    if( b->bindstg & b_fnconst )
	    {
		emit( p_ldpf, b );
		pushInt();
		return;
	    }
            if (valneeded) emit( p_ldsp, b );
	    pushInt();
            return;

    case bitofstg_(s_extern):
	    if( b->bindstg & b_fnconst )
	    {
	    	/* There is a serious problem here in that the compiler	*/
	    	/* cannot distinguish between locally defined external	*/
	    	/* functions and real external functions.		*/
	    	/* When compiling resident modules, always go through	*/
	    	/* a stub.						*/
#if 0
	    	if( !libfile && !nomodule ) emit( p_ldx, b );
#else
		/* Tony 23/1/95 */
		if (!library_module) emit (p_ldx, b);
#endif	
	    	else 
	    	{
		/* for an external procedure, start off by loading a pointer */
		/* to a stub for it, if the procedure turns out to be local  */
		/* the stub will be disabled.				     */
			emit(p_ldpf, b );
			genstub( 1, bindsym_(b), 1 );
		}
	    }
            else if (valneeded) emit( p_ldxp, b );
	    pushInt();
            return;
        }
      }
#ifdef SOFTWARE_FLOATING_POINT
case s_floatcon:
        if (!valneeded) return GAP;
        emitfloat((((FloatCon *)sv)->floatlen) & bitoftype_(s_double)
              ? J_ADCOND : J_ADCONF, r = bgetregister(), GAP, (FloatCon *)sv);
        return r;
#endif
default:
        syserr("cg_addr(%d)", h0_(sv));
    }
}
/*}}}*/
/*{{{  cg_loadconst */

void cg_loadconst(n,e)
int n; Expr *e;
{   /* void e if !=NULL and return n - used for things like f()*0     */
    if (e) cg_exprvoid(e);
    emit( f_ldc, n);
    pushInt();
    return;
}
/*}}}*/
/*{{{  structure_assign */

void structure_assign(lhs,rhs,length)
Expr *lhs; Expr *rhs; int length;
{
    Expr *e;
/* In a void context I turn a structure assignment into a call to the    */
/* library function memcpy().                                            */
/* Note that casts between structure types are not valid (and so will    */
/* have been filtered out earlier), but casts to the same structure type */
/* can be present (particularly around (a ? b : c) expressions) as an    */
/* artefact of the behaviour of sem. Prune them off here.                */
    while (h0_(rhs) == s_cast) rhs = arg1_(rhs);
    switch (h0_(rhs))
    {
case s_comma:
        cg_exprvoid(arg1_(rhs));
        structure_assign(lhs, arg2_(rhs), length);
        return;
case s_assign:
/* A chain of assignments as in    p = q = r   get mapped as follows:    */
/*    (  LET struct *g,                                                  */
/*       g = &q,                                                         */
/*       *g = r,                                                         */
/*       p = *g )                                                        */
/*                 thus &q gets evaluated only once                      */
        {   TypeExpr *t = typeofexpr(rhs);
            Binder *gen = gentempbinder(ptrtotype_(t));
            e = mk_expr2(s_let,
                         t,
                         (Expr *)mkBindList(0, gen),
                         mk_expr2(s_comma,
                                  t,
                                  mk_expr2(s_assign,
                                           ptrtotype_(t),
                                           (Expr *)gen,
                                           take_address(arg1_(rhs))),
                                  mk_expr2(s_comma,
                                           t,
                                           mk_expr2(s_assign,
                                                    t,
                                                    mk_expr1(s_content,
                                                             t,
                                                             (Expr *)gen),
                                                    arg2_(rhs)),
                                           mk_expr2(s_assign,
                                                    t,
                                                    lhs,
                                                    mk_expr1(s_content,
                                                             t,
                                                             (Expr *)gen)))));
		pp_expr(e,1);
        }
        break;
case s_fnap:
        e = mk_expr2(s_fnap, te_void, arg1_(rhs),
                  (Expr *)mkExprList(exprfnargs_(rhs), take_address(lhs)));
	pp_expr(e,1);
        break;
case s_cond:
/* Convert    a = (b ? c : d)                                            */
/*    (LET struct *g,                                                    */
/*       g = &a,                                                         */
/*       b ? (*g = c) : (*g = d))                                        */
/*                                                                       */
        {   TypeExpr *t = typeofexpr(lhs);
            Binder *gen = gentempbinder(ptrtotype_(t));
            e = mk_expr2(s_let,
                         t,
                         (Expr *)mkBindList(0, gen),
                         mk_expr2(s_comma,
                                  t,
                                  mk_expr2(s_assign,
                                           ptrtotype_(t),
                                           (Expr *)gen,
                                           take_address(lhs)),
                                  mk_expr3(s_cond,
                                           t,
                                           arg1_(rhs),
                                           mk_expr2(s_assign,
                                                    t,
                                                    mk_expr1(s_content,
                                                             t,
                                                             (Expr *)gen),
                                                    arg2_(rhs)),
                                           mk_expr2(s_assign,
                                                    t,
                                                    mk_expr1(s_content,
                                                             t,
                                                             (Expr *)gen),
                                                    arg3_(rhs)))));
		pp_expr(e,1);
        }
        break;
default:
        e = mk_expr2(s_fnap, primtype_(bitoftype_(s_void)), sim.memcpyfn,
                (Expr *)mkExprList(mkExprList(mkExprList(0,
                    mkintconst(te_int,length,0)),
                    take_address(rhs)),
                    take_address(lhs)));
	pp_expr(e,1);
        break;
    }
    if (debugging(DEBUG_CG))
    {   eprintf("Structure assignment: ");
        pr_expr(e);
        eprintf("\n");
    }
    cg_exprvoid(e);
}
/*}}}*/
/*{{{  load_integer_structure */

void load_integer_structure(e)
Expr *e;
{
/* e is a structure-valued expression, but one where the value is a      */
/* one-word integer-like quantity. Must behave like cg_expr would but    */
/* with special treatment for function calls.                            */
    switch (h0_(e))
    {
case s_comma:
        cg_exprvoid(arg1_(e));
        load_integer_structure(arg2_(e));
        return;
case s_assign:
        load_integer_structure(arg2_(e));
        cg_storein(arg1_(e), s_assign, 1);
        return;
case s_fnap:
        {   TypeExpr *t = type_(e);
            Binder *gen = gentempbinder(ptrtotype_(t));
            bindstg_(gen) |= b_addrof;
            e = mk_expr2(s_fnap, te_void, arg1_(e),
                         (Expr *)mkExprList(exprfnargs_(e), take_address((Expr *)gen)));
            e = mk_expr2(s_let, t, (Expr *)mkBindList(0, gen),
                    mk_expr2(s_comma, t, e, (Expr *)gen));
	    pp_expr(e,1);
            cg_expr(e);
            return;
        }
/* Since casts between structure-types are illegal the only sort of cast   */
/* that can be present here is just one re-asserting the type of the       */
/* expression being loaded. Hence I skip over the cast. This certainly     */
/* happens with a structure version of (a = b ? c : d) where the type gets */
/* inserted to give (a = (structure)(b ? c : d)).                          */
case s_cast:
            load_integer_structure(arg1_(e));
            return;
case s_cond:
        {   LabelNumber *l3 = nextlabel();
            cg_cond(e, TRUE, l3, TRUE, TRUE);
            start_new_basic_block(l3);
	    emitcjfix(); popInt();
            return;
        }

default:
        cg_expr(e);
        return;
    }
}
/*}}}*/
/*{{{  cg_cond1 */
static void cg_cond1(e,valneeded,l3,structload)
Expr *e; int valneeded; LabelNumber *l3; int structload;
{   while (h0_(e)==s_comma)
    {   cg_exprvoid(arg1_(e));
        e = arg2_(e);
    }
    if (h0_(e)==s_cond) cg_cond(e,valneeded, l3, structload, FALSE);
    else if (structload)
    {   
        load_integer_structure(e);
    } 
    else cg_expr1(e, valneeded);
}
/*}}}*/
/*{{{  cg_cond */
void cg_cond(x,valneeded,l3,structload,top)
Expr *x; 
int valneeded; LabelNumber *l3;
bool structload, top;
{
#if 1
    LabelNumber *l1 = nextlabel();
    int saveida = ida, savefda = fda;

    push_trace("cg_cond");

    if(debugging(DEBUG_CG)) 
	trace("valneeded %d structload %d top %d",valneeded,structload,top);

    cg_test(arg1_(x), FALSE, l1);

    cg_cond1(arg2_(x), valneeded, l3, structload);

    if( valneeded )
    {
	    emit(p_j, l3);
	    popInt();
    }
    else
    {
	emit( f_j, l3 );
    }
    start_new_basic_block(l1);
    setInt(saveida);
    setFloat(savefda);
    if (valneeded && saveida < 3) emitcjfix();

    cg_cond1(arg3_(x), valneeded, l3, structload);

/* This is because the caller is going to do a rev ... */
/* This could be improved when we're leaving the result in FA */
    if (valneeded && top)
    {
       emit( f_ldc, 0 );
       pushInt(); 
    }

    pop_trace();
    return;

#else

    LabelNumber *l1 = nextlabel();
    int saveida = ida, savefda = fda;

    cg_test(arg1_(x), FALSE, l1);

    cg_cond1(arg2_(x), valneeded, l3, structload);

    pushInt();
    emit(p_j, l3);
    popInt();
    start_new_basic_block(l1);
    setInt(saveida);
    setFloat(savefda);
    if (saveida < 3) emitcjfix();

    cg_cond1(arg3_(x), valneeded, l3, structload);

/* This is because the caller is going to do a rev ... */
/* This could be improved when we're leaving the result in FA */
    if (top)
    {
       emit( f_ldc, 0 );
       pushInt(); 
    }

    return;
#endif
}
/*}}}*/
/*{{{  open_compilable */
bool open_compilable(x,mcmode,mclength)
Expr *x; int mcmode; int mclength;
{
    Expr *fname = arg1_(x);
    Expr *a1 = NULL, *a2 = NULL, *a3 = NULL;
    int narg = 0, n;
    ExprList *a = exprfnargs_(x);
    if (h0_(fname) != s_addrof) return 0;
    fname = arg1_(fname);
    if (fname == NULL || h0_(fname) != s_binder) return 0;
/* Only consider this call if the function is a direct function name */
    if (a != NULL)
    {   a1 = exprcar_(a);
        narg++;
        a = cdr_(a);
        if (a != NULL)
        {   a2 = exprcar_(a);
            narg++;
            a = cdr_(a);
            if (a != NULL)
            {   a3 = exprcar_(a);
                narg++;
                a = cdr_(a);
            }
        }
    }
/* At present the only thing that I do open compilation for is _memcpy() */
/* and then only if it has just 3 args, the last of which is an integer  */
/* with value >= 0 .		 	                                 */
    if (bindsym_((Binder *)fname) == bindsym_((Binder *)arg1_(sim.memcpyfn)) &&
        a == NULL && narg == 3 && h0_(a3) == s_integer &&
        (n = intval_(a3)) >= 0 )
    {   
	/* we know at this point that the stack is empty, since this is	*/
	/* theoretically a function call				*/
	cg_binary( s_nothing, a2, a1, 0, INTREG);
	emit( f_ldc, n );
	pushInt();
	emit(f_opr, op_move );
	setInt(FullDepth); setFloat(FullDepth);
	return TRUE;
    }
/* Also try for _operate(opnd,...) which allows transputer opr		*/
/* instructions to be compiled in-line. Primarily added for the kernel	*/
/* these may also be useful for other users.				*/
#ifdef NEW_OPERATE
	if( ((bindsym_((Binder *)fname) == bindsym_(builtin._operate_word)) ||
	    (bindsym_((Binder *)fname) == bindsym_(builtin._operate_void))) &&
#else
	if(   bindsym_((Binder *)fname) == bindsym_(builtin._operate)
#endif
	   && narg > 0 && h0_(a1) == s_integer )
	{	Expr *a4 = NULL;
		if( a != NULL )
		{
			a4 = exprcar_(a);
			narg++;
			a = cdr_(a);
		}
		n = intval_(a1);		/* operator */

		if( a4 != NULL )		/* 3 operands		*/
		{
			int d = idepth(depth(a2));
			if( d > 1 )		/* opnd1 need > 1 reg	*/
			{
				VLocal *v;
				cg_expr(a2);
				v = pushtemp(INTREG);
				cg_binary(s_nothing, a4, a3, 0, INTREG );
				poptemp( v, INTREG );
			}
			else			/* opnd 1 need only 1 reg */
			{
				cg_binary(s_nothing, a4, a3, 0, INTREG );
				cg_expr(a2);
			}
		}
		else if( a3 != NULL )		/* two operands		*/
		{
			cg_binary(s_nothing, a3, a2, 0, INTREG );
		}
		else if( a2 != NULL )		/* only 1 operand 	*/
		{
			cg_expr(a2);
		}
		/* else no operands */
		
		emit( f_opr, n );

#ifdef NEW_OPERATE
		if((bindsym_((Binder *)fname) == bindsym_(builtin._operate_word)))
#else
		if((bindsym_((Binder *)fname) == bindsym_(builtin._operate)))
#endif
			setInt(FullDepth-1); 	/* allow for a result		*/
		else setInt(FullDepth);

		setFloat(FullDepth);
		return TRUE;
	}

/* Now try for _direct( func, arg ) which allows certain, restricted	*/
/* direct transputer instructions to be executed.			*/
	if( (bindsym_((Binder *)fname) == bindsym_(builtin._direct)) &&
		narg == 2 && h0_(a1) == s_integer )
	{
		Xop op;
		n = intval_(a1);
		if( debugging(DEBUG_CG) )
			trace("_direct %x %s arg %d %x",
				n,fName(n),h0_(a2),bindstg_((Binder *)a2));
			
		switch( n )
		{
		case f_stl: 	op = p_stvl;             goto localop;
		case f_ldlp:	op = p_ldvlp; pushInt(); goto localop;
		case f_ldl:	op = p_ldvl;  pushInt();
		localop:
			if( h0_(a2) == s_integer )
			{
				emit( n, intval_(a2) );
				return TRUE;
			}
			if( h0_(a2) != s_binder || ( h0_(a2) == s_binder &&
			   (bindstg_((Binder *)a2) & bitofstg_(s_auto)) == 0 ) )
			{
			argerror:
				cc_err("invalid argument given to _direct()");
				return TRUE;
			}
			emit( op, bindxx_((Binder *)a2) );
			return TRUE;
			
		case f_stnl:
			if( h0_(a2) != s_integer ) goto argerror;
			emit( n, intval_(a2) );
			return TRUE;
		case f_ldnl:
		case f_ldnlp:
		case f_ldc:
			if( h0_(a2) != s_integer ) goto argerror;
			pushInt();
		case f_eqc:
		case f_adc:
			emit( n, intval_(a2) );
			return TRUE;

		case f_ajw:
			if( h0_(a2) != s_integer ) goto argerror;
			emit( p_fnstack, - intval_(a2) );
			return TRUE;
						
		case f_j:
		case f_call:
		case f_cj:
		default:
			cc_err("invalid function code given to _direct()");
			return TRUE;
		}
	}
/* _setpri(pri) generates a special piece of code to alter the priority	*/
/* of the current process. This cannot be constructed out of _operate	*/
/* and _direct calls because of the need to supply a label as a parameter*/
	if( (bindsym_((Binder *)fname) == bindsym_(builtin._setpri)) &&
		narg == 1 )
	{
		LabelNumber *l;
		int pri;

		if( !integer_constant(a1) || result2 < 0 || result2 > 1 ) 
			cc_err("Operand of _setpri must be constant 0 or 1");

		pri = result2;
		l = nextlabel();
		emit( p_ldpi, l );
		emit( f_stl, -1 );
		cg_expr( a1 );
		emit( f_ldlp, 0 );
		emit( f_opr, op_or );
		emit( f_opr, op_runp );
		emit( f_opr, op_stopp );
                start_new_basic_block(l);
                if( pri == 0 )
                {	
                	/* when switching from low to high priority,	*/
                	/* allow the low pri process to stop.		*/
                	emit( f_opr, op_ldtimer );
                	emit( f_adc, 3 );
                	emit( f_opr, op_tin );
                }
		return TRUE;
	}
/* now see if we can get hold of the calling stack frame		*/
	if( (bindsym_((Binder *)fname) == bindsym_(builtin._stackframe)) &&
		narg == 0)
	{
		emit( p_stackframe, 0 );
		setInt(FullDepth-1); 	/* allow for a result		*/
		setFloat(FullDepth);
		return TRUE;
	}
/* The next line is UTTERLY artificial & is just there so that the 2     */
/* vars concerned get referenced and so the compiler does not mutter.    */
    if (mcmode == mclength) return FALSE;
/* end of fudge.                                                         */
    return FALSE;
}
/*}}}*/
/*{{{  dense_case_table */
int dense_case_table(low_value,high_value,ncases)
int low_value; int high_value; int ncases;
{
/* This function provides a criterion for selection of a test-and-branch */
/* or a jump-table implementation of a case statement. Note that it is   */
/* necessary to be a little careful about arithmetic overflow in this    */
/* code, and that the constants here will need tuning for the target     */
/* computer's instruction timing characteristics.                        */
#if 0
    int span = high_value/4 - low_value/4;
    if (ncases>5 && span < ncases/2) return TRUE;
    return FALSE;
#else
	int span = high_value/3 - low_value/3;
	if( ncases > 12 && ncases > span ) return TRUE;
	return FALSE;
#endif
}
/*}}}*/
/*{{{  linear_casebranch */
static void linear_casebranch(t,v,ncases,defaultlab)
VLocal *t;
CasePair *v; int ncases;
LabelNumber *defaultlab;
{   /* This code doesn't feel very good, however it works and DOESN'T
       set error. The nicer sequence using adc - delta; dup ; cj
       has the problem that unless the value has been range checked first
       we can cause an overflow in one of the adc's (BADNESS).
    */
    while (ncases-- != 0)
    {   
#ifdef never
	if( floatingHardware ) emit( f_opr, op_dup );
	else 
	{
	   emit( p_ldvl, t );
	}
#else
	emit( p_ldvl, t );
#endif
	pushInt();
#ifdef never
	if( v->caseval != 0 )
	{ /* small optimisation for when case value is zero */
		emit( f_eqc, v->caseval );
		emit( f_eqc, 0 );
	}
#else
	/* note that the following code can cause overflow	*/
	if( v->caseval != 0 )
	{
		emit( f_adc, -v->caseval );
	}
#endif	
	emit( f_cj, v->caselab ); popInt();
	
        v++;
    }
    emit(f_j, defaultlab);
}
/*}}}*/
/*{{{  table_casebranch */

#ifdef never
/* @@@ no jump tables at present */
static void table_casebranch(CasePair *v, int ncases,
                             LabelNumber *defaultlab)
{
    int m = v[0].caseval, n = v[ncases-1].caseval, i;
    int size = n - m + 1;
    LabelNumber **table = BindAlloc((size+1) * sizeof(LabelNumber *));
    VRegnum r1 = bgetregister();
    /* before you optimise the SUBK on the next line for the case m==0 */
    /* remember that 370 & clipper machine generators corrupt r1, and  */
    /* regalloc.c (the proper place) does not know this.               */
    emit(J_SUBK, r1, r, m);
    table[0] = defaultlab;
    for (i=0; i<size; i++)
       table[i+1] = (i+m == v->caseval) ? (v++)->caselab : defaultlab;
    /* It is important that literals are not generated so as to break up */
    /* the branch table that follows - J_CASEBRANCH requests this.       */
    /* Type check kluge in the next line...                              */
    emit(J_CASEBRANCH, r1, (VRegnum)table, size + 1);
    bfreeregister(r1);
}
#endif

void table_casebranch(t,v,ncases,defaultlab)
VLocal *t;
CasePair *v; int ncases;
LabelNumber *defaultlab;
{
	int m = v[0].caseval, n = v[ncases-1].caseval, i;
	int size = n - m + 1;
	LabelNumber *l = nextlabel();
	LabelNumber **table = BindAlloc(size * sizeof(LabelNumber *));

	for (i=0; i<size; i++)
	{
		table[i] = (i+m == v->caseval) ? (v++)->caselab : defaultlab;
		table[i]->refs++;
	}

	emit( p_ldvl, t );
	emit( f_ldc, m-1 );
	emit( f_opr, op_gt );
	emit( f_cj, defaultlab );	/* default if < m 	*/

	emit( f_ldc, n+1 );
	emit( p_ldvl, t );
	emit( f_opr, op_gt );
	emit( f_cj, defaultlab );	/* default if > n 	*/
	
	emit( p_ldvl, t );
	emit( f_adc, -m );		/* scale down to range	*/
	emit( f_ldc, 4 );
	emit( f_opr, op_prod );		/* form table offset	*/
	
	emit( p_ldpi, l );
	emit( f_opr, op_sum );		/* add table address	*/
	
	emit( f_opr, op_gcall );	/* jump into table	*/
	
	emitcasebranch(table,size,l);	 /* generate table	*/
}
/*}}}*/
/*{{{  casebranch */

void casebranch(t,v,ncases,defaultlab)
VLocal *t;
CasePair *v; int ncases;
LabelNumber *defaultlab;
{
    if (ncases<4) linear_casebranch(t,v, ncases, defaultlab);
    else if (dense_case_table(v[0].caseval, v[ncases-1].caseval,ncases))
             table_casebranch(t, v, ncases, defaultlab);
    else
    {
	int mid = ncases/2;
        LabelNumber *l1 = nextlabel();
#ifdef never
	if( floatingHardware ) emit(f_opr,op_dup);
	else 
	{
	   emit( p_ldvl, t );
	}
#else
	emit( p_ldvl, t );
#endif
	pushInt();
	emit( f_ldc, v[mid].caseval ); pushInt();
	emit(f_opr, op_gt ); popInt();
	emit( f_cj, l1 ); popInt();
        casebranch(t, &v[mid+1], ncases-mid-1, defaultlab);/* upper half */
        start_new_basic_block(l1);
	emitcjfix();				/* pop zero left by cj */
        casebranch(t, v, mid+1, defaultlab);		/* lower half */
    }
}
/*}}}*/
/*{{{  cg_case_or_default */

void cg_case_or_default(l1)
LabelNumber *l1;
{
        start_new_basic_block(l1);
	cg_count();
}

/*}}}*/
/*{{{  cg_count */

void cg_count()
{
}
/*}}}*/

/* End of section cg1.c */
