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
	int NrOfStacks=0, StackNr;
	int TopDownStack[10][10];
	int DownTopStack[10][50];
	int Height[10], HeightAssigned[10]={0}, MaxHeight;
	char cNumberingLine[100]={0};

	while (fgets(InputLine, 100, InputFile))
	{
		// Bookkeeping
		InputLineNr++;
		// Strip line ending
		while ((InputLine[strlen(InputLine)-1]=='\n')
				|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1] ='\0';
		if (!NrOfStacks)
		{
			// Regardless of occupations, chars are provided for all stacks at maximum height.
			// This even applies to the numbering line, below the stack contents, before blank line
			NrOfStacks = (strlen(InputLine)+1)/4;
			// Now the contents of the numbering line can be exactly predicted
			for (StackNr=0; StackNr<NrOfStacks; StackNr++)
			{
				if (StackNr)  strcat(cNumberingLine, " ");
				char Text[3]={0};
				sprintf(Text, "%2d ", StackNr+1);  // Stack numbering is 1-based
				strcat(cNumberingLine, Text);
			} /* for (StackNr) */
			printf("Expecting this numbering line: ++%s++\n", cNumberingLine);
		}
		else if (NrOfStacks != (strlen(InputLine)+1)/4)
		{
			fprintf(stderr, "Anomaly: InputLine #%d does not conform to earlier seen NrOfStacks=%d\n",
					InputLineNr, NrOfStacks);
			exit(3);
		}
		if (!strcmp(InputLine, cNumberingLine))
		{
			MaxHeight = InputLineNr-1;
			break;
		}
		InputPos = 0;
		for (StackNr=0; StackNr<NrOfStacks; StackNr++)
		{
			if (StackNr)
			{
				if (InputLine[InputPos] != ' ')
				{
					fprintf(stderr, "Space expected iso. %c as stack separator on pos #%d of InputLine #%d: %s\n",
							InputLine[InputPos], InputPos, InputLineNr, InputLine);
					exit(3);
				}
				InputPos++;
			}
			if (!strncmp(InputLine+InputPos, "   ", 3))
			{
				// Empty stack content detected - previous content, if exists, should also be empty
				if ((InputLineNr>=2) && (TopDownStack[StackNr][InputLineNr-2] != ' '))
				{
					fprintf(stderr, "Space detected for TopDownStack[%d][%d] on pos #%d of InputLine #%d: %s\n",
							StackNr, InputLineNr, InputPos, InputLineNr, InputLine);
					fprintf(stderr, "  but previous content was %c at TopDownStack[%d][%d] ?\n",
							TopDownStack[StackNr][InputLineNr-1], StackNr, InputLineNr-1);
					exit(3);
				}
				TopDownStack[StackNr][InputLineNr-1] = ' ';
				InputPos += 3;
				continue;
			}
			// Not empty : should follow stack notation
			if (InputLine[InputPos] != '[')
			{
				fprintf(stderr, "Opening brace expected expected iso. %c on pos #%d of InputLine #%d: %s\n",
						InputLine[InputPos], InputPos, InputLineNr, InputLine);
				exit(3);
			}
			InputPos++;
			if ((InputLine[InputPos]<'A') || (InputLine[InputPos]>'Z'))
			{
				fprintf(stderr, "Capital letter expected expected iso. %c on pos #%d of InputLine #%d: %s\n",
						InputLine[InputPos], InputPos, InputLineNr, InputLine);
				exit(3);
			}
			TopDownStack[StackNr][InputLineNr-1] = InputLine[InputPos];
			InputPos++;
			if (InputLine[InputPos] != ']')
			{
				fprintf(stderr, "Closing brace expected expected iso. %c on pos #%d of InputLine #%d: %s\n",
						InputLine[InputPos], InputPos, InputLineNr, InputLine);
				exit(3);
			}
			InputPos++;
		} /* for (StackNr) */
	} /* while (fgets) */
	
	// Stack contents and numbering line have been parsed.
	// Now expect the blank line as separator from the Moves section.
	InputLineNr++;
	if (!fgets(InputLine, 100, InputFile))
	{
		fprintf(stderr, "Cannot input lines after numbering line ? InputLineNr=%d\n", InputLineNr);
		exit(3);
	}
	// Strip line ending
	while ((InputLine[strlen(InputLine)-1]=='\n')
			|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1] ='\0';
	if (strlen(InputLine))
	{
		fprintf(stderr, "Expected blank line after numbering line ? InputLine #%d: %s\n", InputLineNr, InputLine);
		exit(3);
	}

	// Construct the DownTopStacks, now that all contents are known
	int RowNr;
	for (StackNr=0; StackNr<NrOfStacks; StackNr++)
	{
		Height[StackNr] = 0;
		for (RowNr=0; RowNr<MaxHeight; RowNr++)
		{
			DownTopStack[StackNr][RowNr] = TopDownStack[StackNr][MaxHeight-1-RowNr];
			if ((!HeightAssigned[StackNr]) && (DownTopStack[StackNr][RowNr]==' '))
			{
				Height[StackNr] = RowNr;
				HeightAssigned[StackNr] = 1;
			}
		}
	}
	for (StackNr=0; StackNr<NrOfStacks; StackNr++)
		if (!HeightAssigned[StackNr])
			Height[StackNr] = MaxHeight;
	printf("%d Stacks of MaxHeight %d have been parsed.\n", NrOfStacks, MaxHeight);

	// Debugging
	if (1)
	{
		printf("%s\n", cNumberingLine);
		for (RowNr=0; RowNr<MaxHeight; RowNr++)
		{
			for (StackNr=0; StackNr<NrOfStacks; StackNr++)
			{
				if (StackNr)  putchar(' ');
				if (DownTopStack[StackNr][RowNr] == ' ')
					printf("   ");
				else
					printf("[%c]", DownTopStack[StackNr][RowNr]);
			}
			putchar('\n');
		}
		for (StackNr=0; StackNr<NrOfStacks; StackNr++)
			printf("#%d  ", Height[StackNr]);
		putchar('\n');
	} /* if (Debug) */

	/*******************/
	/* Data processing */
	/*******************/
	int MoveNr=0;
	int CrateNr, NrOfCrates;
	int FromStack, ToStack;
	while (fgets(InputLine, 100, InputFile))
	{
		// Bookkeeping
		InputLineNr++;
		MoveNr++;
		// Strip line ending
		while ((InputLine[strlen(InputLine)-1]=='\n')
				|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1] ='\0';
		// Parse this move input
		if (3 != sscanf(InputLine, "move %d from %d to %d", &NrOfCrates, &FromStack, &ToStack))
		{
			fprintf(stderr, "Could not scan 3 numbers from InputLine #%d: %s\n", InputLineNr, InputLine);
			exit(3);
		}
		FromStack--; ToStack--;  // Stack numbering is 1-based

		// Sanity check
		if (ToStack == FromStack)
		{
			fprintf(stderr, "Move #%d from InputLine #%d has equal FromStack = ToStack = %d ?\n",
					MoveNr, InputLineNr, FromStack+1);
			exit(3);
		}
		if (Height[FromStack] < NrOfCrates)
		{
			fprintf(stderr, "Move #%d from InputLine #%d specifies %d Crates but Height[%d] is only %d ?\n",
					MoveNr, InputLineNr, NrOfCrates, FromStack+1, Height[FromStack]);
			exit(3);
		}
		if (Height[ToStack] + NrOfCrates > 50)
		{
			fprintf(stderr, "Move #%d from InputLine #%d specifies %d Crates but Height[%d] is already %d \n",
					MoveNr, InputLineNr, NrOfCrates, ToStack+1, Height[ToStack]);
			exit(3);
		}

		// Perform this Move
		for (CrateNr=0; CrateNr<NrOfCrates; CrateNr++)
		{
			DownTopStack[ToStack][Height[ToStack] + CrateNr] = DownTopStack[FromStack][Height[FromStack]-1-CrateNr];
			DownTopStack[FromStack][Height[FromStack]-1-CrateNr] = ' ';
		}
		Height[FromStack] -= NrOfCrates;
		Height[ToStack] += NrOfCrates;
	} /* while (fgets) */
	printf("Parsed %d Moves\n", MoveNr);
	printf("%d InputLines were read.\n", InputLineNr);
	fclose(InputFile);

	/*************/
	/* Reporting */
	/*************/
	char TopString[100]={0};
	for (StackNr=0; StackNr<NrOfStacks; StackNr++)
		TopString[StackNr] = DownTopStack[StackNr][Height[StackNr]-1];
	TopString[StackNr] = '\0';
	printf("TopString found: %s", TopString);
	return 0;
}
