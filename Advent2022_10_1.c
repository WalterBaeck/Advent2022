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
  int CycleNr = 1;
  int Register = 1;
  int SumStrength=0;

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

    // Perform the instruction
    switch (Instr)
    {
      case eNop:  CycleNr++;  break;
      case eAdd:  CycleNr+=2; Register+=Arg;  break;
    }

    // Look out for 20-fold multiples of the CycleNr
    if (((CycleNr%20) == 0) || (((CycleNr%20) == 1) && (Instr==eAdd)))
    {
      int SuperCycle = CycleNr/20;
      // Only interested in CycleNr 20, 60, 100, ...
      if (!(SuperCycle & 1))  continue;
      int CycleReport=CycleNr, RegReport=Register;
      // Was this a special case ?
      if (((CycleNr%20) == 1) && (Instr==eAdd))
      {
        CycleReport--;
        RegReport-=Arg;
      }
      printf("CycleNr %3d: Reg=%2d  =>  strength %d\n", CycleReport, RegReport,
        CycleReport * RegReport);
      SumStrength += (CycleReport * RegReport);
    }
  } /* while (fgets) */

  printf("%d inputlines read, now Cycle=%d and Reg=%d\n", InputLineNr, CycleNr, Register);
  printf("SumStrength = %d\n", SumStrength);
  return 0;
}
