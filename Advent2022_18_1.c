#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// = = = = = = = = = = = = = =       D a t a t y p e s        = = = = = = = = = = = = = =


// = = = = = = = = = = = = = =       V a r i a b l e s        = = = = = = = = = = = = = =

#define MAX_GRID 20
char Grid[MAX_GRID][MAX_GRID][MAX_GRID];

int X,Y,Z;

// = = = = = = = = = = = = = =       F u n c t i o n s        = = = = = = = = = = = = = =


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

	/******************/
	/* Initialisation */
	/******************/
	memset(Grid, 0, MAX_GRID*MAX_GRID*MAX_GRID*sizeof(char));

	/******************/
	/* Data gathering */
	/******************/
	// Parse the input
	while (fgets(InputLine, 1000, InputFile))
	{
		// Bookkeeping
		InputLineNr++;
		// Strip line ending
		while ((InputLine[strlen(InputLine)-1]=='\n')
				|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1]='\0';
    if (3 != sscanf(InputLine, "%d,%d,%d", &X, &Y, &Z))
    {
      fprintf(stderr, "Cannot scan 3 coordinates from InputLine #%d: %s\n", InputLineNr, InputLine);
      exit(2);
    }
    if ((X<0) || (X>=MAX_GRID))
    { fprintf(stderr, "X=%d outside Grid on InputLine #%d\n", X, InputLineNr); exit(3); }
    if ((Y<0) || (Y>=MAX_GRID))
    { fprintf(stderr, "Y=%d outside Grid on InputLine #%d\n", Y, InputLineNr); exit(3); }
    if ((Z<0) || (Z>=MAX_GRID))
    { fprintf(stderr, "Z=%d outside Grid on InputLine #%d\n", Z, InputLineNr); exit(3); }
    // Mark this Grid coordinate as taken
    if (Grid[X][Y][Z])
    {
      fprintf(stderr, "InputLine #%d occupies [%d,%d,%d] but this was already taken ?\n",
          InputLineNr, X, Y, Z);
      exit(3);
    }
    Grid[X][Y][Z] = 1;
	} /* while (fgets) */
	printf("%d InputLines were read\n", InputLineNr);
	fclose(InputFile);

	/*******************/
	/* Data processing */
	/*******************/
  int Surface=0;
  for (X=0; X<MAX_GRID; X++)
    for (Y=0; Y<MAX_GRID; Y++)
      for (Z=0; Z<MAX_GRID; Z++)
      {
        // Is this part of the rock ?
        if (!Grid[X][Y][Z])  continue;
        // Check X-direction
        if (!X)  Surface++; else if (!Grid[X-1][Y][Z])  Surface++;
        if (X==MAX_GRID-1)  Surface++; else if (!Grid[X+1][Y][Z])  Surface++;
        // Check Y-direction
        if (!Y)  Surface++; else if (!Grid[X][Y-1][Z])  Surface++;
        if (Y==MAX_GRID-1)  Surface++; else if (!Grid[X][Y+1][Z])  Surface++;
        // Check Z-direction
        if (!Z)  Surface++; else if (!Grid[X][Y][Z-1])  Surface++;
        if (Z==MAX_GRID-1)  Surface++; else if (!Grid[X][Y][Z+1])  Surface++;
      }

	/*************/
	/* Reporting */
	/*************/
	printf("Surface is %d\n", Surface);
	return 0;
}
