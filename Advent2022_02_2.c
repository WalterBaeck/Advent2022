#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int Arguments, char* Argument[])
{
	FILE* InputFile = stdin;
	char InputLine[100];
	char* InputPtr;
	int InputLineNr=0, InputPos, InputLen;

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

	typedef enum {eRock, ePaper, eScissors}  tShape;

	/*************/
	/* Operation */
	/*************/
	int TotalScore=0, ShapeScore, MatchScore;
	tShape MyShape,OppShape;

	while (fgets(InputLine, 100, InputFile))
	{
		// Bookkeeping
		InputLineNr++;
		// Strip line ending
		while ((InputLine[strlen(InputLine)-1]=='\n')
				|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1] ='\0';
		// Input validation
		if ((InputLine[0] < 'A') || (InputLine[0] > 'C'))
		{
			fprintf(stderr, "Could not scan OpponentShape in Line #%d: %s\n", InputLineNr, InputLine);
			exit(2);
		}
		if ((InputLine[2] < 'X') || (InputLine[2] > 'Z'))
		{
			fprintf(stderr, "Could not scan MyShape in Line #%d: %s\n", InputLineNr, InputLine);
			exit(2);
		}
		// Input parsing
		OppShape = (tShape)(InputLine[0] - 'A');
		MatchScore = 3 * (InputLine[2] - 'X');
		// Given the match outcome, figure out MyShape
		MyShape = (tShape)((OppShape + 2 + (InputLine[2] - 'X'))%3);
		ShapeScore = 1 + MyShape;
		// Debugging
		printf("%2d:  ShapeScore %d    MatchScore %d\n", InputLineNr, ShapeScore, MatchScore);
		// Add to the running total
		TotalScore += (ShapeScore + MatchScore);
	} /* while (fgets) */
	printf("%d InputLines were read.\n", InputLineNr);

	/****************/
	/* Finalisation */
	/****************/
	printf("TotalScore: %d\n", TotalScore);
	return 0;
}
