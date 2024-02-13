#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Variations.h"

int main(int Arguments, char* Argument[])
{
	FILE* InputFile = stdin;
	char InputLine[150];
	char* InputPtr;

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

	int InputPos, InputLen=0;
	int InputLineNr=0;

	int Num[150][50];
	int StartX,StartY,  EndX,EndY;
	while (fgets(InputLine, 150, InputFile))
	{
	///////////////////////////    Get one full line of chars     ///////////////////////////
		while ((InputLine[strlen(InputLine)-1]=='\r') || (InputLine[strlen(InputLine)-1]=='\n'))
			InputLine[strlen(InputLine)-1] = '\0';
		if (!InputLen)
			InputLen = strlen(InputLine);
		for (InputPos=0; InputPos<InputLen; InputPos++)
		{
			if (InputLine[InputPos] == 'S')
			{
				Num[InputPos][InputLineNr] = 0;
				continue;
			}
			if (InputLine[InputPos] == 'E')
			{
				Num[InputPos][InputLineNr] = 25;
				StartX = InputPos;  StartY=InputLineNr;
				continue;
			}
			if ((InputLine[InputPos] < 'a') || (InputLine[InputPos] > 'z'))
			{
				fprintf(stderr, "Cannot interpret line %d character %d: %c as a digit\n",
						InputLineNr+1, InputPos, InputLine[InputPos]);
				exit(2);
			}
			if (InputLineNr >= 50)
			{
				fprintf(stderr, "No storage for more than 50 InputLines\n");
				exit(3);
			}
			if (InputPos >= 150)
			{
				fprintf(stderr, "No storage for more than 150 chars on an InputLine\n");
				exit(3);
			}
			Num[InputPos][InputLineNr] = InputLine[InputPos] - 'a';
		}
		InputLineNr++;
	} /* while (fgets) */
	printf("%d lines of length %d scanned\n", InputLineNr, InputLen);
  printf("Optimising path from Start[%d,%d] to any reachable level 'a'\n", StartX,StartY);

	///////////////////////////    Find the cheapest cost of every cell     ///////////////////////////
	int X=StartX,Y=StartY;
	int DestX, DestY;
	int Cost[150][50];
	memset(Cost, 0, 150 * 50 * sizeof(int));
	// Fake cost for starting cell
	Cost[StartX][StartY] = 1;
#define MAX_EXTRA 1000
  int ExtraX[MAX_EXTRA],ExtraY[MAX_EXTRA], NrOfExtra=0;
	int NewX[MAX_EXTRA],NewY[MAX_EXTRA], NrOfNew,NewNr;
	// Start out with just 1 cell being 'newly discovered'
	NewX[0]=StartX; NewY[0]=StartY; NrOfNew=1;

	// ++++++ Forever keep trying to find more New cells, starting from the current series of New cells ++++++
	for (; NrOfNew>0;)
	{
		for (NewNr=0; NewNr<NrOfNew; NewNr++)
		{
			X=NewX[NewNr]; Y=NewY[NewNr];
			// ------ From the current position, reach out to all adjacent cells ------
			int ChoiceNr;
			for (ChoiceNr=1; ChoiceNr<=4; ChoiceNr++)
			{
        // Determine the Dest coordinates for this Choice
        DestX=X; DestY=Y;
        switch (ChoiceNr)
        {
          case 1: DestY--; break;
          case 2: DestX++; break;
          case 3: DestY++; break;
          case 4: DestX--; break;
        }
        // ------ Consider all possible reasons to reject this Choice : ------
        // - leaving the grid
        if ((DestX < 0) || (DestX >= InputLen))     continue;
        if ((DestY < 0) || (DestY >= InputLineNr))  continue;
        // - a cheaper route to this cell was found earlier
        if ((Cost[DestX][DestY]) && (Cost[DestX][DestY] <= Cost[X][Y]+1))
          continue;
        // - this Choice requires too steep a descent
        if (Num[DestX][DestY] < Num[X][Y]-1)        continue;

        // ------ All conditions have been met : so we found a viable Extra cell ------
        // Register this as the new Cost for the adjacent cell
        //   (whether it is newly discovered, or the Cost is cheaper than before, regardless)
        Cost[DestX][DestY] = Cost[X][Y] + 1;
        // This new destination should be added to the Extra array ...
        //   .. UNLESS it's already in there, no need to repeat it
        int PrevNr;
        for (PrevNr=0; PrevNr<NrOfExtra; PrevNr++)
          if ((ExtraX[PrevNr]==DestX) && (ExtraY[PrevNr]==DestY))  break;
        if (PrevNr<NrOfExtra)  continue;
        // Not yet registered within the Extra array ? Then add it now.
        ExtraX[NrOfExtra]=DestX; ExtraY[NrOfExtra]=DestY;
        NrOfExtra++;
			} /* for (ChoiceNr) */
		} /* for (NewNr) */
    // Overflow guard
    if (NrOfExtra >= MAX_EXTRA)
    {
      fprintf(stderr, "NrOfExtra >= %d overflow\n", MAX_EXTRA);
      exit(4);
    }
		// After having gone through the whole batch of New cells, generating Extra cells,
		//   we have no further use for the current batch of New cells.
		// The Extra cells now take their place.
		memcpy(NewX, ExtraX, NrOfExtra*sizeof(int));
		memcpy(NewY, ExtraY, NrOfExtra*sizeof(int));
		NrOfNew = NrOfExtra;
    NrOfExtra = 0;
	} /* for (;NrOfExtra;) */

	// To find the best End cell, go over all cells with elevation 'a',
  //   but note that some 'valleys' exist that have remained unreachable !
  EndX=0; EndY=0;
  for (X=0; X<InputLen; X++)
    for (Y=0; Y<InputLineNr; Y++)
      if ((!Num[X][Y]) && (Cost[X][Y]) && (Cost[X][Y] < Cost[EndX][EndY]))
      { EndX=X; EndY=Y; }
  // When reporting the associated cost,
	//   don't forget to subtract the fake cost=1 of the Start cell.
	printf("Found Cost[%d,%d] = %d\n", EndX,EndY, Cost[EndX][EndY]-1);

	return 0;
}		
