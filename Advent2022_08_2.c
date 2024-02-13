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
	int Score, MaxScore=0;
	// Outer edges will have size=0 in one direction, so don't even consider them
	// Within a direction, the line-of-sight ends at the edge, even if the edge cell is still smaller
	for (GridY=1; GridY<Height-1; GridY++)
		for (GridX=1; GridX<Width-1; GridX++)
		{
			CurGrid = Grid[GridX][GridY];
			// Look in all 4 directions to find anything visible
			Score = 1;
			for (SideX=GridX-1; SideX>0; SideX--)     if (Grid[SideX][GridY]>=CurGrid)  break;
			Score *= (GridX-SideX);
			for (SideX=GridX+1; SideX<Width-1; SideX++)  if (Grid[SideX][GridY]>=CurGrid)  break;
			Score *= (SideX-GridX);
			for (SideY=GridY-1; SideY>0; SideY--)     if (Grid[GridX][SideY]>=CurGrid)  break;
			Score *= (GridY-SideY);
			for (SideY=GridY+1; SideY<Height-1; SideY++) if (Grid[GridX][SideY]>=CurGrid)  break;
			Score *= (SideY-GridY);
			// Debugging
			//printf("[%d][%d] : score %d\n", GridX, GridY, Score);
			if (Score > MaxScore)  MaxScore=Score;
		}

	/****************/
	/* Finalisation */
	/****************/
	printf("Found maxscore %d in the %d x %d grid\n", MaxScore, Height, Width);
	return 0;
}
