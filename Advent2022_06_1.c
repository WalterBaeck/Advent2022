#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int Arguments, char* Argument[])
{
	FILE* InputFile = stdin;
	char InputLine[5000];
	char* InputPtr;
	int InputLineNr=0, InputPos, InputLen;

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
	if (!(InputLen = fread(InputLine, 1, 5000, InputFile)))
	{
		fprintf(stderr, "Could not read any chars from input ?\n");
		exit(2);
	}
	// Strip line ending
	while ((InputLine[InputLen-1]=='\n') || (InputLine[InputLen-1]=='\r'))
	{
		InputLine[InputLen-1] = '\0';
		InputLen--;
	}

	/*************/
	/* Operation */
	/*************/
	for (InputPos=3; InputPos<InputLen; InputPos++)
	{
		int Pos1,Pos2;
		int FoundDuplicate=0;
		for (Pos1=-3; Pos1<0; Pos1++)
			for (Pos2=-2; Pos2<=0; Pos2++)
			{
				if (Pos1 == Pos2)  continue;
				if (InputLine[InputPos+Pos1] == InputLine[InputPos+Pos2])
				{
					FoundDuplicate = 1;
					break;
				}
			}
		if (!FoundDuplicate)  break;
	} /* for (InputPos) */

	/****************/
	/* Finalisation */
	/****************/
	InputLine[InputPos+1] = '\0';
	printf("Unique quartet found after InputPos %d: %s\n", InputPos+1, InputLine+InputPos-3);
	return 0;
}
