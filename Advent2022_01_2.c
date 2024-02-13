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

	/******************/
	/* Data gathering */
	/******************/
	int ElfNr=0;
	int Elf[1000]={0};

	while (fgets(InputLine, 100, InputFile))
	{
		// Bookkeeping
		InputLineNr++;
		// Strip line ending
		while ((InputLine[strlen(InputLine)-1]=='\n')
				|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1] ='\0';
		// Blank line means Next Elf
		if (!strlen(InputLine))
		{
			ElfNr++;
			continue;
		}
		// Read this number and add it to the running total
		int Number;
		if (1 != sscanf(InputLine, "%d", &Number))
		{
			fprintf(stderr, "Could not read Number from InputLine #%d: %s\n", InputLineNr, InputLine);
			exit(1);
		}
		Elf[ElfNr] += Number;
	} /* while (fgets) */
	printf("%d InputLines were read.\n", InputLineNr);
	int NrOfElves = ElfNr+1;

	/*******************/
	/* Data processing */
	/*******************/
#define TOP 3
	int Max[TOP]={0};
	int MaxNr=TOP-1;

	// Go over all numbers, and compare with the smallest Max value
	for (ElfNr=0; ElfNr<NrOfElves; ElfNr++)
		if (Elf[ElfNr] > Max[MaxNr])
		{
			// Current value of smallest Max element can certainly be tossed, in favor of number found
			Max[MaxNr] = Elf[ElfNr];

			// Higher-up Max values can also be trumped. Exchange them with their successor.
			while (MaxNr--)
			{
				if (Max[MaxNr] > Max[MaxNr+1])  break;
				int temp = Max[MaxNr];
				Max[MaxNr] = Max[MaxNr+1];
				Max[MaxNr+1] = temp;
			}

			// Prepare for next number
			MaxNr = TOP - 1;
		}

	/*************/
	/* Reporting */
	/*************/
	printf("Maxima found:");
	int MaxSum=0;
	for (MaxNr=0; MaxNr<TOP; MaxNr++)
	{
		printf(" %d", Max[MaxNr]);
		MaxSum += Max[MaxNr];
	}
	printf("  -  with a total of %d\n", MaxSum);
	return 0;
}
