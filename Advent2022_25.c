#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// = = = = = = = = = = = = = =     C o n v e r s i o n s    = = = = = = = = = = = = = =
unsigned long long FromBase5(char* Text)
{
  unsigned long long Number = 0;
  unsigned long long Value = 1;
  int Pos;

  for (Pos=strlen(Text)-1; Pos>=0; Pos--, Value*=5)
    switch (Text[Pos])
    {
      case '=':  Number -= (2*Value);  break;
      case '-':  Number -=    Value;   break;
      case '0':                        break;
      case '1':  Number +=    Value;   break;
      case '2':  Number += (2*Value);  break;
      default:
        fprintf(stderr, "Unrecognized char %c at pos %d of Snafu %s\n", Text[Pos], Pos, Text);
        exit(2);
    }
  return Number;
} /* FromBase5() */

char* ToBase5(unsigned long long Number)
{
  char* Text = (char*)malloc(31 * sizeof(char));
  Text[30] = '\0';
  unsigned long long Digit;
  int Pos;

  for (Pos=29; Pos>=0; Pos--)
  {
    Digit = Number%5;
    //printf("Pos %2d: Number %lld Modulo5 %lld .. ", Pos, Number, Digit);
    Number /= 5;
    switch (Digit)
    {
      case 0:  Text[Pos] = '0';  break;
      case 1:  Text[Pos] = '1';  break;
      case 2:  Text[Pos] = '2';  break;
      case 3:  Text[Pos] = '=';  Number++;  break;
      case 4:  Text[Pos] = '-';  Number++;  break;
    }
    //printf("so Text[%2d]=%c and Number %lld\n", Pos, Text[Pos], Number);
    if (!Number)  break;
  }
  return Text+Pos;
} /* ToBase5() */

// = = = = = = = = = = = = = =      i n t   M a i n ( )     = = = = = = = = = = = = = =

int main(int Arguments, char* Argument[])
{
	FILE* InputFile = stdin;
	char InputLine[1000];
	char* InputPtr;
	int InputPos, InputLen=0;
	int InputLineNr=0;

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

  unsigned long long Sum = 0;
  unsigned long long Term;
  char* Snafu;

	while (fgets(InputLine, 1000, InputFile))
	{
		// Bookkeeping
		InputLineNr++;

	/******************/
	/* Data gathering */
	/******************/
		// Strip line ending
		while ((InputLine[strlen(InputLine)-1]=='\n')
				|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1]='\0';
		// Parse the input
    Term = FromBase5(InputLine);
    Snafu = ToBase5(Term);
    if (strcmp(Snafu, InputLine))
    {
      fprintf(stderr, "InputLine #%d: %s transformed to number %lld but backtransform becomes %s ?\n",
          InputLineNr, InputLine, Term, Snafu);
      exit(4);
    }
    Sum += Term;
	} /* while (fgets) */
	printf("%d InputLines were read.\n", InputLineNr);
	fclose(InputFile);

  printf("Sum is %lld or Snafu %s\n", Sum, ToBase5(Sum));

	return 0;
}
