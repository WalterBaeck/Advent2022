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
		MyShape  = (tShape)(InputLine[2] - 'X');
		ShapeScore = 1 + (InputLine[2] - 'X');
		// Determine match outcome
		if (OppShape == MyShape)  MatchScore = 3;
		// Winning happens by selecting the next Shape in the series
		else if (((OppShape + 1)%3) == MyShape)  MatchScore = 6;
		else  MatchScore = 0;
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
