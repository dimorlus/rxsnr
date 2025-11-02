//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define BUFSIZE  16384  // max section size
#define NUMMAX   256    // max rules number
#define LINELEN  1024   // max line length
typedef enum {erOk, erRegErr, erSectionSize, erRulesCount, erIO} rerrors;
class rules
{
  private:
   char* header;
   int hd_size;
   char* footer;
   int ft_size;
   rerrors err;
   int num;
   int lines;
   int match;
   struct
    {
     TRegExp *reg;
     int match;
     char* replace;
     int rp_size;
    } srules[NUMMAX];
   int get_section(FILE *In, int ic, char **section, int &ssize, char &cc);
  public:
   rules(const char* filename);
   void fheader(FILE *Out) {fputs(header, Out);}
   void fline(const char* line, FILE *Out);
   void ffooter(FILE *Out) {fputs(footer, Out);}
   int rulnum(void) {return num;}
   int linenum(void) {return lines;}
   int matchnum(void) {return match;}
   rerrors error(void) {return err;}
   const char *regerror(void) {return srules[num].reg?srules[num].reg->regerr():"";}
   ~rules(void);
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
int rules::get_section(FILE *In, int ic, char **section, int &ssize, char &cc)
{
 char buf[BUFSIZE];
 int size = 0;
 char c = '\0';
 bool skip = ic != 0;
 do
  {
   if (!skip) ic = fgetc(In);
   skip = false;
   if (ic != EOF)
    {
     if (c)
      {
       if (c == '/')
        {
         if (ic == '/') // skip double "//"
          {
           buf[size++] = c;
           ic = 0;
          }
         else break; // single section delimiter '/' found
        }
       else buf[size++] = c;
      }
     c = ic;
    }
   else break; // EOF
  } while (size < BUFSIZE);
 if (size == BUFSIZE) err = erSectionSize;
 buf[size] = '\0';
 *section = new char[size+1];
 strcpy(*section, buf);
 ssize = size;
 cc = c;
 return ic;
}
//---------------------------------------------------------------------------
rules::rules(const char* filename)
{
 FILE *In = NULL;
 memset(srules, 0, sizeof(srules));
 header = NULL;
 hd_size = 0;
 footer = NULL;
 ft_size = 0;
 num = 0;
 lines = 0;
 match = 0;
 err = erOk;

 In = fopen(filename, "r+b");
 if (In)
  {
   char c = '\0';
   int ic = 0;
   ic = get_section(In, ic, &header, hd_size, c);
   while ((num < NUMMAX) && (err == erOk) && (c == '/'))
    {
     char *regstr = NULL;
     int reglen;
     ic = get_section(In, ic, &regstr, reglen, c);
     srules[num].reg = new TRegExp(regstr);
     if (srules[num].reg && !srules[num].reg->RegError())
      {
       ic = get_section(In, ic, &srules[num].replace, srules[num].rp_size, c);
       num++;
      }
     else err = erRegErr;
     if (regstr) delete [] regstr;
     if (c == '/')
      {
       ic = get_section(In, ic, &footer, ft_size, c);
       if ((c == '/') && footer)
        {
         delete [] footer;
         footer = NULL;
        }
      }
    }
   if (num == NUMMAX) err = erRulesCount;
   fclose(In);
  }
 else err = erIO;
}
//---------------------------------------------------------------------------
void rules::fline(const char* line, FILE *Out)
{
 lines++;
 for(int n = 0; n < num; n++)
  {
    if (srules[n].reg && srules[n].reg->Match(line))
     {
      int outsize = srules[n].reg->Replace(srules[n].replace, NULL, 32768);
      if (outsize)
       {
        if (outsize < 32768)
         {
          match++;
          srules[n].match++;
          char *outline = new char[outsize+1];
          srules[n].reg->Replace(srules[n].replace, outline, outsize);
          fputs(outline, Out);
          delete [] outline;
         }
        else err = erSectionSize;
       }
      break;
     }
  }
}

//---------------------------------------------------------------------------
rules::~rules(void)
{
 if (header) delete [] header;
 if (footer) delete [] footer;
 for(int i = 0; i < num; i++)
  {
   if (srules[i].reg) delete srules[i].reg;
   if (srules[i].replace) delete [] srules[i].replace;
  }
}
//---------------------------------------------------------------------------
const char help0[] = {
 "-t - do not replace tabs with spaces.\n"
 "The input file is processed line by line by each template.\n"
 "If a match is found, the output stream is replaced, after\n"
 "that the next line of the original file is processed.\n"
 "Thus, one line can be processed by only one template. A line\n"
 "matched with the regular expression (template) from the source\n"
 "file can be replaced by one or several lines.\n"};
//---------------------------------------------------------------------------
const char help1[] = {
"\n"
 "The structure of the file for regexp replacement:\n\n"
 "At the beginning of the template file, there can be a header\n"
 "part that is put at the beginning of the output file.\n"
 "\n"
 "\\regexp1\\regreplace1\\\n"
 "\\regexp2\\regreplace2\n"
 "regreplace3\n"
 "regreplace4\\\n"
 "\\regexp3\\regreplace5\\\n"
 "\n"
 "And at the end of the template file, there can be a footer\n"
 "part that is put at the end of the output file.\n"
};
//---------------------------------------------------------------------------
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
 "        For example, [a-z,A-Z,!,$,%,_,#,0-9]+\n"
 "( )     Braces group characters or expressions. Groups can be nested, with\n"
 "        a maximum number of 16 groups in a single pattern. For the Replace\n"
 "        operation, the groups are referred to by a backslash and a number\n"
 "        according to the position in the 'Text to find' expression, beginning\n"
 "        with 0.\n"
 "        For example, given the text to find and replacement strings,\n"
 "        Find: ([0-9])([a-c]*), Replace: NUM\\1, the string 3abcabc is changed\n"
 "        to NUMabcabc.\n"
 "|       A vertical bar matches either expression on either side of the\n"
 "        vertical bar.\n"
 "        For example, bar|car will match either bar or car.\n"
 "\\       A backslash before a wildcard character tells the Code editor to\n"
 "        treat that character literally, not as a wildcard.\n"
 "        For example, \\^ matches ^ and does not look for the start of a line.\n"
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

//---------------------------------------------------------------------------
void help(FILE *Out)
{
 fprintf(Out, help0);
}
//---------------------------------------------------------------------------
void HELP(FILE *Out)
{
 fprintf(Out, help0);
 fprintf(Out, help1);
 fprintf(Out, help2);
}
//---------------------------------------------------------------------------
void detab(char *str)
{
 while(str && *str)
  {
   if (*str == '\t') *str++ = ' ';
   else str++;
  }
}
//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
 FILE *Out = stdout;
 FILE *Err = stderr;
 int i;
 int f = 0;
 int h = 0;
 int H = 0;
 bool tabs=false;

 if (argc < 3)
  {
   fprintf(Err, "srtmp [-t] -f template.txt input.txt [+]output.txt\n");
   fprintf(Err, "Template search and replace utility (sed/awk inspired)\n");
   fprintf(Err, "(C) Dmitry Orlov, dimorlus@gmail.com, 2017, 19\n");
   fprintf(Err, "srtmp -h|H for help\n");
  }
 for(i=1; i < argc; i++)
  {
   if ((argv[i][0] == '-') && (argv[i][2] == '\0')) // -?
    {
     if (argv[i][1] == 'f') f = i;
     if (argv[i][1] == 'h') h = i;
     if (argv[i][1] == '?') h = i;
     if (argv[i][1] == 'H') H = i;
     if (argv[i][1] == 't') tabs = true;
    }
  }

 if (h) help(Out);
 else
 if (H) HELP(Out);
 else
 if (argc > 3)
  {
   char* rulename=NULL;
   char* listname=NULL;
   if ((f > 0) && (f < 4))
    {
     rulename = argv[f+1];
     listname = argv[f+2];
    }
   else
    {
     help(Err);
     return 1;
    }
   if (argc > 4)
    {
     if (argv[4][0] == '+')
      {
       if (argv[4][1] == '+') Out = fopen(&argv[4][1], "w+b"); // ++name -> +name
       else Out = fopen(&argv[4][1], "a+b"); // +name -> append name
      }
     else Out = fopen(argv[4], "w+b");
    }

   if (Out)
    {
     rules *Rules = new rules(rulename);
     if (Rules && (Rules->error() == erOk))
      {
       fprintf(Err, "%d rules loaded\n", Rules->rulnum());
       char line[LINELEN];
       FILE *listfile = fopen(listname, "r+t");
       if (listfile)
        {
         Rules->fheader(Out);
         while (fgets(line, sizeof(line), listfile))
          {
           if (!tabs) detab(line);
           Rules->fline(line, Out);
          }
         Rules->ffooter(Out);
         fclose(listfile);
         fprintf(Err, "%d matches in %d lines processed\n", Rules->matchnum(),
           Rules->linenum());
        }
       else fprintf(Err, "file %s not found", listname);
      }
     else
      {
       fprintf(Err, "rules load error");
       if (Rules)
        switch (Rules->error())
         {
          case erOk:
           fprintf(Err, ".\n");
          break;
          case erRegErr:
           fprintf(Err, ": regular expression error: %s.", Rules->regerror());
          break;
          case erSectionSize:
           fprintf(Err, ": section too big.\n");
          break;
          case erRulesCount:
           fprintf(Err, ": too many rules.\n");
          break;
          case erIO:
           fprintf(Err, ": file not found.\n");
          break;
         }
      }
     delete Rules;
     fclose(Out);
    }
   else fprintf(Err, "Can't write result file.\n");
  }
 return 0;
}
//---------------------------------------------------------------------------
