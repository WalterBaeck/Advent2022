#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int Arguments, char* Argument[])
{
	FILE* InputFile = stdin;
	char InputLine[200];
	char* InputPtr;
	int InputLineNr=0, InputPos, InputLen=0;

	if (Arguments == 2)
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
	int Width=0,Height;
	int Grid[100][100];
	while (fgets(InputLine, 200, InputFile))
	{
		// Bookkeeping
		InputLineNr++;
		// Strip line ending
		while ((InputLine[strlen(InputLine)-1]=='\n')
				|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1] ='\0';
		if (!Width)
			Width = strlen(InputLine);
		if (Width>100)
		{
			fprintf(stderr, "Cannot handle InputLines>100 digits\n");
			exit(3);
		}
		else if (Width != strlen(InputLine))
		{
			fprintf(stderr, "InputLine #%d has length %d but Width=%d was found earlier\n",
					InputLineNr, strlen(InputLine), Width);
			exit(3);
		}
		for (InputPos=0; InputPos<Width; InputPos++)
		{
			// Sanity check
			if ((InputLine[InputPos]<'0') || (InputLine[InputPos]>'9'))
			{
				fprintf(stderr, "Cannot handle char '%c' at pos %d of InputLine #%d\n",
						InputLine[InputPos], InputPos, InputLineNr);
				exit(3);
			}
			Grid[InputPos][InputLineNr-1] = InputLine[InputPos]-'0';
		} /* for (InputPos) */
	} /* while (fgets()) */
	if ((Height = InputLineNr) > 100)
	{
		fprintf(stderr, "Cannot handle InputFile>100 lines\n");
		exit(3);
	}

	/*************/
	/* Operation */
	/*************/
	int GridX,GridY, CurGrid, SideX, SideY;
	// Outer edges are always fully visible
	int SumVisible = 2 * (Width-1 + Height-1);
	for (GridY=1; GridY<Height-1; GridY++)
		for (GridX=1; GridX<Width-1; GridX++)
		{
			CurGrid = Grid[GridX][GridY];
			// Look in all 4 directions to find anything to hide behind
			// Any direction that offers a view to CurGrid is sufficient to make this cell visible
			int Hidden=0;
			for (SideX=0; SideX<GridX; SideX++)        if (Grid[SideX][GridY]>=CurGrid)  Hidden=1;
			if (!Hidden) { SumVisible++;  continue; }
			Hidden=0;
			for (SideX=GridX+1; SideX<Width; SideX++)  if (Grid[SideX][GridY]>=CurGrid)  Hidden=1;
			if (!Hidden) { SumVisible++;  continue; }
			Hidden=0;
			for (SideY=0; SideY<GridY; SideY++)        if (Grid[GridX][SideY]>=CurGrid)  Hidden=1;
			if (!Hidden) { SumVisible++;  continue; }
			Hidden=0;
			for (SideY=GridY+1; SideY<Height; SideY++) if (Grid[GridX][SideY]>=CurGrid)  Hidden=1;
			if (!Hidden) { SumVisible++;  continue; }
		}

	/****************/
	/* Finalisation */
	/****************/
	printf("Found %d visible trees in the %d x %d grid\n", SumVisible, Height, Width);
	return 0;
}
