#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int Arguments, char* Argument[])
{
	FILE* InputFile = stdin;
	char InputLine[100];
	char* InputPtr;
	int InputLineNr=0, InputPos, InputLen;

	if (Arguments <= 2)
	{
		if (!(InputFile = fopen(Argument[1], "r")))
		{
			fprintf(stderr, "Could not open %s for reading\n", Argument[1]);
			exit(2);
		}
	}
	else if (Arguments > 2)
	{
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "%s [InputFile]\n", Argument[0]);
		fprintf(stderr, "\twith no [InputFile] argument, data will be read from stdin\n");
		exit(1);
	}

	/******************/
	/* Data gathering */
	/******************/
  typedef enum {eAdd, eMult, eSquare} tOp;
	typedef struct {
		unsigned long long Item[100];
		int NrOfItems;
    tOp Operation;
    int Operand;
		int Divisor;
    int True;
    int False;
    int NrOfTurns;
	} tMonkey;
  tMonkey Monkey[10];
  int MonkeyNr=0, NrOfMonkeys;
  int ItemNr, Target;

	// Parse the input
  int Num;
  char Text[4];
	while (fgets(InputLine, 100, InputFile))
	{
		// Strip line ending
		while ((InputLine[strlen(InputLine)-1]=='\n')
				|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1]='\0';
    // Expect input in recurring blocks of 7 predictable formats
    switch (InputLineNr%7)
    {
      case 0:
        if (strncmp(InputLine, "Monkey ", 7))
        {
          fprintf(stderr, "Expected 'Monkey ' on InputLine #%d: %s\n", InputLineNr+1, InputLine);
          exit(3);
        }
        if (1 != sscanf(InputLine+7, "%d", &Num))
        {
          fprintf(stderr, "Cannot find MonkeyNr on InputLine #%d: %s\n", InputLineNr+1, InputLine);
          exit(3);
        }
        if (Num != MonkeyNr)
        {
          fprintf(stderr, "Expected MonkeyNr %d on InputLine #%d: %s, found %d instead\n",
              MonkeyNr, InputLineNr+1, InputLine, Num);
          exit(3);
        }
        Monkey[MonkeyNr].NrOfTurns = 0;
        break;
      case 1:
        if (strncmp(InputLine, "  Starting items: ", 18))
        {
          fprintf(stderr, "Expected '  Starting items: ' on InputLine #%d: %s\n", InputLineNr+1, InputLine);
          exit(3);
        }
        for (ItemNr=0, InputPos=18;; ItemNr++)
        {
          char* Separator;
          if (Separator = strchr(InputLine+InputPos, ','))
          {
            strncpy(Text, InputLine+InputPos, (Separator - InputLine) - InputPos);
            Text[(Separator - InputLine) - InputPos] = '\0';
            if (1 != sscanf(Text, "%llu", &(Monkey[MonkeyNr].Item[ItemNr])))
            {
              fprintf(stderr, "Could not scan ItemNr %d for MonkeyNr %d on InputLine #%d: %s\n",
                  ItemNr, MonkeyNr, InputLineNr+1, InputLine);
              exit(3);
            }
            InputPos = (Separator - InputLine) + 1;
          }
          else
          {
            if (1 != sscanf(InputLine+InputPos, "%llu", &(Monkey[MonkeyNr].Item[ItemNr])))
            {
              fprintf(stderr, "Could not scan last Item for MonkeyNr %d on InputLine #%d: %s\n",
                  MonkeyNr, InputLineNr+1, InputLine);
              exit(3);
            }
            Monkey[MonkeyNr].NrOfItems = ItemNr+1;
            break;
          }
        } /* for (ItemNr) */
        break;
      case 2:
        if (strncmp(InputLine, "  Operation: new = old ", 23))
        {
          fprintf(stderr, "Expected '  Operation: new = old ' on InputLine #%d: %s\n",
              InputLineNr+1, InputLine);
          exit(3);
        }
        switch (InputLine[23])
        {
          case '+' : Monkey[MonkeyNr].Operation = eAdd;  break;
          case '*' : Monkey[MonkeyNr].Operation = eMult;  break;
          default:
          {
            fprintf(stderr, "Could not scan last Operation for MonkeyNr %d on InputLine #%d: %s\n",
                MonkeyNr, InputLineNr+1, InputLine);
            exit(3);
          }
        }
        if (!(strcmp(InputLine+25, "old")))
        {
          if (Monkey[MonkeyNr].Operation != eMult)
          {
            fprintf(stderr,
                "Operation should be '*' whenOperand='old' for MonkeyNr %d on InputLine #%d: %s\n",
                MonkeyNr, InputLineNr+1, InputLine);
            exit(3);
          }
          Monkey[MonkeyNr].Operation = eSquare;
        }
        else
        {
          if (1 != sscanf(InputLine+25, "%d", &(Monkey[MonkeyNr].Operand)))
          {
            fprintf(stderr, "Could not scan Operand for MonkeyNr %d on InputLine #%d: %s\n",
                MonkeyNr, InputLineNr+1, InputLine);
            exit(3);
          }
        }
        break;
      case 3:
        if (strncmp(InputLine, "  Test: divisible by ", 21))
        {
          fprintf(stderr, "Expected '  Test: divisible by ' on InputLine #%d: %s\n",
              InputLineNr+1, InputLine);
          exit(3);
        }
        if (1 != sscanf(InputLine+21, "%d", &(Monkey[MonkeyNr].Divisor)))
        {
          fprintf(stderr, "Could not scan Divisor for MonkeyNr %d on InputLine #%d: %s\n",
              MonkeyNr, InputLineNr+1, InputLine);
          exit(3);
        }
        break;
      case 4:
        if (strncmp(InputLine, "    If true: throw to monkey ", 29))
        {
          fprintf(stderr, "Expected '    If true: throw to monkey ' on InputLine #%d: %s\n",
              InputLineNr+1, InputLine);
          exit(3);
        }
        if (1 != sscanf(InputLine+29, "%d", &(Monkey[MonkeyNr].True)))
        {
          fprintf(stderr, "Could not scan True for MonkeyNr %d on InputLine #%d: %s\n",
              MonkeyNr, InputLineNr+1, InputLine);
          exit(3);
        }
        break;
      case 5:
        if (strncmp(InputLine, "    If false: throw to monkey ", 30))
        {
          fprintf(stderr, "Expected '    If false: throw to monkey ' on InputLine #%d: %s\n",
              InputLineNr+1, InputLine);
          exit(3);
        }
        if (1 != sscanf(InputLine+30, "%d", &(Monkey[MonkeyNr].False)))
        {
          fprintf(stderr, "Could not scan False for MonkeyNr %d on InputLine #%d: %s\n",
              MonkeyNr, InputLineNr+1, InputLine);
          exit(3);
        }
        break;
      case 6:
        if (strlen(InputLine))
        {
          fprintf(stderr, "Expected blank line after MonkeyNr %d on InputLine #%d: %s\n",
              InputLineNr+1, InputLine);
          exit(3);
        }
        MonkeyNr++;
    } /* switch (InputLineNr%7) */
		// Bookkeeping
		InputLineNr++;
	} /* while (fgets) */
	fclose(InputFile);
  NrOfMonkeys = MonkeyNr+1;
	printf("%d InputLines were read, %d monkeys found.\n", InputLineNr, NrOfMonkeys);

	/*******************/
	/* Data processing */
	/*******************/
  int AllDivisors = 1;
  for (MonkeyNr=0; MonkeyNr<NrOfMonkeys; MonkeyNr++)
    AllDivisors *= Monkey[MonkeyNr].Divisor;
  printf("Found AllDivisors=%d\n", AllDivisors);

  for (int Round=0; Round<10000; Round++)
  {
    // Debugging
    if(0)
      for (MonkeyNr=0; MonkeyNr<NrOfMonkeys; MonkeyNr++)
      {
        printf("Monkey[%d] Turns=%d Items[%d]",
            MonkeyNr, Monkey[MonkeyNr].NrOfTurns, Monkey[MonkeyNr].NrOfItems);
        if (Monkey[MonkeyNr].NrOfItems)
          printf("=%llu", Monkey[MonkeyNr].Item[0]);
        for (ItemNr=1; ItemNr<Monkey[MonkeyNr].NrOfItems; ItemNr++)
          printf(",%llu", Monkey[MonkeyNr].Item[ItemNr]);
        putchar('\n');
      }
    for (MonkeyNr=0; MonkeyNr<NrOfMonkeys; MonkeyNr++)
    {
      // Debugging
      if(0)
      {
        printf("++ Monkey[%d] Turns=%d Items[%d]",
            MonkeyNr, Monkey[MonkeyNr].NrOfTurns, Monkey[MonkeyNr].NrOfItems);
        if (Monkey[MonkeyNr].NrOfItems)
          printf("=%llu", Monkey[MonkeyNr].Item[0]);
        for (ItemNr=1; ItemNr<Monkey[MonkeyNr].NrOfItems; ItemNr++)
          printf(",%llu", Monkey[MonkeyNr].Item[ItemNr]);
        printf(" ++\n");
      }
      for (ItemNr=0; ItemNr<Monkey[MonkeyNr].NrOfItems; ItemNr++)
      {
        // Bookkeeping
        Monkey[MonkeyNr].NrOfTurns++;
        // This is the worry level of the Item the Monkey considers now
        unsigned long long Worry = Monkey[MonkeyNr].Item[ItemNr];
        // Overflow guard
        unsigned long long OldWorry = Worry;
        // First carry out the specified Operation on the Item
        switch (Monkey[MonkeyNr].Operation)
        {
          case eAdd:
            Worry += Monkey[MonkeyNr].Operand;
            break;
          case eMult:
            Worry *= Monkey[MonkeyNr].Operand;
            break;
          case eSquare:
            Worry *= Worry;
            break;
        }
        // Overflow guard
        if (Worry < OldWorry)
        {
          fprintf(stderr, "After operation %s with operand %d, OldWorry %llu led to smaller worry %llu\n",
              Monkey[MonkeyNr].Operation==eAdd ? "+" : 
              (Monkey[MonkeyNr].Operation == eMult ? "x" : "^2"),
              Monkey[MonkeyNr].Operand, OldWorry, Worry);
          exit(5);
        }
        // Reduce this number if possible
        Worry %= AllDivisors;
        // Then test against the Divisor
        if (Worry % Monkey[MonkeyNr].Divisor)
          Target = Monkey[MonkeyNr].False;
        else
          Target = Monkey[MonkeyNr].True;
        // Debugging
        if (0)
        printf("Monkey[%d] with Items[%d] gets result %llu from OldWorry %llu\n",
            Target, Monkey[Target].NrOfItems, Worry, OldWorry);
        // Add the adjusted Worry level to the end of the Item array of Target monkey
        Monkey[Target].Item[Monkey[Target].NrOfItems++] = Worry;
        if (Monkey[Target].NrOfItems >= 100)
        {
          fprintf(stderr, "Array overflow for MonkeyNr #%d\n", Target);
          exit(4);
        }
      }
      // After this monkey has thrown away all its items, it remains empty-handed
      Monkey[MonkeyNr].NrOfItems = 0;
    }
    // Debugging
    if (0)
    if ((!Round) || (Round==19) || ((Round%1000)==999))
    {
      printf("== After Round %d ==\n", Round+1);
      for (MonkeyNr=0; MonkeyNr<NrOfMonkeys; MonkeyNr++)
        printf("Monkey %d inspected items %d times.\n", MonkeyNr, Monkey[MonkeyNr].NrOfTurns);
    }
    // Debugging
    if (0)
    {
    printf("== After Round %d ==\n", Round+1);
    for (MonkeyNr=0; MonkeyNr<NrOfMonkeys; MonkeyNr++)
    {
      printf("Monkey[%d] Turns=%d Items[%d]",
          MonkeyNr, Monkey[MonkeyNr].NrOfTurns, Monkey[MonkeyNr].NrOfItems);
      if (Monkey[MonkeyNr].NrOfItems)
        printf("=%llu", Monkey[MonkeyNr].Item[0]);
      for (ItemNr=1; ItemNr<Monkey[MonkeyNr].NrOfItems; ItemNr++)
        printf(",%llu", Monkey[MonkeyNr].Item[ItemNr]);
      putchar('\n');
    }
    }
  } /* for (Round) */

	/*************/
	/* Reporting */
	/*************/
  unsigned long long Max1=0, Max2=0;
  for (MonkeyNr=0; MonkeyNr<NrOfMonkeys; MonkeyNr++)
    if (Monkey[MonkeyNr].NrOfTurns > Max2)
    {
      Max2 = Monkey[MonkeyNr].NrOfTurns;
      if (Max2 > Max1)
      {
        Max2 = Max1;
        Max1 = Monkey[MonkeyNr].NrOfTurns;
      }
    }
	printf("Found Product=%llu = %llu x %llu\n", Max1*Max2, Max1, Max2);
	return 0;
}
