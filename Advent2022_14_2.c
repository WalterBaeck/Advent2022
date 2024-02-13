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
	int MinX=300,MaxX=700,MinY=0,MaxY=0;
	int Grid[401][166];

	/******************/
	/* Initialisation */
	/******************/
	for (Y=0; Y<166; Y++)
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
		if ((BeginX < MinX) || (BeginX > MaxX) || (BeginY < MinY) || (BeginY > 163))
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
			if ((EndX < MinX) || (EndX > MaxX) || (EndY < MinY) || (EndY > 163))
			{
				fprintf(stderr, "Locations [%d,%d] outside grid on InputLine #%d: %s\n",
						EndX, EndY, InputLineNr, InputLine);
				exit(3);
			}
			if (EndY > MaxY)  MaxY=EndY;
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

	// Add a horizontal empty line below the lowest rock structure
	for (X=0; X<=MaxX-MinX; X++)
		Grid[X][MaxY+1] = '.';
	// Add a horizontal layer of rock below that
	for (X=0; X<=MaxX-MinX; X++)
		Grid[X][MaxY+2] = '#';
	MaxY += 2;
	if (MaxY > 165)
	{
		fprintf(stderr, "Extra layers beneath go out of grid: MaxY=%d\n", MaxY);
		exit(4);
	}

	/*******************/
	/* Data processing */
	/*******************/
	int SandNr=0;
	for (;; SandNr++)
	{
		// Debugging
if (1)
{
	  for (Y=0; Y<12; Y++)
		{
			for (X=188; X<=212; X++)
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

if (1)
{
		// Debugging
	printf("SandNr %d came to rest at [%d,%d]\n", SandNr+1, X+MinX, Y+MinY);
}

			// No possibility to drop down: this sand has reached a stable position
			Grid[X][Y]='o'; break;

		} /* for (;;) */
		if (!InGrid)
		{
			fprintf(stderr, "Odd, SandNr %d still managed to escape the Grid from [%d,%d]\n",
					SandNr+1, X, Y);
			exit(4);
		}

		// Check whether the input has become clogged
		if ((X == cSandX-MinX) && (Y == cSandY-MinY))  break;

	} /* for (SandNr) */

	/*************/
	/* Reporting */
	/*************/
	printf("SandNr %d clogged the input\n", SandNr+1);
	return 0;
}
