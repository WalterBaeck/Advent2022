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

	/*************/
	/* Operation */
	/*************/
	int TotalPriority=0, Priority;

	while (fgets(InputLine, 100, InputFile))
	{
    // Initializing
    int Occur[52]={0};
    Priority = 0;
		// Bookkeeping
		InputLineNr++;
		// Strip line ending
		while ((InputLine[strlen(InputLine)-1]=='\n')
				|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1] ='\0';
    InputLen = strlen(InputLine);
    int InputHalf = InputLen/2;
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
      int InputIndex = InputLine[InputPos] - 'a';
      if (InputIndex < 0)
        InputIndex = 26 + InputLine[InputPos] - 'A';
      if ((InputIndex < 0) || (InputIndex >= 52))
      {
        fprintf(stderr, "Internal error: Line #%d pos #%d: %c yields InputIndex=%d\n",
            InputLineNr, InputPos, InputLine[InputPos], InputIndex);
        exit(3);
      }
      if (InputPos < InputHalf)
        Occur[InputIndex]++;
      else // InputPos >= InputHalf
      {
        if (Occur[InputIndex])
          // So this InputIndex occurs in both the 1st and the 2nd compartment.
          // It becomes the Priority for repackaging. But wait,
          //   has any other Priority already been established ?
          if ((Priority) && (Priority!=InputIndex+1))
          {
            fprintf(stderr, "While Priority=%d has already been established,\n", Priority);
            fprintf(stderr, "Line #%d pos #%d: %c with InputIndex=%d also occurs twice\n",
                InputLineNr, InputPos, InputLine[InputPos], InputIndex);
            exit(3);
          }
          else
            Priority = InputIndex + 1;  // Priority is 1-based
      }
    } /* for (InputPos) */

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
