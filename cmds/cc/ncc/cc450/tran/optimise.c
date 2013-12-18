/* $Id: optimise.c,v 1.1 1995/05/19 11:38:17 nickc Exp $ */
#ifdef __old
#include "cchdr.h"
#include "util.h"
#include "xpuops.h"
#include "xrefs.h"
#include "cg.h"
#include "AEops.h"
#else
#include "globals.h"
#include "util.h"
#include "xpuops.h"
#include "xrefs.h"
#include "cg.h"
#include "aeops.h"
#endif

extern Block *topblock;
LabelNumber *branch_chain();

void optimise()
{
	if( branch_opt ) (void) branch_chain(topblock->lab);
	else
	{	/* mark all blocks as alive	*/
		Block *b = topblock;
		while( b != NULL ) 
		{
			b->flags |= BlkFlg_alive;
			b = b->next;
		}
	}
}

/* Note: the following routine is VERY recursive, it should be 	*/
/* re-written to use pointer reversal.				*/
LabelNumber *branch_chain(lab)
LabelNumber *lab;
{
	Block *b = lab->block;
	bool empty = block_empty(b);
	bool done = b->flags & BlkFlg_done;
	LabelNumber *s1, *s2;
	
	if( b->flags & BlkFlg_alive )
	{
		if(debugging(DEBUG_CG))
			trace("branch_chain block %d alive",lab->name);
		return b->lab;
	}

	if( b->flags & BlkFlg_visit )
	{
		if(debugging(DEBUG_CG))
			trace("branch_chain block %d visited",lab->name);
		
		/* if we have looped back to an empty block, mark it as	*/
		/* alive and go on down. 				*/
		if( empty )
		{
			b->flags |= BlkFlg_alive;
			lab = branch_chain(b->succ1);
			b->flags &= ~BlkFlg_alive;
			return lab;
		}
		return b->lab;
	}
	
	s1 = b->succ1;
	s2 = b->succ2;
	
	switch( b->jump )
	{
	case f_j:
	case p_j:
		if(debugging(DEBUG_CG))
			trace("branch_chain block %d %s j %d %s",
				lab->name,empty?"empty":"inuse",b->succ1->name,
				done?"done":"");
		if( done ) 
		{ if( empty ) return b->succ1; else return lab; }
		if( !empty ) b->flags |= BlkFlg_alive;				
		/* we have to worry here about things like for(;;);	*/
		/* which causes a circular chain of empty blocks	*/
		/* stop looping by setting the visit bit.		*/
		b->flags |= BlkFlg_visit;
		b->succ1=branch_chain(s1);
		b->flags &= ~BlkFlg_visit;
		/* if the successor has changed, increment his ref count*/
		if( b->succ1 != s1 ) b->succ1->refs++;
	
		if( empty )
		{
			/* an empty block is never ultimately alive	*/
			/* unmark it and return its successor.		*/
			/* BUT if we have an empty loop, mark it alive  */
			if( b->succ1 == lab ) b->flags |= BlkFlg_alive;
			b->flags |= BlkFlg_done;
			if(debugging(DEBUG_CG))
				trace("replace %d with %d",lab->name,b->succ1->name);
			return b->succ1;
		}

		b->flags |= BlkFlg_done;
		
		if(debugging(DEBUG_CG)) trace("returning %d",lab->name);
		return lab;

	case f_cj:
		if(debugging(DEBUG_CG))
			trace("branch_chain block %d cj %d j %d %s",
				lab->name,b->succ2->name,b->succ1->name,
				done?"done":"");
		if( done ) return lab;
		b->flags |= BlkFlg_alive|BlkFlg_visit;
		b->succ1=branch_chain(s1);
		if( b->succ1 != s1 ) b->succ1->refs++;
		b->succ2=branch_chain(s2);
		if( b->succ2 != s2 ) b->succ2->refs++;
		b->flags &= ~BlkFlg_visit;
		b->flags |= BlkFlg_done;
		if(debugging(DEBUG_CG)) trace("returning %d",lab->name);
		return lab;
		
	case p_case:
	{
		int i;
		if(debugging(DEBUG_CG))
			trace("branch_chain case table %d",lab->name,done?"done":"");
		if( done ) return lab;
		b->flags |= BlkFlg_alive|BlkFlg_visit;
		for( i = 0; i < b->operand2.tabsize; i++ )
		{
			if(debugging(DEBUG_CG))
				trace("branch_chain case %d -> %d",i,b->operand1.table[i]->name);
			b->operand1.table[i] = branch_chain(b->operand1.table[i]);
		}
		b->flags &= ~BlkFlg_visit;
		b->flags |= BlkFlg_done;
		return lab;
	}

	case p_noop:
		b->flags |= BlkFlg_alive;
		b->flags |= BlkFlg_done;
		return b->lab;
		
	default:
		syserr("Unknown branch type in branch_chain: %x",b->jump);
	}
	/* should never get here, but to keep compiler happy ... */
	return NULL;
}

/* A block with no code, or just noop or infolines is empty.	*/
/* A noop2 is like noop but counts as a real instruction.	*/
bool block_empty(b)
Block *b;
{
	Tcode *t = b->code;
	
	while( t != NULL )
	{
		switch( t->op )
		{
		default: 
			return FALSE;
		case p_noop:
		case p_infoline:
			t = t->next;
		}
	}
	return TRUE;
}
