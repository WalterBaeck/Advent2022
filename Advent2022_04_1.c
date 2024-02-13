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
	int TotalContained=0;
  int Begin1, End1, Begin2, End2;
  int BeginMax, EndMin;

	while (fgets(InputLine, 100, InputFile))
	{
		// Bookkeeping
		InputLineNr++;
    // Input reformatting: convert Hyphens to commas
    for (int HyphenNr=0; HyphenNr<2; HyphenNr++)
    {
      InputPtr = strtok((HyphenNr ? NULL : InputLine), "-");
      if (!InputPtr)
      {
        fprintf(stderr, "No Hyphen #%d found in InputLine #%d ?\n", HyphenNr, InputLineNr);
        exit(2);
      }
      // Replace the '\0' that was put by strtok() at the Hyphen position
      InputPtr[strlen(InputPtr)] = ',';
    } /* for (HyphenNr) */
    // Input gathering
    if (4 != sscanf(InputLine, "%d,%d,%d,%d", &Begin1, &End1, &Begin2, &End2))
    {
      fprintf(stderr, "Could not scan 4 boundaries from InputLine #%d: %s\n",
          InputLineNr, InputLine);
      exit(3);
    }
    // Processing
    BeginMax = (Begin2 > Begin1 ) ? Begin2 : Begin1;
    EndMin = (End2 < End1) ? End2 : End1;
    if ((Begin1 == BeginMax) && (End1 == EndMin))
    {
      printf("InputLine #%d %d-%d,%d-%d has first part fully covered by second part\n",
          InputLineNr, Begin1, End1, Begin2, End2);
      TotalContained++;
    }
    else if ((Begin2 == BeginMax) && (End2 == EndMin))
    {
      printf("InputLine #%d %d-%d,%d-%d has second part fully covered by first part\n",
          InputLineNr, Begin1, End1, Begin2, End2);
      TotalContained++;
    }
	} /* while (fgets) */
	printf("%d InputLines were read.\n", InputLineNr);

	/****************/
	/* Finalisation */
	/****************/
	printf("TotalContained: %d\n", TotalContained);
	return 0;
}
