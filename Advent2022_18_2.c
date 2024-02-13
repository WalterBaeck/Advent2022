#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// = = = = = = = = = = = = = =       D a t a t y p e s        = = = = = = = = = = = = = =

typedef enum {eTrappedAir, eOutsideAir, eRock} tGrid;

// = = = = = = = = = = = = = =       V a r i a b l e s        = = = = = = = = = = = = = =

#define MAX_GRID 20
tGrid Grid[MAX_GRID][MAX_GRID][MAX_GRID];

int X,Y,Z;

int OutsideX[2500],OutsideY[2500],OutsideZ[2500];
int NrOfOutside=0, OutsideNr;

int DiscoveredX[2500],DiscoveredY[2500],DiscoveredZ[2500];
int NrOfDiscovered=0;

// = = = = = = = = = = = = = =       F u n c t i o n s        = = = = = = = = = = = = = =

void DiscoverTrappedAir(int myX, int myY, int myZ)
{
  if (Grid[myX][myY][myZ] == eTrappedAir)
  {
    Grid[myX][myY][myZ] = eOutsideAir;
    DiscoveredX[NrOfDiscovered] = myX;
    DiscoveredY[NrOfDiscovered] = myY;
    DiscoveredZ[NrOfDiscovered] = myZ;
    if (++NrOfDiscovered >= 2500)
    {
      fprintf(stderr, "More than 2500 Discovered cells\n");
      exit(4);
    }
  }
} /* DiscoverTrappedAir(myX, myY, myZ) */

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
  // Initially, the whole grid consists of air, uncertain if it's gonna be reachable from outside
	memset(Grid, eTrappedAir, MAX_GRID*MAX_GRID*MAX_GRID*sizeof(char));

	/******************/
	/* Data gathering */
	/******************/
	// Parse the input to fill in the Rock parts of the grid
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
    if (Grid[X][Y][Z] != eTrappedAir)
    {
      fprintf(stderr, "InputLine #%d occupies [%d,%d,%d] but this was already taken ?\n",
          InputLineNr, X, Y, Z);
      exit(3);
    }
    Grid[X][Y][Z] = eRock;
	} /* while (fgets) */
	printf("%d InputLines were read\n", InputLineNr);
	fclose(InputFile);

	/*******************/
	/* Data processing */
	/*******************/
  // We need some way of connecting all the eOutsideAir together,
  // so that islands of eTrapped (the initial default of every grid cell) remain.

  // It's tempting to start at [0,0,0] which happens to be always Air,
  // and build a connected tree within the maximum bounds of the Grid.
  // However, the risk exists that [0,0,0] belongs to a pocket of eOutsideAir that
  // is enclosed between the Grid boundaries and eRock. Further swathes of eOutsideAir
  // might not get connected in that case.

  // Therefore we use a different approach.
  // We start by scanning all of the Grid boundaries, any Air in there will be eOutsideAir.
  // We put all these Grid cells into a list, and compile a new list of adjacent eOutsideAir
  // cells. Turn after turn, we start from the list of freshly discovered eOutsideAir cells
  // and connect adjacent Air cells that were previously categorized as eTrappedAir.
  // When no further adjacent cells are encountered, the discovery process is complete.

  // Initial scan
  for (Y=0; Y<MAX_GRID; Y++) for (Z=0; Z<MAX_GRID; Z++)
  {
    // Scan all 6 side planes of the Grid cube; any Air in those cells is eOutsideAir by definition
    DiscoverTrappedAir(0,Y,Z);
    DiscoverTrappedAir(MAX_GRID-1,Y,Z);
    DiscoverTrappedAir(Y,0,Z);
    DiscoverTrappedAir(Y,MAX_GRID-1,Z);
    DiscoverTrappedAir(Y,Z,0);
    DiscoverTrappedAir(Y,Z,MAX_GRID-1);
  }

  // Now keep looking for neighbor cells
  while (NrOfDiscovered)
  {
    // The just Discovered cells now become the starting list of eOutsideAir cells
    memcpy(OutsideX, DiscoveredX, NrOfDiscovered*sizeof(int));
    memcpy(OutsideY, DiscoveredY, NrOfDiscovered*sizeof(int));
    memcpy(OutsideZ, DiscoveredZ, NrOfDiscovered*sizeof(int));
    NrOfOutside = NrOfDiscovered;
    NrOfDiscovered = 0;

    // Go over the whole list of eOutsideAir cells and look for the neighbors of each one
    for (OutsideNr=0; OutsideNr<NrOfOutside; OutsideNr++)
    {
      X=OutsideX[OutsideNr]; Y=OutsideY[OutsideNr]; Z=OutsideZ[OutsideNr];
      if (X)             DiscoverTrappedAir(X-1,Y,Z);
      if (X<MAX_GRID-1)  DiscoverTrappedAir(X+1,Y,Z);
      if (Y)             DiscoverTrappedAir(X,Y-1,Z);
      if (Y<MAX_GRID-1)  DiscoverTrappedAir(X,Y+1,Z);
      if (Z)             DiscoverTrappedAir(X,Y,Z-1);
      if (Z<MAX_GRID-1)  DiscoverTrappedAir(X,Y,Z+1);
    } /* for (OutsideNr) */
  } /* while (NrOfgDiscovered) */

	/*************/
	/* Reporting */
	/*************/
  int Surface=0;
  for (X=0; X<MAX_GRID; X++)
    for (Y=0; Y<MAX_GRID; Y++)
      for (Z=0; Z<MAX_GRID; Z++)
      {
        // Is this part of the rock ?
        if (Grid[X][Y][Z] != eRock)  continue;
        // Check X-direction
        if (!X)  Surface++; else if (Grid[X-1][Y][Z]==eOutsideAir)  Surface++;
        if (X==MAX_GRID-1)  Surface++; else if (Grid[X+1][Y][Z]==eOutsideAir)  Surface++;
        // Check Y-direction
        if (!Y)  Surface++; else if (Grid[X][Y-1][Z]==eOutsideAir)  Surface++;
        if (Y==MAX_GRID-1)  Surface++; else if (Grid[X][Y+1][Z]==eOutsideAir)  Surface++;
        // Check Z-direction
        if (!Z)  Surface++; else if (Grid[X][Y][Z-1]==eOutsideAir)  Surface++;
        if (Z==MAX_GRID-1)  Surface++; else if (Grid[X][Y][Z+1]==eOutsideAir)  Surface++;
      }
	printf("Surface is %d\n", Surface);
	return 0;
}
