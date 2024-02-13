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

	typedef struct {
		char Content;    // '.' (empty) or '#' (Elf)
		int ProposedDir; // Only when inhabited by an Elf, the direction where it proposes to go
		int SpokenFor;   // How many adjacent Elves have proposed this cell as their target
	} tField;
	tField Field[300][300];
	int X,Y, NewX,NewY;
	int DirSeq[4] = {3, 1, 2, 0};  // NSWE in the same notation as Day22 ; we'll use -1 for "Stay here"
	int SeqNr=0, Round=0, PropDir, DirNr;

	/******************/
	/* Initialisation */
	/******************/
	// Start with an entirely blank Field
	memset(Field, 0, 300 * 300 * sizeof(tField));
	for (X=0; X<300; X++)  for (Y=0; Y<300; Y++)  Field[X][Y].Content = '.';

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
		// Copy the input content into the middle of the Field
		for (X=0; X<strlen(InputLine); X++)
		{
			// Sanity check
			if ((InputLine[X] != '.') && (InputLine[X] != '#'))
			{
				fprintf(stderr, "Unrecognized characted %c at pos %d of InputLine #%d: %s\n",
						InputLine[X], X, InputLineNr, InputLine);
				exit(2);
			}
			Field[X+111][InputLineNr+111].Content = InputLine[X];
		}
	} /* while (fgets) */
	printf("%d InputLines were read\n", InputLineNr);

	/*******************/
	/* Data processing */
	/*******************/
	for (Round=0; ; Round++)
	{
		// Direction proposals: scan the whole field for Elves and decide their proposal
		for (X=0; X<300; X++)  for (Y=0; Y<300; Y++)  if (Field[X][Y].Content == '#')
		{
			// The default is the proposal not to move at all
			Field[X][Y].ProposedDir = -1;
			// How busy is it in the neighboring cells ?
			int Crowd = 0;
			for (NewX=X-1; NewX<=X+1; NewX++)  for (NewY=Y-1; NewY<=Y+1; NewY++)
				if (Field[NewX][NewY].Content == '#')  Crowd++;
			// Is it crowded enough to propose a move ?
			if (Crowd > 1)
			{
				// Consider each of the 4 possible directions, starting from the current preferred direction
				for (DirNr=0; DirNr<4; DirNr++)
				{
					PropDir = DirSeq[(SeqNr + DirNr) % 4];
					Crowd = 0;
					switch (PropDir)
					{
						case 0: NewX = X+1;
							for (NewY=Y-1; NewY<=Y+1; NewY++)
								if (Field[NewX][NewY].Content == '#')  Crowd++;
							break;
						case 1: NewY = Y+1;
							for (NewX=X-1; NewX<=X+1; NewX++)
								if (Field[NewX][NewY].Content == '#')  Crowd++;
							break;
						case 2: NewX = X-1;
							for (NewY=Y-1; NewY<=Y+1; NewY++)
								if (Field[NewX][NewY].Content == '#')  Crowd++;
							break;
						case 3: NewY = Y-1;
							for (NewX=X-1; NewX<=X+1; NewX++)
								if (Field[NewX][NewY].Content == '#')  Crowd++;
							break;
					}
					// Have we found a favorable direction ? Then propose it.
					if (!Crowd)
					{
						// Debugging
						if (0)
						printf("[X=%d,Y=%d] proposes dir %d\n", X,Y, PropDir);

						Field[X][Y].ProposedDir = PropDir;
						switch (PropDir)
						{
							case 0: Field[X+1][Y].SpokenFor++;  break;
							case 1: Field[X][Y+1].SpokenFor++;  break;
							case 2: Field[X-1][Y].SpokenFor++;  break;
							case 3: Field[X][Y-1].SpokenFor++;  break;
						}
						break;  /* from for(DirNr) */
					}
					// If no suitable direction is found, the proposal will remain to Stay
				} /* for (DirNr) */
			} /* if (Crowd > 1) */
		} /* if Elf present in this cell */

		// Performing any moves now
		int MovesDone = 0;
		for (X=0; X<300; X++)  for (Y=0; Y<300; Y++)
			if ((Field[X][Y].Content == '#') && (Field[X][Y].ProposedDir >= 0))
			{
				// What was the intended destination cell for this Elf ?
				NewX=X; NewY=Y;
				switch (Field[X][Y].ProposedDir)
				{
					case 0: NewX = X+1;  break;
					case 1: NewY = Y+1;  break;
					case 2: NewX = X-1;  break;
					case 3: NewY = Y-1;  break;
				}
				// Are there any conflicts for the target cell ?
				if (Field[NewX][NewY].SpokenFor != 1)
				{
					if (0)
					printf("Proposed move dir %d from [X=%d,Y=%d] to [NewX=%d,NewY=%d] conflicts\n",
							Field[X][Y].ProposedDir, X,Y, NewX,NewY);
					Field[NewX][NewY].SpokenFor = 0;  // This denotes a detected conflict
				}
				else
				{
					if (0)
					printf("Performing move dir %d from [X=%d,Y=%d] to [NewX=%d,NewY=%d]\n",
							Field[X][Y].ProposedDir, X,Y, NewX,NewY);
					// Sanity check
					if ((NewX < 1) || (NewX >= 300-1) || (NewY < 1) || (NewY >= 300-1))
					{
						fprintf(stderr, "Danger: Elf moves from [X=%d,Y=%d] into [NewX=%d,NewY=%d]\n",
								X,Y, NewX,NewY);
						exit(3);
					}
					// The target cell is clear, perform the move
					Field[X][Y].Content = '.';
					Field[X][Y].SpokenFor = 0;
					Field[NewX][NewY].Content = '#';
					Field[NewX][NewY].ProposedDir = -1;
					MovesDone++;
				}
			}
		if (!MovesDone)
		{
			printf("Breaking off after Round %d because no more Moves happen.\n", Round);
			break;
		}

		// Increase the preferred direction
		SeqNr++;
		SeqNr %= 4;
	} /* for (Round) */

	/*************/
	/* Reporting */
	/*************/
	printf("Ended on Round %d\n", Round+1);

	return 0;
}
