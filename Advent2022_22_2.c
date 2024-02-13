#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// = = = = = = = = = = = = = =     C u b e   h a s s l e    = = = = = = = = = = = = = =

char Field[200][202];
int SizeY;

// We divide the rectangular input grid into square boxes, arrange differently (!)
//   between the Test and the Puzzle case :

// Test:       222      Puzzle:    111222
// Test: (0)(1)222(3)   Puzzle: (0)111222
// Test:       222      Puzzle:    111222
// Test: 444555666      Puzzle:    444
// Test: 444555666(7)   Puzzle: (3)444(5)
// Test: 444555666      Puzzle:    444
// Test:       101111   Puzzle: 666777
// Test: (8)(9)101111   Puzzle: 666777(8)
// Test:       101111   Puzzle: 666777
//                      Puzzle: 999
//                      Puzzle: 999(1011)
//                      Puzzle: 999

// We will be storing a reference list to connect these squares, based on the Direction.
// Note that often there is an X/Y transpose during the crossing, sometimes not.
// We also note whether the Low end of the original square's edge coordinates,
// connects to the High end of the destination's square edge coordinates.
// Finally, we note at which side of the destination square we end up,
// and what the new direction will be.
typedef struct {
	int DestSquare;
	int NewDir;
	int HLswitch;
} tFromToSquare;
tFromToSquare Small[12][4], Large[12][4];

// We just fill in all the empirical information from paper models
void InitializeFromToSquares(void)
{
	void FillIn(tFromToSquare Table[][4], int FromSquare, int OldDir,
			int DestSquare, int NewDir, int HLswitch)
	{
		tFromToSquare* Entry = &(Table[FromSquare][OldDir]);
		Entry->DestSquare = DestSquare;
		Entry->NewDir = NewDir;
		Entry->HLswitch = HLswitch;
		// There's always a paired entry where the FromSquare/FromDir and ToSquare/ToDir are exchanged;
		// this notes the edge crossing in the opposite sense of the arrow drawn on the paper model.
		Entry = &(Table[DestSquare][(NewDir+2)%4]);
		Entry->DestSquare = FromSquare;
		Entry->NewDir = (OldDir+2)%4;
		Entry->HLswitch = HLswitch;
	}

	// ++++++++++    S m a l l     layout     +++++++++
	// Purple arrow
	FillIn(Small, 2, 2, 5, 1, 0);
	// Brown arrow
	FillIn(Small, 2, 3, 4, 1, 1);
	// Black arrow
	FillIn(Small, 2, 0,11, 2, 1);
	// Grey arrow
	FillIn(Small, 4, 2,11, 3, 1);
	// Dark-blue arrow
	FillIn(Small, 4, 1,10, 3, 1);
	// Light-blue arrow
	FillIn(Small, 5, 1,10, 0, 1);
	// Dark-green arrow
	FillIn(Small, 6, 0,11, 1, 1);

	// ++++++++++   P u z z l e    layout     +++++++++
  // Pink arrow
  FillIn(Large, 1, 3, 9, 0, 0);
  // Red arrow
  FillIn(Large, 2, 3, 9, 3, 0);
  // Light-blue arrow
  FillIn(Large, 1, 2, 6, 0, 1);
  // Dark-green arrow
  FillIn(Large, 4, 2, 6, 1, 0);
  // Light-green arrow
  FillIn(Large, 4, 0, 2, 3, 0);
  // Yellow arrow
  FillIn(Large, 2, 0, 7, 2, 1);
  // Orange arrow
  FillIn(Large, 7, 1, 9, 2, 0);
} /* InitializeFromToSquares() */

int PerformMove(int X, int Y, int Dir, int *NewX, int *NewY, int *NewDir)
{
  // First attempt to just move one step further in this Direction
  *NewX=X; *NewY=Y; *NewDir=Dir;
  switch (Dir)
  {
    case 0: ++*NewX; break;
    case 1: ++*NewY; break;
    case 2: --*NewX; break;
    case 3: --*NewY; break;
  }
  // When this move goes outside the square boxes, we need to make a 3-D Cube move
  if (Field[*NewX][*NewY] == ' ')
  {
    int FromSquare, SquareSize, SquareModuloX,SquareModuloY, NewModuloX,NewModuloY, XYtranspose;
    tFromToSquare *ToSquare;
    // Depending on the Square size, look up the appropriate FromToSquare
    if (SizeY < 100)
    {
      SquareSize = 4;
      FromSquare = (X-1)/SquareSize + 4 * ((Y-1)/SquareSize);
      ToSquare = &(Small[FromSquare][Dir]);
    }
    else
    {
      SquareSize = 50;
      FromSquare = (X-1)/SquareSize + 3 * ((Y-1)/SquareSize);
      ToSquare = &(Large[FromSquare][Dir]);
    }
    SquareModuloX = (X-1) % SquareSize; SquareModuloY = (Y-1) % SquareSize;
    *NewDir = ToSquare->NewDir;
		XYtranspose = (Dir ^ *NewDir) & 1;
    // The FromToSquare lookup allows to generically determine the new coordinates within the destination square
    switch (ToSquare->NewDir)
    {
      case 0: NewModuloX = 0;
        NewModuloY = XYtranspose ? SquareModuloX : SquareModuloY;
        if (ToSquare->HLswitch) NewModuloY = (SquareSize-1 - NewModuloY); break;
      case 1: NewModuloY = 0;
        NewModuloX = XYtranspose ? SquareModuloY : SquareModuloX;
        if (ToSquare->HLswitch) NewModuloX = (SquareSize-1 - NewModuloX); break;
      case 2: NewModuloX = SquareSize-1;
        NewModuloY = XYtranspose ? SquareModuloX : SquareModuloY;
        if (ToSquare->HLswitch) NewModuloY = (SquareSize-1 - NewModuloY); break;
      case 3: NewModuloY = SquareSize-1;
        NewModuloX = XYtranspose ? SquareModuloY : SquareModuloX;
        if (ToSquare->HLswitch) NewModuloX = (SquareSize-1 - NewModuloX); break;
    }
    // Depending on the Square size, convert these coordinates back to absolute coordinates
    if (SizeY < 100)
    {
      *NewX = (ToSquare->DestSquare%4)*SquareSize + NewModuloX + 1;
      *NewY = (ToSquare->DestSquare/4)*SquareSize + NewModuloY + 1;
    }
    else
    {
      *NewX = (ToSquare->DestSquare%3)*SquareSize + NewModuloX + 1;
      *NewY = (ToSquare->DestSquare/3)*SquareSize + NewModuloY + 1;
    }

    // Debugging
    if (1)
    printf("CUBE [X=%d,Y=%d] Dir %d with SquareSize=%d goes from Square %d to %d into [X=%d,Y=%d] Dir=%d\n",
        X,Y, Dir, SquareSize, FromSquare, ToSquare->DestSquare, *NewX,*NewY, *NewDir);
  } /* if regular continuation lands on a ' ' */
  // Perhaps the move gets stranded in front of a wall
  if (Field[*NewX][*NewY] == '#') return 0;
  return 1;
} /* PerformMove() */

// = = = = = = = = = = = = = =      i n t   M a i n ( )     = = = = = = = = = = = = = =

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

	int X,Y, SizeX=200;
	int Dir=0, NewX,NewY, NewDir;

	/******************/
	/* Initialisation */
	/******************/
	// We will be enveloping the entire field in a border of spaces, to make wraparound checking easier
	for (X=0; X<SizeX; X++)  Field[X][0] = ' ';
	InitializeFromToSquares();

	/******************/
	/* Data gathering */
	/******************/
	// Parse the input
	while (fgets(InputLine, 200, InputFile))
	{
		// Bookkeeping
		InputLineNr++;
		// Strip line ending
		while ((InputLine[strlen(InputLine)-1]=='\n')
				|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1]='\0';
		if (!strlen(InputLine))  break;  // Blank line marks the end of the field
		if (strlen(InputLine) > 200-2)
		{
			fprintf(stderr, "InputLine #%d is too long: %s\n", InputLineNr, InputLine);
			exit(2);
		}
		// Start with a border space
		Field[0][InputLineNr] = ' ';
		// Then copy the input content
		for (X=0; X<strlen(InputLine); X++)
		{
			// Sanity check
			if ((InputLine[X] != ' ') && (InputLine[X] != '.') && (InputLine[X] != '#'))
			{
				fprintf(stderr, "Unrecognized characted %c at pos %d of InputLine #%d: %s\n",
						InputLine[X], X, InputLineNr, InputLine);
				exit(2);
			}
			Field[X+1][InputLineNr] = InputLine[X];
		}
		// Fill the rest of the line with spaces (this includes the border)
		for (; X<SizeX-1; X++)  Field[X+1][InputLineNr] = ' ';
	} /* while (fgets) */
	for (X=0; X<SizeX; X++)  Field[X][InputLineNr] = ' ';
	SizeY = InputLineNr+1;

	// Determine starting position
	Y = 1; // Always start within top row
	for (X=0; X<SizeX; X++)  if (Field[X][Y] == '.') break;
	printf("%d InputLines were read, SizeY = %d, Start[X=%d,Y=%d]\n", InputLineNr, SizeY, X,Y);

	/*******************/
	/* Data processing */
	/*******************/
	char MoveLine[10000], Text[4];
	if (!(InputLen = fread(MoveLine, sizeof(char), 10000, InputFile)))
	{
		fprintf(stderr, "Could not read up to 10000 chars from last line of input\n");
		exit(2);
	}
	fclose(InputFile);
	MoveLine[InputLen] = '\0';
	while ((MoveLine[strlen(MoveLine)-1]=='\n')
			|| (MoveLine[strlen(MoveLine)-1]=='\r'))  MoveLine[strlen(MoveLine)-1]='\0';
	InputPos = 0;
	for(;;)
	{
		// Always a number, then a letter. Parse the number first and perform the moves.
		int NumLen=0, MoveNr, NrOfMoves;
		while ((MoveLine[InputPos+NumLen] >= '0') && (MoveLine[InputPos+NumLen] <= '9'))  NumLen++;
		if (NumLen > 3)
		{
			fprintf(stderr, "Not enough room for %d digits from pos %d of MoveLine\n", NumLen, InputPos);
			exit(3);
		}
		strncpy(Text, MoveLine+InputPos, NumLen);
		Text[NumLen] = '\0';
		if (1 != sscanf(Text, "%d", &NrOfMoves))
		{
			fprintf(stderr, "Could not scan %d digits from pos %d of MoveLine, Text: ++%s++\n",
					NumLen, InputPos, Text);
			exit(2);
		}
		InputPos += NumLen;
		for (MoveNr=0; MoveNr<NrOfMoves; MoveNr++)
		{
			// Find out where this move would take us, if it's at all possible
      if (!PerformMove(X,Y, Dir, &NewX,&NewY,&NewDir))
			{
				// Debugging
				if (1)
				printf("Move %d out of %d in Dir %d gets stopped short by a wall at [X=%d,Y=%d]\n",
						MoveNr+1, NrOfMoves, Dir, NewX, NewY);
				break;  /* from for(MoveNr) */
			}
			// Actually perform the move
			X=NewX; Y=NewY; Dir=NewDir;
		} /* for (MoveNr) */

		// Debugging
		if (1)
		printf("After %d moves in Dir %d, we have arrived at [X=%d,Y=%d]\n", MoveNr, Dir, X,Y);

		// Have we reached the end of the MoveLine ?
		if (!MoveLine[InputPos])  break;  /* from for(;;) */

		// Parse the rotation character and perform the turn.
		switch (MoveLine[InputPos])
		{
			case 'R': Dir++; break;
			case 'L': Dir+=3; break;
			default:
				fprintf(stderr, "Unrecognized rotation character %c at pos %d of MoveLine\n",
						MoveLine[InputPos], InputPos);
				exit(2);
		}
		Dir %= 4;

		// Debugging
		if (1)
		printf("After rotation %c, new Dir is %d\n", MoveLine[InputPos], Dir);

		InputPos++;
	} /* for(;;) */

	/*************/
	/* Reporting */
	/*************/
	printf("Finally, we have arrived at [X=%d,Y=%d] facing Dir=%d, so result is %d\n",
			X,Y, Dir, 1000*Y + 4*X + Dir);
	return 0;
}
