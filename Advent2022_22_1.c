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

	char Field[200][202];
	int X,Y, SizeX=200,SizeY;
	int Dir=0, NewX,NewY;

	/******************/
	/* Initialisation */
	/******************/
	// We will be enveloping the entire field in a border of spaces, to make wraparound checking easier
	for (X=0; X<SizeX; X++)  Field[X][0] = ' ';

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
			// Find out where this move would take us, wrapping around over any blank spaces
			NewX=X; NewY=Y;
			switch (Dir)
			{
				case 0: if (Field[++NewX][Y] == ' ')
				{ for (NewX=0; NewX<SizeX; NewX++)  if (Field[NewX][Y] != ' ')  break; } break;
				case 1: if (Field[X][++NewY] == ' ')
				{ for (NewY=0; NewY<SizeY; NewY++)  if (Field[X][NewY] != ' ')  break; } break;
				case 2: if (Field[--NewX][Y] == ' ')
				{ for (NewX=SizeX-1; NewX; NewX--)  if (Field[NewX][Y] != ' ')  break; } break;
				case 3: if (Field[X][--NewY] == ' ')
				{ for (NewY=SizeY-1; NewY; NewY--)  if (Field[X][NewY] != ' ')  break; } break;
			}
			// Perhaps the move gets stranded in front of a wall
			if (Field[NewX][NewY] == '#')
			{
				// Debugging
				if (1)
				printf("Move %d out of %d in Dir %d gets stopped short by a wall at [X=%d,Y=%d]\n",
						MoveNr+1, NrOfMoves, Dir, NewX, NewY);
				break;  /* from for(MoveNr) */
			}
			// Actually perform the move
			X=NewX; Y=NewY;
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
