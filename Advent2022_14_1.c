#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

	int X,Y, cSandX=500,cSandY=0;
	int MinX=470,MaxX=569,MinY=0,MaxY=179;
	int Grid[100][180];

	/******************/
	/* Initialisation */
	/******************/
	for (Y=0; Y<=MaxY-MinY; Y++)
		for (X=0; X<=MaxX-MinX; X++)
			Grid[X][Y] = '.';
	Grid[cSandX-MinX][cSandY-MinY] = '+';

	/******************/
	/* Data gathering */
	/******************/
	int BeginX,EndX, BeginY,EndY, Sign;
	// Parse the input
	while (fgets(InputLine, 1000, InputFile))
	{
		// Bookkeeping
		InputLineNr++;
		// Strip line ending
		while ((InputLine[strlen(InputLine)-1]=='\n')
				|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1]='\0';
		if (2 != sscanf(InputLine, "%d,%d", &BeginX, &BeginY))
		{
			fprintf(stderr, "Could not scan BeginX,BeginY from InputLine #%d: %s\n", InputLineNr, InputLine);
			exit(3);
		}
		if ((BeginX < MinX) || (BeginX > MaxX) || (BeginY < MinY) || (BeginY > MaxY))
		{
			fprintf(stderr, "Locations [%d,%d] outside grid on InputLine #%d: %s\n",
					BeginX, BeginY, InputLineNr, InputLine);
			exit(3);
		}
		BeginX -= MinX; BeginY -= MinY;
		InputPtr = InputLine;
		while (InputPtr = strstr(InputPtr, " -> "))
		{
			InputPtr += 4;
			if (2 != sscanf(InputPtr, "%d,%d", &EndX, &EndY))
			{
				fprintf(stderr, "Could not scan EndX,EndY from InputLine #%d: %s\n", InputLineNr, InputLine);
				exit(3);
			}
			if ((EndX < MinX) || (EndX > MaxX) || (EndY < MinY) || (EndY > MaxY))
			{
				fprintf(stderr, "Locations [%d,%d] outside grid on InputLine #%d: %s\n",
						EndX, EndY, InputLineNr, InputLine);
				exit(3);
			}
			EndX -= MinX; EndY -= MinY;
			// Draw this line segment within the graph now
			if (EndY == BeginY)
			{
				Sign = (BeginX<EndX) ? 1 : -1;
				for (X=BeginX; X!=EndX; X+=Sign)  Grid[X][BeginY]='#';
				Grid[X][BeginY]='#';
			}
			else if (EndX == BeginX)
			{
				Sign = (BeginY<EndY) ? 1 : -1;
				for (Y=BeginY; Y!=EndY; Y+=Sign)  Grid[BeginX][Y]='#';
				Grid[BeginX][Y]='#';
			}
			// The End becomes the Beginning of a new segment
			BeginX=EndX; BeginY=EndY;
		} /* while (InputPtr) */
	} /* while (fgets) */
	printf("%d InputLines were read.\n", InputLineNr);
	fclose(InputFile);

	/*******************/
	/* Data processing */
	/*******************/
	int SandNr=0;
	for (;; SandNr++)
	{
		// Debugging
if (0)
{
	  for (Y=0; Y<10; Y++)
		{
			for (X=24; X<=33; X++)
				putchar(Grid[X][Y]);
			putchar('\n');
		}
}

		printf("== Sand %2d ==\n", SandNr+1);
		X=cSandX-MinX; Y=cSandY-MinY;
		// Always try to keep this unit of sand moving, without leaving the grid
		int InGrid = 1;
		for (;;)
		{
			// Drop straight down
			if (Y+1 > MaxY-MinY)      { InGrid=0; break; }  // leaving the grid
			if (Grid[X][Y+1]=='.')    { Y++;      continue; }
			// Drop diagonally left-down
			if (X-1 < 0)              { InGrid=0; break; }  // leaving the grid
			if (Grid[X-1][Y+1]=='.')  { X--; Y++; continue; }
			// Drop diagonally right-down
			if (X+1 > MaxX-MinX)      { InGrid=0; break; }  // leaving the grid
			if (Grid[X+1][Y+1]=='.')  { X++; Y++; continue; }

if (0)
{
		// Debugging
	printf("SandNr %d came to rest at [%d,%d]\n", SandNr+1, X+MinX, Y+MinY);
}

			// No possibility to drop down: this sand has reached a stable position
			Grid[X][Y]='o'; break;
		} /* for (;;) */
		if (!InGrid)      break;
	} /* for (SandNr) */

	/*************/
	/* Reporting */
	/*************/
	printf("SandNr %d fell out of the grid\n", SandNr);
	return 0;
}
