//---------------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include "RegExpClass.h"
//---------------------------------------------------------------------------

/************************************************\
*                                                *
*   REGEXP.DDL Implementation Module             *
*   Copyright (c) 1992 by Borland International  *
*   Copyright (c) 1986 by Univerisity of Toronto *
*                                                *
\************************************************/

/* This file was originally written by Henry Spencer for
 * the University of Toronto and was modified by
 * Borland International to compile with Borland C++ 3.1
 * and for use in a DLL.
 */
/*
 * The "internal use only" fields in regexp.h are present to pass info from
 * compile to execute that permits the execute phase to run lots faster on
 * simple cases.  They are:
 *
 * regstart     char that must begin a match; '\0' if none obvious
 * reganch      is the match anchored (at beginning-of-line only)?
 * regmust      string (pointer into program) that match must include, or NULL
 * regmlen      length of regmust string
 *
 * Regstart and reganch permit very fast decisions on suitable starting points
 * for a match, cutting down the work a lot.  Regmust permits fast rejection
 * of lines that cannot possibly match.  The regmust tests are costly enough
 * that regcomp() supplies a regmust only if the r.e. contains something
 * potentially expensive (at present, the only such thing detected is * or +
 * at the start of the r.e., which can involve a lot of backup).  Regmlen is
 * supplied because the test in regexec() needs it and regcomp() is computing
 * it anyway.
 */


/*
 * Structure for regexp "program".  This is essentially a linear encoding
 * of a nondeterministic finite-state machine (aka syntax charts or
 * "railroad normal form" in parsing technology).  Each node is an opcode
 * plus a "next" pointer, possibly plus an operand.  "Next" pointers of
 * all nodes except BRANCH implement concatenation; a "next" pointer with
 * a BRANCH on both ends of it is connecting two alternatives.  (Here we
 * have one of the subtle syntax dependencies:  an individual BRANCH (as
 * opposed to a collection of them) is never concatenated with anything
 * because of operator precedence.)  The operand of some types of node is
 * a literal string; for others, it is a node leading into a sub-FSM.  In
 * particular, the operand of a BRANCH node is the first node of the branch.
 * (NB this is *not* a tree structure:  the tail of the branch connects
 * to the thing following the set of BRANCHes.)  The opcodes are:
 */

/* definition   number  opnd?   meaning */
#define END     0       /* no   End of program. */
#define BOL     1       /* no   Match "" at beginning of line. */
#define EOL     2       /* no   Match "" at end of line. */
#define ANY     3       /* no   Match any one character. */
#define ANYOF   4       /* str  Match any character in this string. */
#define ANYBUT  5       /* str  Match any character not in this string. */
#define BRANCH  6       /* node Match this alternative, or the next... */
#define BACK    7       /* no   Match "", "next" ptr points backward. */
#define EXACTLY 8       /* str  Match this string. */
#define NOTHING 9       /* no   Match empty string. */
#define STAR    10      /* node Match this (simple) thing 0 or more times. */
#define PLUS    11      /* node Match this (simple) thing 1 or more times. */
#define OPEN    20      /* no   Mark this point in input as start of #n. */
                        /*      OPEN+1 is number 1, etc. */
#define CLOSE   70      /* no   Analogous to OPEN. */
/*
 * Opcode notes:
 *
 * BRANCH       The set of branches constituting a single choice are hooked
 *              together with their "next" pointers, since precedence prevents
 *              anything being concatenated to any individual branch.  The
 *              "next" pointer of the last BRANCH in a choice points to the
 *              thing following the whole choice.  This is also where the
 *              final "next" pointer of each individual branch points; each
 *              branch starts with the operand node of a BRANCH node.
 *
 * BACK         Normal "next" pointers all implicitly point forward; BACK
 *              exists to make loop structures possible.
 *
 * STAR,PLUS    '?', and complex '*' and '+', are implemented as circular
 *              BRANCH structures using BACK.  Simple cases (one character
 *              per match) are implemented with STAR and PLUS for speed
 *              and to minimize recursive plunges.
 *
 * OPEN,CLOSE   ...are numbered at compile time.
 */

/*
 * A node is one char of opcode followed by two chars of "next" pointer.
 * "Next" pointers are stored as two 8-bit pieces, high order first.  The
 * value is a positive offset from the opcode of the node containing it.
 * An operand, if any, simply follows the node.  (Note that much of the
 * code generation knows about this implicit relationship.)
 *
 * Using two bytes for the "next" pointer is vast overkill for most things,
 * but allows patterns to get big without disasters.
 */

#define OP(p)           (*(p))
#define NEXT(p)         (((*((p)+1)&0377)<<8) + (*((p)+2)&0377))
#define OPERAND(p)      ((p) + 3)

/*
 * Utility definitions.
 */

#ifndef UCHARAT
#ifndef CHARBITS
#define UCHARAT(p)      ((int)*(unsigned char *)(p))
#else
#define UCHARAT(p)      ((int)*(p)&CHARBITS)
#endif
#endif

#define FAIL(m)         { regerror = m; return(NULL); }
#define ISMULT(c)       ((c) == '*' || (c) == '+' || (c) == '?')
#define META            "^$.[()|?+*\\"

/*
 * Flags to be passed up and down.
 */
#define HASWIDTH        01      /* Known never to match null string. */
#define SIMPLE          02      /* Simple enough to be STAR/PLUS operand. */
#define SPSTART         04      /* Starts with * or +. */
#define WORST           0       /* Worst case. */

/*
 * The first byte of the regexp internal "program" is actually this magic
 * number; the start node begins in the second byte.
 */

#define MAGIC   0234
//---------------------------------------------------------------------------
const char * serrors[] =
{
 "Ok",
 "Not found",
 "Invalid parameter",
 "Expression too big",
 "Out of memory",
 "Too many subexpressions",
 "Unmatch end parents",
 "Invalid repeat",
 "Nested repeat",
 "Invalid range",
 "Unmatch end bracket",
 "Trailing backslash",
 "Internal"
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TRegExp::TRegExp(const char *Exp)
{
 pregexp = NULL;
 regerror = 0;
 pregexp = regcomp(Exp);
}

//---------------------------------------------------------------------------
TRegExp::~TRegExp(void)
{
 if (pregexp) free(pregexp);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool TRegExp::Match(const char *Str)
{
    return regexec(pregexp, Str) ? true : false;
}

//---------------------------------------------------------------------------
int TRegExp::Replace(int no, char *Res, int ResSize)
{
 if (Res) Res[0] = '\0';
 if (pregexp && (pregexp->startp[no]) && (pregexp->endp[no]))
  {
   int size = (int)(pregexp->endp[no] - pregexp->startp[no]);
   if (size < ResSize)
    {
     if (Res) memmove(Res, pregexp->startp[no], size);
     if (Res) Res[size] = '\0';
     return size;
    }
  }
 return 0;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool TRegExp::Replace(const char *Rpls, char *Res)
{
    return regsub(pregexp, Rpls, Res) ? true : false;
}
//---------------------------------------------------------------------------

int TRegExp::Replace(const char *Rpls, char *Res, int ResSize)
{
 return regsub(pregexp, Rpls, Res, ResSize);
}

//---------------------------------------------------------------------------

/*
 - regcomp - compile a regular expression into internal code
 *
 * We can't allocate space until we know how big the compiled form will be,
 * but we can't compile it (and thus know how big it is) until we've got a
 * place to put the code.  So we cheat:  we compile it twice, once with code
 * generation turned off and size counting turned on, and once "for real".
 * This also means that we don't allocate space until we are sure that the
 * thing really will compile successfully, and we never have to move the
 * code and thus invalidate pointers into it.  (Note that it has to be in
 * one piece because free() must be able to free it all.)
 *
 * Beware that the optimization-preparation code in here knows about some
 * of the structure of the compiled regexp.
 */

//---------------------------------------------------------------------------
TRegExp::regexp * TRegExp::regcomp(const char *exp)
{
  regexp *r;
  char *scan;
  char *longest;
  int len;
  int flags;

  if (exp == NULL) FAIL(RE_INVALIDPARAMETER);

  rexp = exp;
  /* First pass: determine size, legality. */
  regparse = exp;
  regnpar = 1;
  regsize = 0L;
  regcode = &regdummy;
  regc(MAGIC);
  if (reg(0, &flags) == NULL) return(NULL);

  /* Small enough for pointer-storage convention? */
  if (regsize >= 32767L)          /* Probably could be 65535L. */
          FAIL(RE_EXPRESSIONTOOBIG);

  /* Allocate space. */
  r = (regexp *)malloc(sizeof(regexp) + (unsigned)regsize);
  if (r == NULL) FAIL(RE_OUTOFMEMORY);

  /* Second pass: emit code. */
  regparse = exp;
  regnpar = 1;
  regcode = r->program;
  regc(MAGIC);
  if (reg(0, &flags) == NULL) return(NULL);

  /* Dig out information for optimizations. */
  r->regstart = '\0';     /* Worst-case defaults. */
  r->reganch = 0;
  r->regmust = NULL;
  r->regmlen = 0;
  scan = r->program+1;                    /* First BRANCH. */
  if (OP(regnext(scan)) == END)          /* Only one top-level choice. */
   {
    scan = OPERAND(scan);

    /* Starting-point info. */
    if (OP(scan) == EXACTLY) r->regstart = *OPERAND(scan);
    else
    if (OP(scan) == BOL) r->reganch++;

    /*
     * If there's something expensive in the r.e., find the
     * longest literal string that must appear and make it the
     * regmust.  Resolve ties in favor of later strings, since
     * the regstart check works with the beginning of the r.e.
     * and avoiding duplication strengthens checking.  Not a
     * strong reason, but sufficient in the absence of others.
     */
    if (flags&SPSTART)
     {
       longest = NULL;
       len = 0;
       for (; scan != NULL; scan = regnext(scan))
        if (OP(scan) == EXACTLY && strlen(OPERAND(scan)) >= (unsigned)len)
         {
          longest = OPERAND(scan);
          len = (int)strlen(OPERAND(scan));
         }
       r->regmust = longest;
       r->regmlen = len;
     }
   }
  return(r);
}

/*
 - reg - regular expression, i.e. main body or parenthesized thing
 *
 * Caller must absorb opening parenthesis.
 *
 * Combining parenthesis handling with the base level of regular expression
 * is a trifle forced, but the need to tie the tails of the branches to what
 * follows makes it hard to avoid.
 */

//---------------------------------------------------------------------------
char * TRegExp::reg(int paren, int *flagp)
{
  char *ret;
  char *br;
  char *ender;
  int parno;
  int flags;

  *flagp = HASWIDTH;      /* Tentatively. */

  /* Make an OPEN node, if parenthesized. */
  if (paren)
   {
    /* if (regnpar >= NSUBEXP) */
    if (regnpar > NSUBEXP)
            FAIL(RE_TOOMANYSUBEXPS);
    parno = regnpar;
    regnpar++;
    ret = regnode((char)(OPEN+parno));
   }
  else ret = NULL;

  /* Pick up the branches, linking them together. */
  br = regbranch(&flags);
  if (br == NULL) return(NULL);
  if (ret != NULL) regtail(ret, br);       /* OPEN -> first. */
  else ret = br;
  if (!(flags&HASWIDTH)) *flagp &= ~HASWIDTH;
  *flagp |= flags&SPSTART;
  while (*regparse == '|')
   {
    regparse++;
    br = regbranch(&flags);
    if (br == NULL) return(NULL);
    regtail(ret, br);       /* BRANCH -> BRANCH. */
    if (!(flags&HASWIDTH)) *flagp &= ~HASWIDTH;
    *flagp |= flags&SPSTART;
   }

  /* Make a closing node, and hook it on the end. */
  ender = regnode((char)((paren) ? CLOSE+parno : END));
  regtail(ret, ender);

  /* Hook the tails of the branches to the closing node. */
  for (br = ret; br != NULL; br = regnext(br))
          regoptail(br, ender);

  /* Check for proper termination. */
  if (paren && *regparse++ != ')')
   {
     FAIL(RE_UNMATCHEDPARENS);
   }
  else
  if (!paren && *regparse != '\0')
   {
    if (*regparse == ')')
     {
      FAIL(RE_UNMATCHEDPARENS);
     }
    else FAIL(RE_INTERNAL);
    /* NOTREACHED */
   }
  return(ret);
}

/*
 - regbranch - one alternative of an | operator
 *
 * Implements the concatenation operator.
 */
//---------------------------------------------------------------------------
char * TRegExp::regbranch(int *flagp)
{
  char *ret;
  char *chain;
  char *latest;
  int flags;

  *flagp = WORST;         /* Tentatively. */

  ret = regnode(BRANCH);
  chain = NULL;
  while (*regparse != '\0' && *regparse != '|' && *regparse != ')')
   {
    latest = regpiece(&flags);
    if (latest == NULL) return(NULL);
    *flagp |= flags&HASWIDTH;
    if (chain == NULL) *flagp |= flags&SPSTART; /* First piece. */
    else regtail(chain, latest);
    chain = latest;
   }
  if (chain == NULL) (void) regnode(NOTHING);   /* Loop ran zero times. */
  return(ret);
}

/*
 - regpiece - something followed by possible [*+?]
 *
 * Note that the branching code sequences used for ? and the general cases
 * of * and + are somewhat optimized:  they use the same NOTHING node as
 * both the endmarker for their branch list and the body of the last branch.
 * It might seem that this node could be dispensed with entirely, but the
 * endmarker role is not redundant.
 */

//---------------------------------------------------------------------------
char* TRegExp::regpiece(int *flagp)
{
  char *ret;
  char op;
  char *next;
  int flags;

  ret = regatom(&flags);
  if (ret == NULL) return(NULL);

  op = *regparse;
  if (!ISMULT(op))
   {
    *flagp = flags;
    return(ret);
   }

  if (!(flags&HASWIDTH) && op != '?') FAIL(RE_INVALIDREPEAT);
  *flagp = (op != '+') ? (WORST|SPSTART) : (WORST|HASWIDTH);

  if (op == '*' && (flags&SIMPLE)) reginsert(STAR, ret);
  else
  if (op == '*')
   {
    /* Emit x* as (x&|), where & means "self". */
    reginsert(BRANCH, ret);                 /* Either x */
    regoptail(ret, regnode(BACK));          /* and loop */
    regoptail(ret, ret);                    /* back */
    regtail(ret, regnode(BRANCH));          /* or */
    regtail(ret, regnode(NOTHING));         /* null. */
   }
  else
  if (op == '+' && (flags&SIMPLE)) reginsert(PLUS, ret);
  else
  if (op == '+')
   {
    /* Emit x+ as x(&|), where & means "self". */
    next = regnode(BRANCH);                 /* Either */
    regtail(ret, next);
    regtail(regnode(BACK), ret);            /* loop back */
    regtail(next, regnode(BRANCH));         /* or */
    regtail(ret, regnode(NOTHING));         /* null. */
   }
  else
  if (op == '?')
   {
    /* Emit x? as (x|) */
    reginsert(BRANCH, ret);                 /* Either x */
    regtail(ret, regnode(BRANCH));          /* or */
    next = regnode(NOTHING);                /* null. */
    regtail(ret, next);
    regoptail(ret, next);
   }
  regparse++;
  if (ISMULT(*regparse)) FAIL(RE_NESTEDREPEAT);

  return(ret);
}


/*
 - regatom - the lowest level
 *
 * Optimization:  gobbles an entire sequence of ordinary characters so that
 * it can turn them into a single node, which is smaller to store and
 * faster to run.  Backslashed characters are exceptions, each becoming a
 * separate node; the code is simpler that way and it's not worth fixing.
 */
//---------------------------------------------------------------------------
char* TRegExp::regatom(int *flagp)
{
  char *ret;
  int flags;

  *flagp = WORST;         /* Tentatively. */

  switch (*regparse++)
   {
    case '^':
            ret = regnode(BOL);
            break;
    case '$':
            ret = regnode(EOL);
            break;
    case '.':
            ret = regnode(ANY);
            *flagp |= HASWIDTH|SIMPLE;
            break;
    case '[':
           {
            int rclass;
            int classend;

            if (*regparse == '^')
             { /* Complement of range. */
               ret = regnode(ANYBUT);
               regparse++;
             }
            else ret = regnode(ANYOF);
            if (*regparse == ']' || *regparse == '-') regc(*regparse++);
            while (*regparse != '\0' && *regparse != ']')
             {
              if (*regparse == '-')
               {
                regparse++;
                if (*regparse == ']' || *regparse == '\0') regc('-');
                else
                 {
                  rclass = UCHARAT(regparse-2)+1;
                  classend = UCHARAT(regparse);
                  if (rclass > classend+1) FAIL(RE_INVALIDRANGE);
                  for (; rclass <= classend; rclass++) regc((char)rclass);
                  regparse++;
                 }
               }
              else regc(*regparse++);
            }
            regc('\0');
            if (*regparse != ']') FAIL(RE_UNMATCHEDBRACKET);
            regparse++;
            *flagp |= HASWIDTH|SIMPLE;
           }
            break;
    case '(':
            ret = reg(1, &flags);
            if (ret == NULL) return(NULL);
            *flagp |= flags&(HASWIDTH|SPSTART);
            break;
    case '\0':
    case '|':
    case ')':
            FAIL(RE_INTERNAL);      /* Supposed to be caught earlier. */
            break;
    case '?':
    case '+':
    case '*':
            FAIL(RE_INVALIDREPEAT);
            break;
    case '\\':
            if (*regparse == '\0') FAIL(RE_TRAILINGBACKSLASH);
            ret = regnode(EXACTLY);
            if (*regparse == 'b') {regc('\x08'); regparse++;}
            else
            if (*regparse == 't') {regc('\x09'); regparse++;}
            else
            if (*regparse == 'n') {regc('\x0a'); regparse++;}
            else
            if (*regparse == 'v') {regc('\x0b'); regparse++;}
            else
            if (*regparse == 'f') {regc('\x0c'); regparse++;}
            else
            if (*regparse == 'r') {regc('\x0d'); regparse++;}
            else
            if (*regparse == 'e') {regc('\x1b'); regparse++;}
            else  regc(*regparse++);
            regc('\0');
            *flagp |= HASWIDTH|SIMPLE;
            break;
    default:
            {
             int len;
             char ender;

             regparse--;
             len = (int)strcspn(regparse, META);
             if (len <= 0) FAIL(RE_INTERNAL);
             ender = *(regparse+len);
             /* Back off clear of ?+* operand. */
             if (len > 1 && ISMULT(ender)) len--;
             *flagp |= HASWIDTH;
             if (len == 1) *flagp |= SIMPLE;
             ret = regnode(EXACTLY);
             while (len > 0)
              {
               regc(*regparse++);
               len--;
              }
             regc('\0');
            }
            break;
   }

  return(ret);
}

/*
 - regnode - emit a node
 */
//---------------------------------------------------------------------------
char* TRegExp::regnode(char op)                  /* Location. */
{
  char *ret;
  char *ptr;

  ret = regcode;
  if (ret == &regdummy)
   {
    regsize += 3;
    return(ret);
   }

  ptr = ret;
  *ptr++ = op;
  *ptr++ = '\0';          /* Null "next" pointer. */
  *ptr++ = '\0';
  regcode = ptr;

  return(ret);
}

/*
 - regc - emit (if appropriate) a byte of code
 */
//---------------------------------------------------------------------------
void TRegExp::regc(unsigned char b)
{
 if (regcode != &regdummy) *regcode++ = b;
 else regsize++;
}

/*
 - reginsert - insert an operator in front of already-emitted operand
 *
 * Means relocating the operand.
 */
//---------------------------------------------------------------------------
void  TRegExp::reginsert(char op, char *opnd)
{
  char *src;
  char *dst;
  char *place;

  if (regcode == &regdummy)
   {
    regsize += 3;
    return;
   }

  src = regcode;
  regcode += 3;
  dst = regcode;
  while (src > opnd) *--dst = *--src;

  place = opnd;           /* Op node, where operand used to be. */
  *place++ = op;
  *place++ = '\0';
  *place++ = '\0';
}

/*
 - regtail - set the next-pointer at the end of a node chain
 */
//---------------------------------------------------------------------------
void  TRegExp::regtail(char *p, char *val)
{
  char *scan;
  char *temp;
  int offset;

  if (p == &regdummy) return;

  /* Find last node. */
  scan = p;
  for (;;)
   {
    temp = regnext(scan);
    if (temp == NULL) break;
    scan = temp;
   }

  if (OP(scan) == BACK) offset = (int)(scan - val);
  else offset = (int)(val - scan);
  *(scan+1) = (char)((offset>>8)&0377);
  *(scan+2) = (char)(offset&0377);
}

/*
 - regoptail - regtail on operand of first argument; nop if operandless
 */
//---------------------------------------------------------------------------
void  TRegExp::regoptail(char *p, char *val)
{
 /* "Operandless" and "op != BRANCH" are synonymous in practice. */
 if (p == NULL || p == &regdummy || OP(p) != BRANCH) return;
 regtail(OPERAND(p), val);
}

/*
 * regexec and friends
 */

/*
 - regexec - match a regexp against a string
 */
//---------------------------------------------------------------------------
int TRegExp::regexec(regexp *prog,  const char *string)
{
  const char *s;

  /* Be paranoid... */
  if (prog == NULL || string == NULL)
   {
    regerror = RE_INVALIDPARAMETER;
    return(0);
   }

  /* Check validity of program. */
  if (UCHARAT(prog->program) != MAGIC)
   {
    regerror = RE_INVALIDPARAMETER;
    return(0);
   }

  /* If there is a "must appear" string, look for it. */
  if (prog->regmust != NULL)
   {
    s = string;
    while ((s = strchr(s, prog->regmust[0])) != NULL)
     {
      if (strncmp(s, prog->regmust, prog->regmlen) == 0) break; /* Found it. */
      s++;
     }
    if (s == NULL) return(0); /* Not present. */
   }

  /* Mark beginning of line for ^ . */
  regbol = string;

  /* Simplest case:  anchored match need be tried only once. */
  if (prog->reganch) return(regtry(prog, string));

  /* Messy cases:  unanchored match. */
  s = string;
  if (prog->regstart != '\0')
   /* We know what char it must start with. */
   while ((s = strchr(s, prog->regstart)) != NULL)
    {
     if (regtry(prog, s)) return(1);
     s++;
    }
  else
   /* We don't -- general case. */
   do
    {
     if (regtry(prog, s)) return(1);
    }
   while (*s++ != '\0');

  /* Failure. */
  return(0);
}

/*
 - regtry - try match at specific point
 */
//---------------------------------------------------------------------------
int TRegExp::regtry(regexp *prog, const char *string)        /* 0 failure, 1 success */
{
  int i;
  const char** sp;
  const char** ep;

  reginput = string;
  regstartp = prog->startp;
  regendp = prog->endp;

  sp = prog->startp;
  ep = prog->endp;
  for (i = NSUBEXP; i >= 0; i--)
   {
    *sp++ = NULL;
    *ep++ = NULL;
   }
  if (regmatch(prog->program + 1))
   {
    prog->startp[0] = string;
    prog->endp[0] = reginput;
    return(1);
   }
  else return(0);
}

/*
 - regmatch - main matching routine
 *
 * Conceptually the strategy is simple:  check to see whether the current
 * node matches, call self recursively to see whether the rest matches,
 * and then act accordingly.  In practice we make some effort to avoid
 * recursion, in particular by going through "ordinary" nodes (that don't
 * need to know whether the rest of the match failed) by a loop instead of
 * by recursion.
 */
//---------------------------------------------------------------------------
int TRegExp::regmatch(char *prog)       /* 0 failure, 1 success */
{
 char *scan;    /* Current node. */
 char *next;    /* Next node. */

 scan = prog;
 while (scan != NULL)
  {
   next = regnext(scan);
   switch (OP(scan))
    {
     case BOL:
      if (reginput != regbol) return(0);
     break;
     case EOL:
      if (*reginput != '\0') return(0);
     break;
     case ANY:
      if (*reginput == '\0') return(0);
      reginput++;
     break;
     case EXACTLY:
      {
       int len;
       char *opnd;

       opnd = OPERAND(scan);
       /* Inline the first character, for speed. */
       if (*opnd != *reginput) return(0);
       len = (int)strlen(opnd);
       if (len > 1 && strncmp(opnd, reginput, len) != 0) return(0);
       reginput += len;
      }
     break;
     case ANYOF:
      if (*reginput == '\0' || strchr(OPERAND(scan), *reginput) == NULL)
        return(0);
      reginput++;
     break;
     case ANYBUT:
      if (*reginput == '\0' || strchr(OPERAND(scan), *reginput) != NULL)
        return(0);
      reginput++;
     break;
     case NOTHING:
             break;
     case BACK:
             break;
     case OPEN+1:
     case OPEN+2:
     case OPEN+3:
     case OPEN+4:
     case OPEN+5:
     case OPEN+6:
     case OPEN+7:
     case OPEN+8:
     case OPEN+9:
     case OPEN+10:
     case OPEN+11:
     case OPEN+12:
     case OPEN+13:
     case OPEN+14:
     case OPEN+15:
     case OPEN+16:
     case OPEN+17:
      {
       int no;
       const char* save;

       no = OP(scan) - OPEN;
       save = reginput;

       if (regmatch(next))
        {
         /*
          * Don't set startp if some later
          * invocation of the same parentheses
          * already has.
          */
         if (regstartp[no] == NULL) regstartp[no] = save;
         return(1);
        }
       else return(0);
      }
     break;
     case CLOSE+1:
     case CLOSE+2:
     case CLOSE+3:
     case CLOSE+4:
     case CLOSE+5:
     case CLOSE+6:
     case CLOSE+7:
     case CLOSE+8:
     case CLOSE+9:
     case CLOSE+10:
     case CLOSE+11:
     case CLOSE+12:
     case CLOSE+13:
     case CLOSE+14:
     case CLOSE+15:
     case CLOSE+16:
     case CLOSE+17:
      {
       int no;
       const char* save;

       no = OP(scan) - CLOSE;
       save = reginput;

       if (regmatch(next))
        {
         /*
          * Don't set endp if some later
          * invocation of the same parentheses
          * already has.
          */
         if (regendp[no] == NULL) regendp[no] = save;
         return(1);
        }
       else return(0);
      }
     break;
     case BRANCH:
      {
       const char* save;

       /* No choice. */
       if (OP(next) != BRANCH) next = OPERAND(scan); /* Avoid recursion. */
       else
        {
         do
          {
           save = reginput;
           if (regmatch(OPERAND(scan))) return(1);
           reginput = save;
           scan = regnext(scan);
          }
         while (scan != NULL && OP(scan) == BRANCH);
         return(0);
         /* NOTREACHED */
        }
      }
     break;
     case STAR:
     case PLUS:
      {
       char nextch;
       int no;
       const char* save;
       int min;

       /*
        * Lookahead to avoid useless match attempts
        * when we know what character comes next.
        */
       nextch = '\0';
       if (OP(next) == EXACTLY) nextch = *OPERAND(next);
       min = (OP(scan) == STAR) ? 0 : 1;
       save = reginput;
       no = regrepeat(OPERAND(scan));
       while (no >= min)
        {
          /* If it could work, try it. */
          if (nextch == '\0' || *reginput == nextch)
            if (regmatch(next)) return(1);
          /* Couldn't or didn't -- back up. */
          no--;
          reginput = save + no;
        }
       return(0);
      }
     break;
     case END:
      return(1);      /* Success! */
     break;
     default:
      regerror = RE_INVALIDPARAMETER;
      return(0);
     break;
    }
   scan = next;
  }

 /*
  * We get here only if there's trouble -- normally "case END" is
  * the terminating point.
  */
 regerror = RE_INVALIDPARAMETER;
 return(0);
}

/*
 - regrepeat - repeatedly match something simple, report how many
 */
//---------------------------------------------------------------------------
int TRegExp::regrepeat(char *p)
{
  int count = 0;
  const char* scan;
  char *opnd;

  scan = reginput;
  opnd = OPERAND(p);
  switch (OP(p))
   {
    case ANY:
     count = (int)strlen(scan);
     scan += count;
    break;
    case EXACTLY:
     while (*opnd == *scan)
      {
       count++;
       scan++;
      }
    break;
    case ANYOF:
     while (*scan != '\0' && strchr(opnd, *scan) != NULL)
      {
       count++;
       scan++;
      }
    break;
    case ANYBUT:
     while (*scan != '\0' && strchr(opnd, *scan) == NULL)
      {
       count++;
       scan++;
      }
    break;
    default:                /* Oh dear.  Called inappropriately. */
     regerror = RE_INTERNAL;
     count = 0;      /* Best compromise. */
    break;
   }
  reginput = scan;

  return(count);
}

/*
 - regnext - dig the "next" pointer out of a node
 */
//---------------------------------------------------------------------------
char * TRegExp::regnext( char *p)
{
  int offset;

  if (p == &regdummy) return(NULL);

  offset = NEXT(p);
  if (offset == 0) return(NULL);

  if (OP(p) == BACK) return(p-offset);
  else return(p+offset);
}


/*
 - regsub - perform substitutions after a regexp match
 */

//---------------------------------------------------------------------------
int TRegExp::regsub(regexp *prog, const char *replace, char *dest)
{
 int i, j;
 char c;
 int no;
 int res;

 res = 0;
 if (dest) dest[0] = '\0';
 if ((prog) && (replace) && (UCHARAT(prog->program) == MAGIC))
  {
   i = 0;
   j = 0;
   c = replace[i];
   i++;
   while (replace[i-1])
    {
      if (c == '&') no = 0;
      else
      if ((c == '\\') && (replace[i]) &&
          (replace[i] >= '0') &&
          (replace[i] <= '9'))
       {
         no = replace[i] - '0' + 1;
         i++;
       }
      else
      if ((c == '\\') && (replace[i]) &&
          (replace[i] >= 'A') &&
          (replace[i] <= 'F'))
       {
         no = replace[i] - 'A' + 11;
         i++;
       }
      else no = -1;

      if (no < 0)
       {
         /* Ordinary character. */
         if ((c == '\\') && (replace[i]))
          {
            c = replace[i];
            if (c == 'b') c = '\x08';
            else
            if (c == 't') c = '\x09';
            else
            if (c == 'n') c = '\x0a';
            else
            if (c == 'v') c = '\x0b';
            else
            if (c == 'f') c = '\x0c';
            else
            if (c == 'r') c = '\x0d';
            else
            if (c == 'e') c = '\x1b';
            i++;
          }
         if (dest) dest[j] = c;
         j++;
       }
      else
      if ((prog->startp[no]) && (prog->endp[no]))
       {
        memmove(&dest[j], prog->startp[no], prog->endp[no] - prog->startp[no]);
        j += (int)(prog->endp[no] - prog->startp[no]);
        if (dest) dest[j] = '\0';
       }
      c = replace[i];
      i++;
     }
   if (dest) dest[j] = '\0';
   res = 1;
  }
 return res;
}
//---------------------------------------------------------------------------
/*
 - regsub - perform substitutions after a regexp match
 return lenght of the result string (dest),
 if dest == NULL only return lenght
 */

int TRegExp::regsub(regexp *prog, const char *replace, char *dest, int destsize)
{
 int i, j;
 char c;
 int no;

 if (dest) dest[0] = '\0';
 if ((prog) && (replace) && (UCHARAT(prog->program) == MAGIC))
  {
   i = 0;
   j = 0;
   c = replace[i];
   i++;
   while (replace[i-1])
    {
      if (c == '&') no = 0;
      else
      if ((c == '\\') && (replace[i]) &&
          (replace[i] >= '0') &&
          (replace[i] <= '9'))
       {
         no = replace[i] - '0' + 1;
         i++;
       }
      else
      if ((c == '\\') && (replace[i]) &&
          (replace[i] >= 'A') &&
          (replace[i] <= 'F'))
       {
         no = replace[i] - 'A' + 11;
         i++;
       }
      else no = -1;

      if (no < 0)
       {
         /* Ordinary character. */
         if ((c == '\\') && (replace[i]))
          {
            c = replace[i];
            if (c == 'b') c = '\x08';
            else
            if (c == 't') c = '\x09';
            else
            if (c == 'n') c = '\x0a';
            else
            if (c == 'v') c = '\x0b';
            else
            if (c == 'f') c = '\x0c';
            else
            if (c == 'r') c = '\x0d';
            else
            if (c == 'e') c = '\x1b';
            i++;
          }
         if (j < destsize)
          {
           if (dest) dest[j] = c;
           j++;
          }
       }
      else
      if ((prog->startp[no]) && (prog->endp[no]))
       {
        int size = (int)(prog->endp[no] - prog->startp[no]);
        if (j+size < destsize)
         {
          if (dest) memmove(&dest[j], prog->startp[no], size);
          j += size;
         }
        if (dest) dest[j] = '\0';
       }
      c = replace[i];
      i++;
     }
   if (dest) dest[j] = '\0';
  }
 return j;
}


//---------------------------------------------------------------------------
void TRegExp::regfree(regexp *prog)
{
 if (prog) free(prog);
}

//---------------------------------------------------------------------------
char const *TRegExp::regerr(void)
{
 return serrors[regerror];
}
//---------------------------------------------------------------------------
