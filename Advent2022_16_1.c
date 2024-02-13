#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Variations.h"

// = = = = = = = = = = = = = =       D a t a t y p e s        = = = = = = = = = = = = = =

typedef struct sValve {
	char Name[3];
	int Flow;
	struct sValve* Lead[10];
	int NrOfLeads;
} tValve;

// = = = = = = = = = = = = = =       V a r i a b l e s        = = = = = = = = = = = = = =

tValve Valve[64];
int ValveNr=0, NrOfValves=0, SearchNr, LeadNr;

// = = = = = = = = = = = = = =       F u n c t i o n s        = = = = = = = = = = = = = =

int FindOrCreateNew(char* Text)
{
	// Look for this name in existing valves
	for (SearchNr=0; SearchNr<NrOfValves; SearchNr++)
		if (!strncmp(Text, Valve[SearchNr].Name, 2))  break;
	if (SearchNr >= NrOfValves)
	{
		// Does not exist yet, need to create it now
		strncpy(Valve[NrOfValves].Name, Text, 2);
		Valve[NrOfValves].Name[2] = '\0';
		//printf("Created Valve #%d with name %s\n", NrOfValves+1, Valve[NrOfValves].Name);
		NrOfValves++;
	}
	return SearchNr;
}

// = = = = = = = = = = = = = =      i n t   M a i n ( )     = = = = = = = = = = = = = =

int main(int Arguments, char* Argument[])
{
	FILE* InputFile = stdin;
	char InputLine[1000];
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

	/******************/
	/* Initialisation */
	/******************/
	memset(Valve, 0, 64*sizeof(tValve));

	/******************/
	/* Data gathering */
	/******************/
	// Parse the input
	while (fgets(InputLine, 1000, InputFile))
	{
		// Bookkeeping
		InputLineNr++;
		// Strip line ending
		while ((InputLine[strlen(InputLine)-1]=='\n')
				|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1]='\0';
		if (strlen(InputLine) < strlen("Valve AA has flow rate=0; tunnels lead to valves DD"))
		{
			fprintf(stderr, "Too short InputLine #%d: %s\n", InputLineNr, InputLine);
			exit(3);
		}
		InputPtr = InputLine + strlen("Valve ");
		ValveNr = FindOrCreateNew(InputPtr);
		InputPtr += strlen("AA has flow rate=");
		if (1 != sscanf(InputPtr, "%d", &Valve[ValveNr].Flow))
		{
			fprintf(stderr, "Could not scan Flow from InputLine #%d: %s\n", InputLineNr, InputLine);
			exit(3);
		}
		if (!(InputPtr = strchr(InputPtr, ';')))
		{
			fprintf(stderr, "Could not find separating semicolon on InputLine #%d: %s\n", InputLineNr, InputLine);
			exit(3);
		}
		// Pitfall: single-tunnel network uses no plural noun
		if (!strncmp(InputPtr+2, "tunnel ", strlen("tunnel ")))
			InputPtr += strlen("; tunnel leads to valve ");
		else
			InputPtr += strlen("; tunnels lead to valves ");
		for (Valve[ValveNr].NrOfLeads=0; InputPtr[2]==','; InputPtr+=4)
			Valve[ValveNr].Lead[Valve[ValveNr].NrOfLeads++] = Valve + FindOrCreateNew(InputPtr);
		Valve[ValveNr].Lead[Valve[ValveNr].NrOfLeads++] = Valve + FindOrCreateNew(InputPtr);
	} /* while (fgets) */
	printf("%d InputLines were read - %d Valves found.\n", InputLineNr, NrOfValves);
	fclose(InputFile);

	/*******************/
	/* Data processing */
	/*******************/
	// First make a list of all the nonzero-Flow valves
	int VisitNr=0, NrOfVisit;
	int Visit[30];

	// Debugging
	printf("Nonzero-flow valves:");

	for (ValveNr=0; ValveNr<NrOfValves; ValveNr++)
		if (Valve[ValveNr].Flow)
		{
			// Debugging
			printf("  %s[%d]", Valve[ValveNr].Name, Valve[ValveNr].Flow);

			Visit[VisitNr++] = ValveNr;
		}

	// Debugging
	putchar('\n');

	NrOfVisit = VisitNr;
	printf("Out of these, %d valves have a nonzero Flow rate\n", NrOfVisit);

	// It's time to compose a spanning graph between all the Valves of interest,
	//   as this information will be used many times over.
	// We need to know the shortest distance between nonzero-Flow valves
	//   and the (zero-Flow) AA valve, that is always the starting point.
	printf("=============== Spanning graph: start ===============\n");
	int NrOfInterest = NrOfVisit + 1;
	int Interest[30];
	Interest[0] = FindOrCreateNew("AA");
	memcpy(Interest+1, Visit, NrOfVisit * sizeof(int));
	int NrOfDistance = 0;
	int DistFrom[900], DistTo[900], Distance[900];
	int Choice[30];
	InitCombination(Choice, 2);
	do {
		// Debugging
		printf("++ From %s[%d] To %s[%d] ++\n",
				Valve[Interest[Choice[0]]].Name, Interest[Choice[0]],
				Valve[Interest[Choice[1]]].Name, Interest[Choice[1]]);

			// FromValveNr = Interest[Choice[0]]
			// ToValveNr   = Interest[Choice[1]]

			// What is the shortest path FromValve -> ToValve ?
			// It would be overkill to turn this into a Dijkstra problem,
			// since all tunnels have equal cost 1.
			// It suffices to explore in concentric 'circles' of increasing radius
			// from the CurrentValve, until the TargetValve is encountered.
			// The radius at which that happens, is the shortest possible distance.

#define CIRCLE_MAX 100
			int ValveList[CIRCLE_MAX], NewList[CIRCLE_MAX];
			int ListNr, NrInList=1, NewNr, Radius=0, TargetFound=0;
			ValveList[0] = Interest[Choice[0]];
			for(;;)
			{
				Radius++;
				NewNr = 0;
				for (ListNr=0; ListNr<NrInList; ListNr++)
				{

					// Debugging
					printf("  Valve %s has %d leads:", Valve[ValveList[ListNr]].Name, Valve[ValveList[ListNr]].NrOfLeads);

					for (LeadNr=0; LeadNr<Valve[ValveList[ListNr]].NrOfLeads; LeadNr++)
					{
						int EndNr = Valve[ValveList[ListNr]].Lead[LeadNr] - Valve;
						// Do not add this EndNr to the NewList if it's already in it
						for (SearchNr=0; SearchNr<NewNr; SearchNr++)
							if (NewList[SearchNr] == EndNr)  break;
						if (SearchNr >= NewNr)
						{
							NewList[NewNr++] = EndNr;
							// Overflow check
							if (NewNr >= CIRCLE_MAX)
							{
								fprintf(stderr, "NewNr=%d overflows %d\n", NewNr, CIRCLE_MAX);
								exit(4);
							}
						}

						// Debugging
						printf("  %s", Valve[EndNr].Name);
					 
						if (EndNr == Interest[Choice[1]])
						{
							TargetFound = 1;
							break;
						}
					}
					if (TargetFound)   break;
				}

				// Debugging
				putchar('\n');

				if (TargetFound)     break;
				memcpy(ValveList, NewList, NewNr*sizeof(int));
				NrInList = NewNr;
				// Without any precautions here, inevitably the search will double back on itself
				// and go back-and-forth over the same paths eternally.
				// But in the meantime, the ToValve will also be found.
			} /* for(;;) */
			// So, the minimum cost to get to the TargetValve is now known.
			DistFrom[NrOfDistance] = Interest[Choice[0]];
			DistTo[NrOfDistance]   = Interest[Choice[1]];
			Distance[NrOfDistance] = Radius;
		NrOfDistance++;

		// Debugging
		printf(" + Distance[%3d] = %d +\n", NrOfDistance, Radius);

	} while (NextCombination(Choice, 2, NrOfInterest));
	printf("=============== Spanning graph: finish ===============\n");

	// Then go over all possible orderings to visit these Flow valves
	int Time, Score, MaxScore=0;
	InitPermutation(Choice, NrOfVisit);

	// For any given ordering, see how it scores
	for(;;)
	{
		// The path always starts from Valve AA, which is always zero-Flow
		ValveNr = FindOrCreateNew("AA");
		Time = 0;
		Score = 0;
		// From there, get to all the next Valves in the order that was picked
		for (VisitNr=0; VisitNr<NrOfVisit; VisitNr++)
		{
			// CurrentValveNr = ValveNr
			// TargetValveNr = Visit[Choice[VisitNr]]

			// What is the shortest path from CurrentValve to TargetValve ?
			// Search it in the spanning graph !
			for (SearchNr=0; SearchNr<NrOfDistance; SearchNr++)
				if (((DistFrom[SearchNr] == ValveNr) && (DistTo[SearchNr] == Visit[Choice[VisitNr]]))
						|| ((DistTo[SearchNr] == ValveNr) && (DistFrom[SearchNr] == Visit[Choice[VisitNr]])))  break;
			if (SearchNr >= NrOfDistance)
			{
				fprintf(stderr, "Internal error: distance from %s[%d] to %s[%d] not found in %d distances\n",
						Valve[ValveNr].Name, ValveNr, Valve[Visit[Choice[VisitNr]]].Name, Visit[Choice[VisitNr]],
						NrOfDistance);
				exit(4);
			}

			// So, the minimum cost to get to the TargetValve is now known.
			Time += Distance[SearchNr];
			// Actually opening this Valve also requires one Time unit
			Time++;
			if (Time >= 30) break;
			// .. but it does bring a known benefit
			Score += (30-Time) * Valve[Visit[Choice[VisitNr]]].Flow;

			// Debugging
			printf("%2d: From %s to %s requires %d but benefits %dx%d=%d\n",
					Time+1, Valve[ValveNr].Name, Valve[Visit[Choice[VisitNr]]].Name, Distance[SearchNr], 
					(30-Time), Valve[Visit[Choice[VisitNr]]].Flow, (30-Time) * Valve[Visit[Choice[VisitNr]]].Flow);

			// The place where we now find ourselves, becomes the new starting point
			ValveNr = Visit[Choice[VisitNr]];
		} /* for (VisitNr) */
		if (Score > MaxScore)  MaxScore = Score;

		//Debugging
		printf("-- This path scores %d --\n", Score);

		if (VisitNr < NrOfVisit)
		{
			// The path was broken off early, due to Time shortage.
			// Therefore it's useful to skip all 'sub-permutations' beyond this position.
			if (!(NextPermutationFromPos(Choice, NrOfVisit, VisitNr)))
				break;
		}
		else
		{
			// Regular increase of the PermutationNr
			if (!(NextPermutation(Choice, NrOfVisit)))
				break;
		}
	} /* for(;;) */

	/*************/
	/* Reporting */
	/*************/
	printf("MaxScore was %d\n", MaxScore);
	return 0;
}
