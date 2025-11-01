#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <stdarg.h>

#define NVCL
#include "RegExpClass.h"

#pragma warning(disable: 4996) // Disable deprecated function warnings

#define CP_UTF16        4
#define STRLEN          1024
#define RECNUM          16

// Split slashes ('/') divided string to <num> fields. Return fields count.
int nsplit(int num, char* str, ...)
{
 char c, cc;
 int i = 0;
 int n = 0;
 char **ostr = &str; //ostr[1] point to first <...> argument
 do
  {
   c = str[i++];
   if (c) cc = str[i];  // next character
   if ((c == '\n') || (c == '\r')) // ignore
    {
     if (cc) i++;
     continue;
    }
   if (c == '\t') c = ' '; // change
   if (c == '/')
    {
     if (cc == '/') i++; // screaning
     else
      {
       n++;
       continue;
      }
    }
   if (n && ostr[n])
    {
     *ostr[n]++ = c;
     *ostr[n] = '\0';
    }
  }
 while ((c)&&(n<=num));
 for(i=n; i <= num; i++) if (ostr[n]) *ostr[n] = '\0';
 return n;
}
/* Create output file and write signature */
/* Recognize '+' in the name's beginning as append sign */
/* and double '++' as single '+' as part of the name */
FILE* FCreate(char* outfn, unsigned int code)
{
    FILE* Out = NULL;
    if (outfn[0])
    {
        bool append = false;
        if ((outfn[0] == '+') && (outfn[1] != '+'))
        {
            Out = fopen(&outfn[1], "a+b");
            append = true;
        }
        else
            if ((outfn[0] == '+') && (outfn[1] == '+')) Out = fopen(&outfn[1], "w+b");
            else  Out = fopen(outfn, "w+b");
        if (!Out) Out = stdout;
        switch (code)
        {
        case CP_ACP:
            break;
        case CP_UTF8:
        {
            unsigned char u[] = { 0xef, 0xbb, 0xbf };
            if (!append) fwrite(&u, 3, 1, Out);
        }
        break;
        case CP_UTF16:
        {
            unsigned short u = 0xfeff;
            if (!append) fwrite(&u, 2, 1, Out);
        }
        break;
        }
    }
    return Out;
}

/* detect input file codepage */
unsigned int fCode(FILE* f)
{
    unsigned char sign[3];
    unsigned int codepage = CP_ACP;
    if (f)
    {
        int r = (int)fread(sign, sizeof(sign), 1, f);
        if (r > 0)
        {
            if ((sign[0] == 0xFF) && (sign[1] == 0xFE))
                codepage = CP_UTF16;
            else
                if ((sign[0] == 0xEF) && (sign[1] == 0xBB) && (sign[2] == 0xBF))
                    codepage = CP_UTF8;
        }
        fseek(f, 0, SEEK_SET);
    }
    return codepage;
}

/* check string for eol and correct it. return true if eol found */
bool sceol(char* ostr, int& size)
{
    bool eol = false;
    if ((size > 1) && (ostr[size - 1] == '\n'))
    {
        eol = true;
        if ((size > 2) && (ostr[size - 2] == '\r'));
        else
        {
            ostr[size - 1] = '\r';
            ostr[size++] = '\n';
            ostr[size] = 0;
        }
    }
    else
        if ((size > 1) && (ostr[size - 1] == '\r'))
        {
            eol = true;
            ostr[size++] = '\n';
            ostr[size] = 0;
        }
    return eol;
}

/* check wide string for eol and correct it. return true if eol found */
bool sweol(wchar_t* sostr, int& size)
{
    bool eol = false;
    if ((size > 1) && (sostr[size - 1] == '\n'))
    {
        eol = true;
        if ((size > 2) && (sostr[size - 2] == '\r'));
        else
        {
            sostr[size - 1] = '\r';
            sostr[size++] = '\n';
            sostr[size] = 0;
        }
    }
    else
        if ((size > 1) && (sostr[size - 1] == '\r'))
        {
            eol = true;
            sostr[size++] = '\n';
            sostr[size] = 0;
        }
    return eol;
}

/* write string as ANSI, UTF-8 and UTF-16 text, return symbols count */
/* code = CP_ACP for ANSI string in default codepage */
/* code = CP_UTF8 for UTF-8 string (chars < 127 is original) */
/* code = CP_UTF16 for UTF-16 string (two bytes per symbol) */
int uwritestr(UINT in_code, UINT out_code, char* ostr, bool force_eol, FILE* f)
{
    static const unsigned short ueol[] = { '\r', '\n' };
    static const unsigned char ceol[] = { '\r', '\n' };
    int r = 0;

    switch (out_code)
    {
    case CP_UTF8:
    case CP_ACP:
    {
        switch (in_code)
        {
        case CP_ACP:
        {
            int size = (int)strlen(ostr);
            if (size)
            {
                // check eol and correct it. Clear force_eol if eol found
                if (sceol(ostr, size)) force_eol = false;
                // Request size for wide string
                int wsize = MultiByteToWideChar(CP_ACP, 0, ostr, -1, NULL, 0);
                if (wsize)
                {
                    // allocate buffer for wide string
                    wchar_t* wsostr = new wchar_t[wsize + 1];
                    // convert from ASCII to UTF-16 (wide string)
                    int n = MultiByteToWideChar(CP_ACP, 0, ostr, size, wsostr, wsize);
                    // Request size for UTF-8 string
                    int u8sz = WideCharToMultiByte(CP_UTF8, 0, wsostr, n, NULL, 0, NULL, NULL);
                    // allocate buffer for UTF-8 string
                    char* u8str = new char[u8sz];
                    // convert from UTF-16 (wide string) to UTF-8 string
                    int m = WideCharToMultiByte(CP_UTF8, 0, wsostr, n, u8str, u8sz, NULL, NULL);
                    // free wide string buffer
                    delete[] wsostr;
                    // save UTF-8 string to file
                    if (m) r += (int)fwrite(u8str, 1, m, f);
                    // free UTF-8 string buffer
                    delete[] u8str;
                }
                // write EOL
                if (force_eol) r += (int)fwrite(ceol, 1, 2, f);
            }
        }
        break;
        case CP_UTF8:
        {
            int size = (int)strlen(ostr);
            if (size)
            {
                // check eol and correct it. Clear force_eol if eol found
                if (sceol(ostr, size)) force_eol = false;
                r += (int)fwrite(ostr, 1, size, f);
                // write EOL
                if (force_eol) r += (int)fwrite(ceol, 1, 2, f);
            }
        }
        }
    }
    break;
    case CP_UTF16:
    {
        if (in_code == CP_UTF16) in_code = CP_UTF8; //CP_ACP;
        //Convert UTF-8 to UTF-16
        // Calculate size
        int size = MultiByteToWideChar(in_code, 0, ostr, -1, NULL, 0);
        // allocate buffer
        if (size)
        {
            wchar_t* sostr = new wchar_t[size + 1];
            // convert
            size = MultiByteToWideChar(in_code, 0, ostr, -1, sostr, size);
            // write to file
            while (!sostr[size - 1]) size--;
            // check eol and correct it. Clear force_eol if eol found
            if (sweol(sostr, size)) force_eol = false;
            r += (int)fwrite(sostr, 2, size, f);
            // free buffer
            delete[] sostr;
            // write EOL
            if (force_eol) r += (int)fwrite(ueol, 2, 2, f);
        }
    }
    break;
    }
    return r;
}

/* read string from ASCII, UTF-8 and UTF-16 files */
/* return UTF-8 sequence for processing. I.e. ASCII symbols present as is and */
/* non-ASCII symbols in UTF-8 codepage. Regular expressions engine match ASCII */
/* part of string and UTF-8 sequence matches as any symbol '.' */
char* ugets(UINT in_code, char* str, int size, FILE* f)
{
    char* res = NULL;
    switch (in_code)
    {
    case CP_ACP:
    case CP_UTF8:
    {
        if (fgets(str, size, f))
        {
            int i = (int)strlen(str);
            while ((--i > 0) && ((str[i] == '\n') || (str[i] == '\r'))) str[i] = '\0';
            res = str;
        }
        else res = NULL;
    }
    break;
    case CP_UTF16:
    {
        int i = 0;
        bool eof;
        wint_t r;
        do
        {
            eof = (r = fgetwc(f)) == WEOF;
            if (!eof && (i < size) && (r < 0xFEFF)) //UTF-16 file signature
            {
                wchar_t wc = (wchar_t)r;
                char ss[4];
                if ((r != 10) && (r != 13))  //ignore CR LF
                {
                    // convert wide char to UTF-8 sequence for processing
                    int n = WideCharToMultiByte(CP_UTF8, 0, &wc, 1, ss, sizeof(ss), NULL, NULL);

                    for (int ii = 0; (ii < n) && ss[ii]; ii++)
                    {
                        str[i++] = ss[ii];
                        str[i] = '\0';
                    }
                }
            }
        } while (!eof && (i < size) && (r != 13));
        if (i) res = str;
        else res = NULL;
    }
    }
    return res;
}

/*
CP_UTF16        = 4 (only this)
CP_ACP          = 0
CP_MACCP        = 2
CP_OEMCP        = 1
CP_SYMBOL       = 42
CP_THREAD_ACP   = 3
CP_UTF7         = 65000
CP_UTF8         = 65001

int MultiByteToWideChar(
    UINT CodePage,    // code page
    DWORD dwFlags,    // character-type options
    LPCSTR lpMultiByteStr,    // address of string to map
    int cchMultiByte,    // number of characters in string
    LPWSTR lpWideCharStr,    // address of wide-character buffer
    int cchWideChar     // size of buffer
   );

int WideCharToMultiByte(
    UINT CodePage,    // code page
    DWORD dwFlags,    // performance and mapping flags
    LPCWSTR lpWideCharStr,    // address of wide-character string
    int cchWideChar,    // number of characters in string
    LPSTR lpMultiByteStr,    // address of buffer for new string
    int cchMultiByte,    // size of buffer
    LPCSTR lpDefaultChar,    // address of default for unmappable characters
    LPBOOL lpUsedDefaultChar     // address of flag set when default char. used
   );
*/

const char help0[] = {
    "rxsnr Regular expression search and replace.\n"
    " (c)Dmitry Orlov (dimorlus@gmail.com), 2014, 2025.\n\n"
    "rxsnr [-o|-d|-u] /search/replace[/num] <input>output\n"
    "OR\n"
    "rxsnr [-o|-d|-u] /search/replace/num/input(/[+]output)|(>output)\n"
    "OR\n"
    "rxsnr [-o|-d|-u] sr.txt <input>output\n"
    "OR\n"
    "rxsnr [-o|-d|-u] sr.txt input (output)|(>output)\n"
    "sr.txt:\n/search/replace[/num]\n"
    "OR\n"
    "rxsnr [-o|-d|-u] sr.txt\n"
    "sr.txt:\n/search/replace/num/input/[+]output\n"
    "OR\n"
    "rxsnr [-o|-d|-u] sr.txt >output\n"
    "sr.txt:\n/search/replace/num/input\n\n"
    "Each record use previous record result.\n"
    "+output = append output file\n\n"
    "      -h  - help screen, H - full help to stdout.\n"
    "      -b  - break expressions list after first match.\n"
    "      -u  - unicode (UTF-16) output (ASCII if not).\n"
    "      -8  - unicode (UTF-8) output.\n"
    "      -s  - stream mode (ignore input eol).\n"
    "      -d  - debug mode.\n"
};

const char help1[] = {
    "\nRead string from ASCII, UTF-8 and UTF-16 files.\n"
    "Use UTF-8 sequence for processing. I.e. ASCII symbols present as is and\n"
    "non-ASCII symbols in UTF-8 codepage. Regular expressions engine match ASCII\n"
    "part of string and UTF-8 sequence matches as any symbol '.'\n"
};

const char help2[] = {
    "Regular expressions are characters that customize a search string.\n"
    "The product recognizes these regular expressions:\n"
    "\n"
    "Search expression:\n"
    "\n"
    "^       A circumflex at the start of the string matches the start of a line.\n"
    "$       A dollar sign at the end of the expression matches the end of a line.\n"
    ".       A period matches any character.\n"
    "*       An asterisk after a string matches any number of occurrences of that\n"
    "        string followed by any characters, including zero characters.\n"
    "        For example, bo* matches bot, bo and boo but not b.\n"
    "+       A plus sign after a string matches any number of occurrences of that\n"
    "        string followed by any characters except zero characters.\n"
    "        For example, bo+ matches boo, and booo, but not bo or be.\n"
    "?       A question mark makes the preceding item optional. Greedy, so the\n"
    "        optional item is included in the match if possible.\n"
    "        For example, abc? matches ab or abc.\n"
    "[ ]     Characters in brackets match any one character that appears in the\n"
    "        brackets, but no others. For example [bot] matches b, o, or t.\n"
    "[^]     A circumflex at the start of the string in brackets means NOT.\n"
    "        Hence, [^bot] matches any characters except b, o, or t.\n"
    "[-]     A hyphen within the brackets signifies a range of characters.\n"
    "        For example, [b-o] matches any character from b through o.\n"
    "        For example, [a-z,A-Z,!,$,%%,_,#,0-9]+\n"
    "( )     Braces group characters or expressions. Groups can be nested, with\n"
    "        a maximum number of 16 groups in a single pattern. For the Replace\n"
    "        operation, the groups are referred to by a backslash and a number\n"
    "        according to the position in the \"Text to find\" expression, beginning\n"
    "        with 0.\n"
    "        For example, given the text to find and replacement strings,\n"
    "        Find: ([0-9])([a-c]*), Replace: NUM\\1, the string 3abcabc is changed\n"
    "        to NUMabcabc.\n"
    "|       A vertical bar matches either expression on either side of the\n"
    "        vertical bar.\n"
    "        For example, bar|car will match either bar or car.\n"
    "\\       A backslash before a wildcard character tells the Code editor to\n"
    "        treat that\n"
    "        character literally, not as a wildcard. For example, \\^ matches ^ and\n"
    "        does not look for the start of a line.\n"
    "\\esc    Escape characters.\n"
    "\n"
    "Replace expression:\n"
    "\n"
    "\\0..\\9  Recalls stored substring from matched pattern ()'s.\n"
    "\\A..\\F  Recalls stored substring from matched pattern ()'s from 10 to 16.\n"
    "\\esc    Escape characters.\n"
    "&       Recalls entire matched pattern at replace.\n"
    "\n"
    "Escape Characters:\n"
    "\n"
    "Sequence        Description             DEC     HEX\n"
    "\\b              Backspace               8       0x08\n"
    "\\t              (Horizontal) Tab        9       0x09\n"
    "\\n              New Line Feed (LF)      10      0x0a\n"
    "\\v              Vertical Tab            11      0x0b\n"
    "\\f              Form Feed               12      0x0c\n"
    "\\r              Carriage Return (CR)    13      0x0d\n"
    "\\e              Escape                  27      0x1b\n"
};

class csr
{
private:
    char* sstr;
    char* rstr;
    int num;
    TRegExp* reg;
public:
    csr(char* str);
    bool issr(void) { return reg != NULL; }
    bool search(char* str);
    void replace(char* str);
    const char* get_sstr(void) { return sstr; };
    const char* get_rstr(void) { return rstr; };
    int get_num(void) { return num; };
    ~csr(void);
};

csr::csr(char* str)
{
    bool fail = true;
    char* nstr;
    int size = (int)strlen(str)+1;
    sstr = new char[size];
    rstr = new char[size];
    nstr = new char[size];
    reg = NULL;
    num = -1;
    // Initialize strings
    sstr[0] = rstr[0] = nstr[0] = '\0';
    
    if (nsplit(3, str, sstr, rstr, nstr))
    {
        if (nstr[0]) num = atoi(nstr);
        if (sstr[0] && rstr[0]) reg = new TRegExp(sstr);
        if (reg && !reg->RegError()) fail = false;
        else if (reg) fprintf(stderr, "'%s'\nRegExp error %s.\n", sstr, reg->regerr());
    }
    delete[] nstr;
    if (fail)
    {
        delete[] sstr;
        sstr = NULL;
        delete[] rstr;
        rstr = NULL;
        if (reg) delete reg;
        reg = NULL;
    }
};

csr::~csr(void)
{
    if (sstr) delete[] sstr;
    if (rstr) delete[] rstr;
    if (reg) delete reg;
}

bool csr::search(char* str)
{
    return reg && reg->Match(str);
}

void csr::replace(char* str)
{
    if (reg)
    {
        if (num < 0) reg->Replace(rstr, str);
        else
        {
            // Ѕезопасное формирование строки замены с номером
            char nrstr[256];
            int written = snprintf(nrstr, sizeof(nrstr), rstr, num++);
            if (written > 0 && written < (int)sizeof(nrstr)) {
                reg->Replace(nrstr, str);
            } else {
                // ≈сли строка не поместилась, можно обработать ошибку или обрезать
                nrstr[sizeof(nrstr) - 1] = '\0';
                reg->Replace(nrstr, str);
            }
        }
    }
}

csr* SR[RECNUM];
char istr[STRLEN * 4];  //input string (stream) buffer
char ostr[STRLEN * 4];  //output buffer

void Halt()
{
    for (int i = 0; i < RECNUM; i++)
        if (SR[i]) delete SR[i];
}

#define _OLD_CODE_
//#define _LLM_CODE_
int main(int argc, char* argv[])
{
    char infn[STRLEN];
    char outfn[STRLEN];

    FILE* In = NULL;
    FILE* Out;

    bool br = false; // break expressions
    bool debug = false; //debug mode.
    bool unicode = false; //unicode output.
    bool stream = false; //stream mode (ignore input eol).
    bool utf8 = false; //UTF-8 output.
    bool help = false; // print help.
    bool HELP = false; // print full help to stdout.

    int records = 0;

    _set_fmode(_O_BINARY); // Set default file mode to binary

    memset(SR, 0, sizeof(SR));
    memset(infn, 0, sizeof(infn));
    memset(outfn, 0, sizeof(outfn));
    
//Read search string from command line
 char* cmd = GetCommandLineA();
#ifdef _OLD_CODE_
 memset(SR, 0, sizeof(SR));
 SR[0] = new csr(cmd);
 if (SR[0]->issr())
  {
   records++;
   nsplit(5, cmd, NULL, NULL, NULL, infn, outfn);
   if (strstr(cmd, "-b ")) br = true;
   if (strstr(cmd, "-d ")) debug = true;
   if (strstr(cmd, "-u ")) unicode = true;
   if (strstr(cmd, "-8 ")) utf8 = true;
   if (strstr(cmd, "-s ")) stream = true;
   if (strstr(cmd, "-h ")) help = true;
   if (strstr(cmd, "-H ")) HELP = true;
  }
 else
 if (argc > 1)
  {
   int arg_i;
   for(arg_i = 1; arg_i < argc; arg_i++)
    {
     if ((argv[arg_i][0] == '-') && (argv[arg_i][2] == '\0'))
      {
       if (argv[arg_i][1] == 'b') br = true;
       if (argv[arg_i][1] == 'd') debug = true;
       if (argv[arg_i][1] == 'u') unicode = true;
       if (argv[arg_i][1] == '8') utf8 = true;
       if (argv[arg_i][1] == 's') stream = true;
       if (argv[arg_i][1] == 'h') help = true;
       if (argv[arg_i][1] == 'H') HELP = true;
       continue;
      }
     else break;
    }
   FILE * SFile = fopen(argv[arg_i], "rt");
   if (SFile)
    {
     int i = 0;
     while ((fgets(istr, sizeof(istr), SFile)) && (i < RECNUM))
      {
       if (istr[0] != ';') SR[i] = new csr(istr);
       else continue;
       if (SR[i]&&SR[i]->issr())
        {
         if (i==0) nsplit(5, istr, NULL, NULL, NULL, infn, outfn);
         i++;
        }
      }
     fclose(SFile);
     records = i;
    }
   if (argc > arg_i+1) strcpy(infn, argv[arg_i+1]);
   if (argc > arg_i+2) strcpy(outfn, argv[arg_i+2]);
  }
#endif //_OLD_CODE_

#ifdef _LLM_CODE_
    // First parse all flags
    int arg_start = 1;
    for (int i = 1; i < argc; i++)
    {
        if ((argv[i][0] == '-') && (argv[i][2] == '\0'))
        {
            if (argv[i][1] == 'b') br = true;
            if (argv[i][1] == 'd') debug = true;
            if (argv[i][1] == 'u') unicode = true;
            if (argv[i][1] == '8') utf8 = true;
            if (argv[i][1] == 's') stream = true;
            if (argv[i][1] == 'h') help = true;
            if (argv[i][1] == 'H') HELP = true;
            arg_start = i + 1;
        }
        else break;
    }
    
    // Check for command line search pattern (starts with /)
    if (arg_start < argc && argv[arg_start][0] == '/')
    {
        if (debug) fprintf(stderr, "Creating csr with pattern: '%s'\n", argv[arg_start]);
        SR[0] = new csr(argv[arg_start]);
        if (SR[0] && SR[0]->issr())
        {
            if (debug) fprintf(stderr, "Pattern successfully created\n");
            records++;
            // Parse remaining arguments for input/output files
            if (arg_start + 1 < argc) strcpy(infn, argv[arg_start + 1]);
            if (arg_start + 2 < argc) strcpy(outfn, argv[arg_start + 2]);
        }
        else
        {
            if (debug) fprintf(stderr, "Pattern creation failed\n");
            if (SR[0]) delete SR[0];
            SR[0] = NULL;
        }
    }
    else
        if (arg_start < argc)
        {
            FILE* SFile = fopen(argv[arg_start], "rt");
            if (SFile)
            {
                int i = 0;
                while ((fgets(istr, sizeof(istr), SFile)) && (i < RECNUM))
                {
                    // Remove trailing newline characters
                    int len = (int)strlen(istr);
                    while (len > 0 && (istr[len - 1] == '\n' || istr[len - 1] == '\r'))
                    {
                        istr[--len] = '\0';
                    }
                    
                    if (istr[0] != ';' && istr[0] != '\0') 
                    {
                        SR[i] = new csr(istr);
                        if (SR[i] && SR[i]->issr())
                        {
                            if (i == 0) nsplit(5, istr, NULL, NULL, NULL, infn, outfn);
                            i++;
                        }
                        else if (SR[i])
                        {
                            delete SR[i];
                            SR[i] = NULL;
                        }
                    }
                }
                fclose(SFile);
                records = i;
            }
            if (argc > arg_start + 1) strcpy(infn, argv[arg_start + 1]);
            if (argc > arg_start + 2) strcpy(outfn, argv[arg_start + 2]);
        }
#endif //_LLM_CODE_
    unsigned int out_code = CP_ACP;
    if (utf8) out_code = CP_UTF8;
    else
        if (unicode) out_code = CP_UTF16;

    if (debug)
    {
        fprintf(stderr, "In file : '%s'\nOut file: '%s' : ", infn, outfn);
        switch (out_code)
        {
        case CP_ACP: fprintf(stderr, "ASCII"); break;
        case CP_UTF8: fprintf(stderr, "UTF-8"); break;
        case CP_UTF16: fprintf(stderr, "UTF-16"); break;
        }
        fprintf(stderr, "\n");
        fprintf(stderr, "Options : -d=%d, -b=%d, -u=%d, -s=%d, -h=%d\n\n",
            debug, br, unicode, stream, help);
        for (int ii = 0; ii < records; ii++)
            if (SR[ii])
                fprintf(stderr, "Search[%d] : '%s'\nReplace[%d]: '%s'\nNum[%d]    : %d\n\n",
                    ii, SR[ii]->get_sstr(), ii, SR[ii]->get_rstr(), ii, SR[ii]->get_num());
    }

    if (help)
    {
        fprintf(stderr, help0);
        fprintf(stderr, help1);
        Halt();
        return 1;
    }

    if (HELP)
    {
        fprintf(stdout, help0);
        fprintf(stdout, help1);
        fprintf(stdout, help2);
        Halt();
        return 1;
    }

    if (records)
    {
        if (infn[0]) In = fopen(infn, "rb");
        if (!In) In = stdin;
        unsigned int in_code = fCode(In);
        Out = FCreate(outfn, out_code);
        if ((In) && (Out))
        {
            if (stream)
            { // Stream processing
                int si = 0;
                bool eof;
                int r;
                do
                {
                    eof = true;
                    if (in_code == CP_UTF16) eof = (r = fgetwc(In)) == WEOF;
                    else eof = (r = fgetc(In)) == EOF;
                    if (!eof)
                    {
                        if (in_code == CP_UTF16)
                        {
                            if (r < 0xFEFF)
                            {
                                wchar_t wc = (wchar_t)r;
                                char ss[4];
                                int n = WideCharToMultiByte(CP_UTF8, 0, &wc, 1, ss, sizeof(ss), NULL, NULL);

                                if (si >= sizeof(istr) - 1)  //roll string left
                                {
                                    for (int ii = 0; ii < sizeof(istr) - 1; ii++) istr[ii] = istr[ii + 1];
                                    si--;
                                }

                                for (int ii = 0; (ii < n) && ss[ii]; ii++)
                                {
                                    istr[si++] = ss[ii];
                                    istr[si] = '\0';
                                }
                            }
                        }
                        else
                        {
                            char c = (char)r;
                            if (si >= sizeof(istr) - 1)   //roll string left
                            {
                                for (int ii = 0; ii < sizeof(istr) - 1; ii++) istr[ii] = istr[ii + 1];
                                si--;
                            }
                            istr[si++] = c;
                            istr[si] = '\0';
                        }

                        if (istr[0])
                            for (int sri = 0; sri < records; sri++)
                                if (SR[sri] && SR[sri]->search(istr))
                                {
                                    si = 0;
                                    //strcpy(ostr, istr);
                                    SR[sri]->replace(ostr);
                                    if (ostr[0]) uwritestr(in_code, out_code, ostr, false, Out);
                                    if (br) break;
                                }
                    }
                } while (!eof);
            }
            else
            { //Line processing
                while (ugets(in_code, istr, sizeof(istr), In))
                {
                    if (istr[0])
                        for (int sri = 0; sri < records; sri++)
                        {
                            if (SR[sri] && SR[sri]->search(istr))
                            {
                                //strcpy(ostr, istr);
                                SR[sri]->replace(ostr);
                                if (ostr[0]) uwritestr(in_code, out_code, ostr, true, Out);
                                if (br) break;
                            }
                        }
                }
            }
            if (Out != stdout) fclose(Out);
            if (In != stdin) fclose(In);
            Halt();
            return 0;
        }
    }
    else
    {
        fprintf(stderr, help0);
        Halt();
        return 1;
    }
    
    Halt();
    return 0;
}