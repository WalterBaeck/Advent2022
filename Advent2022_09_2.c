#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int Arguments, char* Argument[])
{
	FILE* InputFile = stdin;
	char InputLine[100];
	char* InputPtr;
	int InputLineNr=0, InputPos, InputLen;

	/******************/
	/* Initialisation */
	/******************/
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

#define SIZE 1000
	char **Grid;
	if (!(Grid = (char**)malloc(SIZE*sizeof(char*))))
	{
		fprintf(stderr, "Could not allocate %d charptrs for Grid\n", SIZE);
		exit(4);
	}
	int Col;
	for (Col=0; Col<SIZE; Col++)
	{
		if (!(Grid[Col] = (char*)malloc(SIZE*sizeof(char))))
		{
			fprintf(stderr, "Could not allocate %d chars for Grid[%d]\n", SIZE, Col);
			exit(4);
		}
		memset(Grid[Col], 0, SIZE*sizeof(char));
	}
	int X[10],Y[10], Piece;
	for (Piece=0; Piece<10; Piece++)  { X[Piece]=SIZE/2;  Y[Piece]=SIZE/2; }
	Grid[SIZE/2][SIZE/2] = 1;

	/*************/
	/* Operation */
	/*************/
	char MoveDir;
	int MoveLen, MoveStep;
	int XH,YH, XT,YT;
	int DistX,DistY, AbsDistX,AbsDistY, SignDistX,SignDistY;

	// Read every move from the input
	while (fgets(InputLine, 100, InputFile))
	{
		// Bookkeeping
		InputLineNr++;
		// Strip line ending
		while ((InputLine[strlen(InputLine)-1]=='\n')
				|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1]='\0';
		if (2 != sscanf(InputLine, "%c %d", &MoveDir, &MoveLen))
		{
			fprintf(stderr, "Could not read MoveDir and MoveLen from InputLine #%d: %s\n",
					InputLineNr, InputLine);
			exit(3);
		}
		// Debugging
		printf("== %c %d ==   ", MoveDir, MoveLen);
		// Now simulate this full Move in single steps
		for (MoveStep=0; MoveStep<MoveLen; MoveStep++)
		{
			XH = X[0];  YH = Y[0];
			switch (MoveDir)
			{
				case 'U':  YH--;  break;
				case 'R':  XH++;  break;
				case 'D':  YH++;  break;
				case 'L':  XH--;  break;
				default:
					fprintf(stderr, "Unrecognized MoveDir %c on InputLine #%d\n", MoveDir, InputLineNr);
					exit(3);
			} /* switch (MoveDir) */
			// Grid boundaries check
			if (YH >= SIZE) { fprintf(stderr, "InputLine #%d makes Head go to Y=%d\n", InputLineNr, YH); exit(4); }
			if (YH < 0)     { fprintf(stderr, "InputLine #%d makes Head go to Y=%d\n", InputLineNr, YH); exit(4); }
			if (XH >= SIZE) { fprintf(stderr, "InputLine #%d makes Head go to X=%d\n", InputLineNr, XH); exit(4); }
			if (XH < 0)     { fprintf(stderr, "InputLine #%d makes Head go to X=%d\n", InputLineNr, XH); exit(4); }
			X[0] = XH;  Y[0] = YH;

			// After every single step within a Move that the Head makes, all further segments can react
			for (Piece=0; Piece<9; Piece++)
			{
				// For this segment, the 'Head' is numbered 'Piece' and the 'Tail' is numbered 'Piece+1'
				XH = X[Piece];  YH = Y[Piece];
				XT = X[Piece+1];  YT = Y[Piece+1];
				// Now perform the same creeping-up motion as for a rope of only 1 segment
				DistX = XH - XT;   DistY = YH - YT;
				if (DistX < 0) { AbsDistX = -DistX;  SignDistX = -1; } else { AbsDistX = DistX;  SignDistX = 1; }
				if (DistY < 0) { AbsDistY = -DistY;  SignDistY = -1; } else { AbsDistY = DistY;  SignDistY = 1; }
				if (!DistX)  SignDistX = 0;  if (!DistY)  SignDistY = 0;
				// Only when the Tail remains touching the Head, does no movement happen at all
				if ((AbsDistX < 2) && (AbsDistY < 2))  { SignDistX = 0;  SignDistY = 0; }
				// Perform the Tail move and mark the Grid cell where it ends up
				XT += SignDistX;   YT += SignDistY;
				// Put the coordinates back into the segment array
				X[Piece+1] = XT;  Y[Piece+1] = YT;
			}
			// The very last segment is the actual Tail of the entire rope, track it
			Grid[XT][YT] = 1;
			// Debugging
			putchar(' ');
			if (SignDistY>0)  putchar('d');  else if (SignDistY<0)  putchar('u');
			if (SignDistX>0)  putchar('r');  else if (SignDistX<0)  putchar('l');
		} /* for (MoveStep) */
		// Debugging
		printf("  Tail[%d][%d]\n", XT-SIZE/2, YT-SIZE/2);
	} /* while (fgets) */
	printf("%d InputLines were read.\n", InputLineNr);
	fclose(InputFile);

	/****************/
	/* Finalisation */
	/****************/
	int Num=0;
	for (YT=0; YT<SIZE; YT++)
		for (XT=0; XT<SIZE; XT++)
			if (Grid[XT][YT])  Num++;
	printf("Found #VisitedByTail=%d\n", Num);
	return 0;
}
