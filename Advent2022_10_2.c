#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int Arguments, char* Argument[])
{
  FILE* InputFile = stdin;
  int InputLen, InputPos, InputLineNr;
  char InputLine[100];

  if (Arguments == 2)
  {
    if (!(InputFile = fopen(Argument[1], "r")))
    {
      fprintf(stderr, "Cannot open %s for reading\n", Argument[1]);
      exit(1);
    }
  }

  typedef enum {eNop, eAdd} tInstr;
  tInstr Instr;
  int Arg;
  int CycleNr = 0;
  int Register = 1;

  while (fgets(InputLine, 100, InputFile))
  {
    // Bookkeeping
    InputLineNr++;
    // Strip newline from the end
    while ((InputLine[strlen(InputLine)-1]=='\n') ||
        (InputLine[strlen(InputLine)-1]=='\r'))
      InputLine[strlen(InputLine)-1] = '\0';
    // Parse input
    if (!strcmp(InputLine, "noop"))
      Instr = eNop;
    else if (!strncmp(InputLine, "addx ", 5))
    {
      Instr = eAdd;
      if (1 != sscanf(InputLine+5, "%d", &Arg))
      {
        fprintf(stderr, "Cannot read addx argument from InputLine #%d: %s\n",
            InputLineNr, InputLine);
        exit(2);
      }
    }
    else
    {
      fprintf(stderr, "Unrecognized instruction on InputLine #%d: %s\n",
          InputLineNr, InputLine);
      exit(2);
    }

    // Prefetch the instruction
    int Duration=1;
    if (Instr==eAdd)  Duration=2;

    // Display pixel
    for (int Cycle=0; Cycle<Duration; Cycle++)
    {
      int ColNr = CycleNr%40;
      // Goto next scanline when ColNr rolls over to zero
      if (!ColNr)  putchar('\n');
      if ((ColNr>=Register-1) && (ColNr<=Register+1))
        putchar('#');
      else
        putchar('.');
      CycleNr++;
    }

    // Execute the instruction
    if (Instr==eAdd)  Register+=Arg;
  } /* while (fgets) */

  printf("\n%d inputlines read, now Cycle=%d and Reg=%d\n", InputLineNr, CycleNr, Register);
  return 0;
}
