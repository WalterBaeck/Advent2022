#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int Arguments, char* Argument[])
{
	FILE* InputFile = stdin;
	char InputLine[100];
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
		char Name[5];
		long long Value;
		int Known;
		char Operator;
		char OperandName[2][5];
		int OperandIndex[2];
		int FromHuman;
  } tMonkey;
  tMonkey Monkey[2000];

  int MonkeyNr=0,NrOfMonkeys;
  int RootIndex=-1, OperandNr, SearchNr;

	/******************/
	/* Initialisation */
	/******************/

	/******************/
	/* Data gathering */
	/******************/
	// Parse the input
	while (fgets(InputLine, 100, InputFile))
	{
		// Bookkeeping
		InputLineNr++;
		// Strip line ending
		while ((InputLine[strlen(InputLine)-1]=='\n')
				|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1]='\0';
		if (strlen(InputLine) < 6)
		{
			fprintf(stderr, "InputLine #%d is too short: %s\n", InputLineNr, InputLine);
			exit(2);
		}
		if (InputLine[4] != ':')
		{
			fprintf(stderr, "Expected colon at pos 4 of InputLine #%d: %s\n", InputLineNr, InputLine);
			exit(2);
		}
		strncpy(Monkey[MonkeyNr].Name, InputLine, 4);
		Monkey[MonkeyNr].Name[4] = '\0';
		if (!strcmp(Monkey[MonkeyNr].Name, "root"))
		{
			printf("\"root\" found as Monkey #%d\n", MonkeyNr);
			RootIndex = MonkeyNr;
		}
		if (!strcmp(Monkey[MonkeyNr].Name, "humn"))
			Monkey[MonkeyNr].FromHuman = 1;
		else
			Monkey[MonkeyNr].FromHuman = 0;
		// Is this a literal ? There are no negative numbers, so these always start with a digit
		if ((InputLine[6] >= '0') && (InputLine[6] <= '9'))
		{
			if (1 != sscanf(InputLine+6, "%lld", &Monkey[MonkeyNr].Value))
			{
				fprintf(stderr, "Could not scan literal value on InputLine #%d: %s\n", InputLineNr, InputLine);
				exit(2);
			}
			Monkey[MonkeyNr].Known = 1;
		}
		else
		{
			// Not a literal: parse two operand names and an operator
			if (strlen(InputLine) < 17)
			{
				fprintf(stderr, "InputLine #%d is too short: %s\n", InputLineNr, InputLine);
				exit(2);
			}
			if ((InputLine[11] != '+') && (InputLine[11] != '-') && (InputLine[11] != '*') && (InputLine[11] != '/'))
			{
				fprintf(stderr, "Expected operator instead of %c at pos 11 of InputLine #%d: %s\n",
						InputLine[11], InputLineNr, InputLine);
				exit(2);
			}
			Monkey[MonkeyNr].Operator = InputLine[11];
			for (OperandNr=0; OperandNr<2; OperandNr++)
			{
				strncpy(Monkey[MonkeyNr].OperandName[OperandNr], InputLine + 6 + OperandNr*7, 4);
				Monkey[MonkeyNr].OperandName[OperandNr][4] = '\0';
				Monkey[MonkeyNr].OperandIndex[OperandNr] = -1;
			}
			Monkey[MonkeyNr].Known = 0;
		}
    MonkeyNr++;
	} /* while (fgets) */
	NrOfMonkeys = MonkeyNr;
	printf("%d InputLines were read for %d Monkeys.\n", InputLineNr, NrOfMonkeys);
	fclose(InputFile);
  if (RootIndex < 0)
  {
    fprintf(stderr, "\"root\" monkey not found in input dataset.\n");
    exit(2);
  }

	// Now that all monkey names are known, go over the whole list again to fill in reference indices
	for (MonkeyNr=0; MonkeyNr<NrOfMonkeys; MonkeyNr++)
		if (!Monkey[MonkeyNr].Known)
			for (SearchNr=0; SearchNr<NrOfMonkeys; SearchNr++)
				for (OperandNr=0; OperandNr<2; OperandNr++)
					if (!strcmp(Monkey[SearchNr].Name, Monkey[MonkeyNr].OperandName[OperandNr]))
						Monkey[MonkeyNr].OperandIndex[OperandNr] = SearchNr;
	// Double-check that all operand names have been covered
	for (MonkeyNr=0; MonkeyNr<NrOfMonkeys; MonkeyNr++)
		for (OperandNr=0; OperandNr<2; OperandNr++)
			if (Monkey[MonkeyNr].OperandIndex[OperandNr] < 0)
			{
				fprintf(stderr, "Monkey[%d]=%s has unknown operand[%d]=%s\n",
						MonkeyNr, Monkey[MonkeyNr].Name, OperandNr, Monkey[MonkeyNr].OperandName[OperandNr]);
				exit(3);
			}

	/*******************/
	/* Data processing */
	/*******************/
	// Just keep going over the whole list and perform the operations where both operands have been worked out
	for (;;)
	{
		int OperationsPerformed = 0;
		for (MonkeyNr=0; MonkeyNr<NrOfMonkeys; MonkeyNr++)
			if ((!Monkey[MonkeyNr].Known) && (MonkeyNr != RootIndex) &&
					(Monkey[Monkey[MonkeyNr].OperandIndex[0]].Known) &&
					(Monkey[Monkey[MonkeyNr].OperandIndex[1]].Known))
			{
				switch (Monkey[MonkeyNr].Operator)
				{
					case '+': Monkey[MonkeyNr].Value = 
						Monkey[Monkey[MonkeyNr].OperandIndex[0]].Value + Monkey[Monkey[MonkeyNr].OperandIndex[1]].Value; break;
					case '-': Monkey[MonkeyNr].Value = 
						Monkey[Monkey[MonkeyNr].OperandIndex[0]].Value - Monkey[Monkey[MonkeyNr].OperandIndex[1]].Value; break;
					case '*': Monkey[MonkeyNr].Value = 
						Monkey[Monkey[MonkeyNr].OperandIndex[0]].Value * Monkey[Monkey[MonkeyNr].OperandIndex[1]].Value; break;
					case '/': Monkey[MonkeyNr].Value = 
						Monkey[Monkey[MonkeyNr].OperandIndex[0]].Value / Monkey[Monkey[MonkeyNr].OperandIndex[1]].Value; break;
				}
				OperationsPerformed++;
				Monkey[MonkeyNr].Known = 1;
				if (0)
				printf("Monkey[%d]=%s works out to value %lld\n", MonkeyNr,Monkey[MonkeyNr].Name, Monkey[MonkeyNr].Value);
				// Try to follow the chain from the human input
				if ((Monkey[Monkey[MonkeyNr].OperandIndex[0]].FromHuman) ||
						(Monkey[Monkey[MonkeyNr].OperandIndex[1]].FromHuman))
				{
					Monkey[MonkeyNr].FromHuman = 1;  // Inherit this property from either operand
					printf("Monkey[%d]=%s%s: %lld %c Monkey[%d]=%s%s: %lld yields Monkey[%d]=%s<H>: %lld\n",
							Monkey[MonkeyNr].OperandIndex[0],
							Monkey[Monkey[MonkeyNr].OperandIndex[0]].Name,
							Monkey[Monkey[MonkeyNr].OperandIndex[0]].FromHuman ? "<H>" : "",
							Monkey[Monkey[MonkeyNr].OperandIndex[0]].Value,
							Monkey[MonkeyNr].Operator,
							Monkey[MonkeyNr].OperandIndex[1],
							Monkey[Monkey[MonkeyNr].OperandIndex[1]].Name,
							Monkey[Monkey[MonkeyNr].OperandIndex[1]].FromHuman ? "<H>" : "",
							Monkey[Monkey[MonkeyNr].OperandIndex[1]].Value,
							MonkeyNr, Monkey[MonkeyNr].Name, Monkey[MonkeyNr].Value);
					if ((Monkey[Monkey[MonkeyNr].OperandIndex[0]].FromHuman) &&
							(Monkey[Monkey[MonkeyNr].OperandIndex[1]].FromHuman))
						printf("*** Double chain dependency detected ! ***\n");
				}
			}
		if (!OperationsPerformed)  break;
		if (0)
		printf("Performed %d operations\n", OperationsPerformed);
	} /* for(;;) */

	/*************/
	/* Reporting */
	/*************/
	if (Monkey[RootIndex].Known)
	{
		fprintf(stderr, "\"root\" value was supposed to remain unknown.\n");
		exit(3);
	}
	printf("\n\"%s\"[%d] now compares between Monkey[%d]=%s%s: %lld and Monkey[%d]=%s%s: %lld\n\n",
			Monkey[RootIndex].Name, RootIndex,
			Monkey[RootIndex].OperandIndex[0],
			Monkey[Monkey[RootIndex].OperandIndex[0]].Name,
			Monkey[Monkey[RootIndex].OperandIndex[0]].FromHuman ? "<H>" : "",
			Monkey[Monkey[RootIndex].OperandIndex[0]].Value,
			Monkey[RootIndex].OperandIndex[1],
			Monkey[Monkey[RootIndex].OperandIndex[1]].Name,
			Monkey[Monkey[RootIndex].OperandIndex[1]].FromHuman ? "<H>" : "",
			Monkey[Monkey[RootIndex].OperandIndex[1]].Value);

	// Now trace back the whole chain of operations to meet this requirement
	MonkeyNr = RootIndex;
	for (;;)
	{
		int TargetNr;
		long long TargetValue, OldTargetValue=TargetValue;
		for (OperandNr=0; OperandNr<2; OperandNr++)
			if (Monkey[Monkey[MonkeyNr].OperandIndex[OperandNr]].FromHuman)
			{
				TargetNr = Monkey[MonkeyNr].OperandIndex[OperandNr];
				break;
			}
		if (MonkeyNr == RootIndex)
			TargetValue = Monkey[Monkey[RootIndex].OperandIndex[1-OperandNr]].Value;
		else
		{
			switch (Monkey[MonkeyNr].Operator)
			{
				case '+':	TargetValue -= Monkey[Monkey[MonkeyNr].OperandIndex[1-OperandNr]].Value; break;
				case '*':	TargetValue /= Monkey[Monkey[MonkeyNr].OperandIndex[1-OperandNr]].Value; break;
					// Not-commutative operations are a bit trickier to reverse
				case '-':
					if (OperandNr)
						TargetValue = Monkey[Monkey[MonkeyNr].OperandIndex[0]].Value - TargetValue;
					else
						TargetValue += Monkey[Monkey[MonkeyNr].OperandIndex[1]].Value;
					break;
				case '/':
					if (OperandNr)
						TargetValue = Monkey[Monkey[MonkeyNr].OperandIndex[0]].Value / TargetValue;
					else
						TargetValue *= Monkey[Monkey[MonkeyNr].OperandIndex[1]].Value;
					break;
			}
			if (OperandNr)
				printf("To make %s[%d]=%lld %c %s[%d]=<H> become %s[%d]=%lld, fill in <H> value %lld\n",
						Monkey[Monkey[MonkeyNr].OperandIndex[0]].Name, Monkey[MonkeyNr].OperandIndex[0],
						Monkey[Monkey[MonkeyNr].OperandIndex[0]].Value, Monkey[MonkeyNr].Operator,
						Monkey[Monkey[MonkeyNr].OperandIndex[1]].Name, Monkey[MonkeyNr].OperandIndex[1],
						Monkey[MonkeyNr].Name, MonkeyNr,
						OldTargetValue, TargetValue);
			else
				printf("To make %s[%d]=<H> %c %s[%d]=%lld become %s[%d]=%lld, fill in <H> value %lld\n",
						Monkey[Monkey[MonkeyNr].OperandIndex[0]].Name, Monkey[MonkeyNr].OperandIndex[0],
						Monkey[MonkeyNr].Operator,
						Monkey[Monkey[MonkeyNr].OperandIndex[1]].Name, Monkey[MonkeyNr].OperandIndex[1],
						Monkey[Monkey[MonkeyNr].OperandIndex[1]].Value,
						Monkey[MonkeyNr].Name, MonkeyNr,
						OldTargetValue, TargetValue);
		}
		MonkeyNr = TargetNr;
		if (!strcmp(Monkey[MonkeyNr].Name, "humn"))  break;
	} /* for(;;) */
	return 0;
}
