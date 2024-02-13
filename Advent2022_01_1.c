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

	/* Initialisation */
	int ElfNr=0;
	int Elf[1000]={0};

	/* Operation */
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

	/* Finalisation */
	int Max=0;
	for (ElfNr=0; ElfNr<NrOfElves; ElfNr++)
		if (Elf[ElfNr] > Max)  Max=Elf[ElfNr];
	printf("Maximum found: %d\n", Max);
	return 0;
}
