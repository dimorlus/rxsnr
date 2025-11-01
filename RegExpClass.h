#pragma once
#ifndef RegExpClassH
#define RegExpClassH

#include <windows.h>

// Regular Expression Error Codes
#define RE_OK                   0
#define RE_NOTFOUND             1
#define RE_INVALIDPARAMETER     2
#define RE_EXPRESSIONTOOBIG     3
#define RE_OUTOFMEMORY          4
#define RE_TOOMANYSUBEXPS       5
#define RE_UNMATCHEDPARENS      6
#define RE_INVALIDREPEAT        7
#define RE_NESTEDREPEAT         8
#define RE_INVALIDRANGE         9
#define RE_UNMATCHEDBRACKET     10
#define RE_TRAILINGBACKSLASH    11
#define RE_INTERNAL             12

#define NSUBEXP  17

class TRegExp
{
    typedef struct regexp {
        const char* startp[NSUBEXP + 1];
        const char* endp[NSUBEXP + 1];
        char regstart;          /* Internal use only. */
        char reganch;           /* Internal use only. */
        char* regmust;          /* Internal use only. */
        int regmlen;            /* Internal use only. */
        char program[1];        /* Internal use only. */
    } regexp;

public:
    TRegExp(const char* Exp);
    ~TRegExp(void);
    bool Match(const char* Str);
    char* startp(int num) { return ((num < NSUBEXP) ? ((char*)pregexp->startp[num]) : NULL); }
    char* endp(int num) { return ((num < NSUBEXP) ? ((char*)pregexp->endp[num]) : NULL); }
    int pos(int num) { return (int)((num < NSUBEXP) ? (((char*)pregexp->startp[num] - regbol)) : -1); }
    int Len(int num) { return (int)((num < NSUBEXP) ? (pregexp->endp[num] - pregexp->startp[num]) : -1); }
    bool Replace(const char* Rpls, char* Res);
    int Replace(const char* Rpls, char* Res, int ResSize);
    int Replace(int no, char* Res, int ResSize);
    int RegError(void) { return regerror; };
    char const* regerr(void);

private:
    regexp* pregexp;
    int regerror;

protected:
    const char* rexp;
    //work variables for regcomp().
    const char* regparse;    /* Input-scan pointer. */
    int regnpar;             /* () count. */
    char regdummy;
    char* regcode;           /* Code-emit pointer; &regdummy = don't. */
    long regsize;            /* Code size. */

    //work variables for regexec().
    const char* reginput;              /* String-input pointer. */
    const char* regbol;              /* Beginning of input, for ^ check. */
    const char** regstartp;            /* Pointer to startp array. */
    const char** regendp;              /* Ditto for endp. */

    //declarations for regcomp()'s friends.
    char* reg(int paren, int* flagp);
    char* regbranch(int* flagp);
    char* regpiece(int* flagp);
    char* regatom(int* flagp);
    char* regnode(char op);
    char* regnext(char* p);
    void regc(unsigned char b);
    void reginsert(char op, char* opnd);
    void regtail(char* p, char* val);
    void regoptail(char* p, char* val);

    //regexec and friends
    int regtry(regexp* prog, const char* string);
    int regmatch(char* prog);
    int regrepeat(char* p);
    void regfree(regexp* prog);

private:
    regexp* regcomp(const char* exp);
    int regexec(regexp* prog, const char* string);
    int regsub(regexp* prog, const char* replace, char* dest);
    int regsub(regexp* prog, const char* replace, char* dest, int destsize);
};

#endif