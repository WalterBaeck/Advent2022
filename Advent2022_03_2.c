#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int Arguments, char* Argument[])
{
	FILE* InputFile = stdin;
	char InputLine[100];
	char* InputPtr;
	int InputLineNr=0, InputPos, InputLen;

  /******************/
  /* Initialisation */
  /******************/
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

	/*************/
	/* Operation */
	/*************/
	int TotalPriority=0, Priority;
  int Occur[3][52];

	while (fgets(InputLine, 100, InputFile))
	{
    /*------- Data gathering -------*/
		// Bookkeeping
    int InputModulo = InputLineNr % 3;
    int InputIndex;
		InputLineNr++;
    // Initializing
    memset(Occur[InputModulo], 0, 52*sizeof(int));
		// Strip line ending
		while ((InputLine[strlen(InputLine)-1]=='\n')
				|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1] ='\0';
    InputLen = strlen(InputLine);
    for (InputPos=0; InputPos<InputLen; InputPos++)
    {
      // Input validation
      if (((InputLine[InputPos] < 'A') || (InputLine[InputPos] > 'Z'))
          && ((InputLine[InputPos] < 'a') || (InputLine[InputPos] > 'z')))
      {
        fprintf(stderr, "Could not scan Type in Line #%d pos #%d: %c\n",
            InputLineNr, InputPos, InputLine[InputPos]);
        exit(2);
      }
      // Input coding
      InputIndex = InputLine[InputPos] - 'a';
      if (InputIndex < 0)
        InputIndex = 26 + InputLine[InputPos] - 'A';
      if ((InputIndex < 0) || (InputIndex >= 52))
      {
        fprintf(stderr, "Internal error: Line #%d pos #%d: %c yields InputIndex=%d\n",
            InputLineNr, InputPos, InputLine[InputPos], InputIndex);
        exit(3);
      }
      Occur[InputModulo][InputIndex] = 1;
    } /* for (InputPos) */

    /*------- Data processing -------*/
    if (InputModulo != 2)  continue;
    // At the end of a group of 3 Elves, scour the 3 Occur arrays for a common Type
    Priority = 0;
    for (InputIndex=0; InputIndex<52; InputIndex++)
      if ((Occur[0][InputIndex]) && (Occur[1][InputIndex]) && (Occur[2][InputIndex]))
      {
        // So this InputIndex occurs in all 3 InputLines within this InputModulo group.
        // It becomes the Priority for badge assignment. But wait,
        //   has any other Priority already been established ?
        if ((Priority) && (Priority!=InputIndex+1))
        {
          fprintf(stderr, "While Priority=%d has already been established,\n", Priority);
          fprintf(stderr, "InputIndex=%d also occurs thrice after InputLine #%d\n",
              InputIndex, InputLineNr);
          exit(3);
        }
        else
          Priority = InputIndex + 1;  // Priority is 1-based
      }

		// Debugging
		printf("%2d:  Priority %d\n", InputLineNr, Priority);
		// Add to the running total
    TotalPriority += Priority;
	} /* while (fgets) */
	printf("%d InputLines were read.\n", InputLineNr);

	/****************/
	/* Finalisation */
	/****************/
	printf("TotalPriority: %d\n", TotalPriority);
	return 0;
}
