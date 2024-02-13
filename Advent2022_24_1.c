#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int Arguments, char* Argument[])
{
	FILE* InputFile = stdin;
	char InputLine[200];
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

  // NSWE in the same notation as Day22 ; we'll use -1 for "Stay here"
  int Field[130][30][4];
	int X,Y, NewX,NewY, SizeX=0,SizeY=0, Dir;
	int Round, Choice;
  int PosX,PosY, NewPosX,NewPosY;
  int *TrailX,*TrailY, *NewTrailX,*NewTrailY;
  int TrailNr, NrOfTrails, NewTrailNr;
  int Covered[130][30];

	/******************/
	/* Initialisation */
	/******************/
	// Start with an entirely blank Field
	memset(Field, 0, 130 * 30 * 4 * sizeof(int));
  TrailX = (int*)malloc(4000 * sizeof(int));
  TrailY = (int*)malloc(4000 * sizeof(int));
  NewTrailX = (int*)malloc(4000 * sizeof(int));
  NewTrailY = (int*)malloc(4000 * sizeof(int));
  TrailX[0] = 0; TrailY[0] = -1; NrOfTrails = 1;

	/******************/
	/* Data gathering */
	/******************/
	// Parse the input
  char UpperLine[130], UnderLine[130];
	while (fgets(InputLine, 200, InputFile))
	{
		// Bookkeeping
		InputLineNr++;
		// Strip line ending
		while ((InputLine[strlen(InputLine)-1]=='\n')
				|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1]='\0';
    if (!SizeX)
    {
      SizeX = strlen(InputLine) - 2;
      if (SizeX >= 130)
      {
        fprintf(stderr, "No room for SizeX=%d\n", SizeX);
        exit(3);
      }
      memset(UpperLine, '#', (SizeX+2) * sizeof(char));
      UpperLine[SizeX+2] = '\0';
      strcpy(UnderLine, UpperLine);
      UpperLine[1] = '.';
      UnderLine[SizeX] = '.';
      if (strcmp(InputLine, UpperLine))
      {
        fprintf(stderr, "Expected %d+2 \'#\' characters in first line\n", SizeX);
        exit(2);
      }
      continue;
    }
    // Is this the end of the grid ?
    if (!strcmp(InputLine, UnderLine))
    {
      SizeY = InputLineNr - 2;
      break;
    }
    if (strlen(InputLine) != SizeX + 2)
    {
      fprintf(stderr, "Expected %d chars on InputLine #%d: %s\n", SizeX+2, InputLineNr, InputLine);
      exit(2);
    }
    if (InputLineNr >= 30)
    {
      fprintf(stderr, "No room for SizeY>=30\n");
      exit(3);
    }
		// Copy the input content into the Field
		for (X=0; X<SizeX; X++)
      switch (InputLine[X+1])
      {
        case '.': break;
        case '>': Field[X][InputLineNr-2][0] = 1; break;
        case 'v': Field[X][InputLineNr-2][1] = 1; break;
        case '<': Field[X][InputLineNr-2][2] = 1; break;
        case '^': Field[X][InputLineNr-2][3] = 1; break;
        default:
          fprintf(stderr, "Unrecognized char %c on pos %d of InputLine #%d\n",
              InputLine[X+1], X+1, InputLineNr);
          exit(2);
      }
	} /* while (fgets) */
	printf("%d InputLines were read\n", InputLineNr);

	/*******************/
	/* Data processing */
	/*******************/
  // Instead of the usual Depth-First-Search with cumbersome backtracking (requiring to turn back all blizzards)
  // we go for a Breadth-First-Search here, with many Trails spreading out simultaneously to cover all reachable
  // cells of the grid at a certain Round.
	for (Round=0;; Round++)
	{
    if (0)
    {
		/*************/
		/* Reporting */
		/*************/
    printf("Minute %2d\n", Round);
    printf(UpperLine); putchar('\n');
    for (Y=0; Y<SizeY; Y++)
    {
      putchar('#');
      for (X=0; X<SizeX; X++)
      {
        int Count = 0; char PropChar='.';
        for (Dir=0; Dir<4; Dir++)
          if (Field[X][Y][Dir])
          {
            Count++;
            switch (Dir)
            {
              case 0: PropChar='>'; break;
              case 1: PropChar='v'; break;
              case 2: PropChar='<'; break;
              case 3: PropChar='^'; break;
            }
          }
        if (Count > 1)  PropChar = '0'+Count;
        if ((PosX==X) && (PosY==Y))  PropChar = 'E';
        putchar(PropChar);
      } /* for (X) */
      putchar('#'); putchar('\n');
    } /* for (Y) */
    printf(UnderLine); putchar('\n');
    putchar('\n');
    } /* Reporting */

    // For all existing Trails, consider where they might move to, in this Round.
    memset(Covered, 0, 130 * 30 * sizeof(int));
    NewTrailNr = 0;
    for (TrailNr=0; TrailNr<NrOfTrails; TrailNr++)
    {
      PosX=TrailX[TrailNr]; PosY=TrailY[TrailNr];
      for (Choice=-1; Choice<4; Choice++)
      {
        // Where would this choice move us to ?
        NewPosX=PosX; NewPosY=PosY;
        switch (Choice)
        {
          case -1:              break;
          case  0:  NewPosX++;  break;
          case  1:  NewPosY++;  break;
          case  2:  NewPosX--;  break;
          case  3:  NewPosY--;  break;
        }
        // Reasons why this choice may be invalid :
        // 1. Hesitating eternally from the start position
        if ((NewPosY < 0) && (Round >= 10))  continue;
        // 2. Having reached a solution
        if ((Choice >= 0) && (NewPosX == SizeX-1) && (NewPosY == SizeY))
        {
          printf("Minimum trail found in Round %d", Round+1);
          break;
        }
        // 3. Moving into the borders
        if ((Choice >= 0) &&
            ((NewPosX < 0) || (NewPosX >= SizeX) || (NewPosY < 0) || (NewPosY >= SizeY)))  continue;
        // 4. We already reached this spot through another Trail in this same Round
        if ((Choice >= 0) && Covered[NewPosX][NewPosY])  continue;
        // 5. Bumping into a blizzard that also arrives in the target cell;
        //    this involves checking the 4 neighboring squares
        for (Dir=0; Dir<4; Dir++)
        {
          X=NewPosX; Y=NewPosY;
          switch (Dir)
          {
            case 0: if (--X < 0)  X = SizeX - 1; break;
            case 1: if (--Y < 0)  Y = SizeY - 1; break;
            case 2: if (++X >= SizeX)  X = 0; break;
            case 3: if (++Y >= SizeY)  Y = 0; break;
          }
          if (Field[X][Y][Dir])  break;
        } /* for (Dir) */
        if (Dir<4)  continue;
        // If all these criteria have been met, then this is a valid choice.
        // Register a new Trailhead, and mark this cell as Covered.
        NewTrailX[NewTrailNr]=NewPosX; NewTrailY[NewTrailNr]=NewPosY;
        if (++NewTrailNr > 4000)
        {
          fprintf(stderr, "No room for %d NewTrails\n", NewTrailNr);
          exit(3);
        }
        Covered[NewPosX][NewPosY] = 1;
      } /*  for (Choice) */
      // Did we break the loop early ? That means we reached a solution
      if (Choice < 4)  break;
    } /* for (TrailNr) */
    // Did we break the loop early ? That means we reached a solution
    if (Choice < 4)  break;

    // The old set of Trails has become useless now; replace it with the New set
    memcpy(TrailX, NewTrailX, NewTrailNr * sizeof(int));
    memcpy(TrailY, NewTrailY, NewTrailNr * sizeof(int));
    NrOfTrails = NewTrailNr;

    // Advance all blizzards
    int NewField[130][30][4];
    memset(NewField, 0, 130 * 30 * 4 * sizeof(int));
    for (X=0; X<SizeX; X++)  for (Y=0; Y<SizeY; Y++)
      for (Dir=0; Dir<4; Dir++)  if (Field[X][Y][Dir])
      {
        NewX=X; NewY=Y;
        switch (Dir)
        {
          case 0: if (++NewX >= SizeX) NewX = 0;   break;
          case 1: if (++NewY >= SizeY) NewY = 0;   break;
          case 2: if (--NewX < 0) NewX = SizeX-1;  break;
          case 3: if (--NewY < 0) NewY = SizeY-1;  break;
        }
        NewField[NewX][NewY][Dir] = 1;
      }
    // Copy into the existing field
    memcpy(Field, NewField, 130 * 30 * 4 * sizeof(int));

    // Debugging
    printf("Round %3d: found %3d Trails\n", Round+1, NrOfTrails);
	} /* for (Round) */

	return 0;
}
