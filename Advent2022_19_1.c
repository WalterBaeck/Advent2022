#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

	// Fixed ordering into 4-element arrays: [0]Ore [1]Clay [2]Obsidian [3]Geode
	int Index;
	int Resources[4];     // The amount of raw material owned
	int Robots[4];        // The amount of generating robots owned
	int CostPrimary[4];   // The Ore cost to manufacture each type of robot
	int CostSecundary[4]; // The supplementary cost of the 'previous' resource to manufacture a robot type

	// Weave a path of Choices through the allotted 24 minutes of Time.
	// In order to allow easy backtracking, remember the situation at each moment.
	int Choice[24], Time, ResAtTime[24][4], RobAtTime[24][4];
	// A choice for any of the 4 Robot types is denoted by the above constants.
	// Furthermore, the choice to buy no Robot at all is denoted by constant 4,
	// and when a choice has not been established yet, this is constant -1.

	int BlueprintNr, Score, SumTotal=0;

	while (fgets(InputLine, 1000, InputFile))
	{
		// Bookkeeping
		InputLineNr++;

	/******************/
	/* Initialisation */
	/******************/
		memset(Resources, 0, 4*sizeof(int));
		memset(Robots, 0, 4*sizeof(int)); Robots[0]=1;   // Start off with 1 Ore robot
		memset(CostPrimary, 0, 4*sizeof(int));
		memset(CostSecundary, 0, 4*sizeof(int));

	/******************/
	/* Data gathering */
	/******************/
		// Strip line ending
		while ((InputLine[strlen(InputLine)-1]=='\n')
				|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1]='\0';
		// Parse the input
		if (7 != sscanf(InputLine, 
						"Blueprint %d: Each ore robot costs %d ore. Each clay robot costs %d ore. Each obsidian robot costs %d ore and %d clay. Each geode robot costs %d ore and %d obsidian.",
						&BlueprintNr, &CostPrimary[0], &CostPrimary[1],
						&CostPrimary[2], &CostSecundary[2], &CostPrimary[3], &CostSecundary[3]))
		{
			fprintf(stderr, "Could not scan 7 numbers from InputLine #%d:\n%s\n", InputLineNr, InputLine);
			exit(2);
		}

		// Debugging
    if (1)
    {
		printf("Ore-cost for each Robot type:   [0]%2d [1]%2d [2]%2d [3]%2d\n", 
				CostPrimary[0], CostPrimary[1], CostPrimary[2], CostPrimary[3]);
		printf("Additional cost for next Robot: [0]%2d [1]%2d [2]%2d [3]%2d\n",
				CostSecundary[1], CostSecundary[2], CostSecundary[3], 0);
    }

	/*******************/
	/* Data processing */
	/*******************/
		// Reset the pathfinder
		Time=0; Choice[0]=-1; Score=0;
		// Exhaustive search of all Choices that can be made at all moments in Time
		for (;;)
		{
			// Arriving at this point in Time: first make note of this situation
			memcpy(ResAtTime[Time], Resources, 4*sizeof(int));
			memcpy(RobAtTime[Time], Robots, 4*sizeof(int));
			// The last-made Choice at this moment in Time is the starting point
			int Decision = Choice[Time];

			// Debugging
      if (0)
			printf("Time %2d: Start %2d  Res[%d,%d,%d,%d] Rob[%d,%d,%d,%d]\n",
					Time+1, Choice[Time], Resources[0], Resources[1], Resources[2], Resources[3],
					Robots[0], Robots[1], Robots[2], Robots[3]);
			while (++Decision < 4)
			{
				// Is it affordable to buy this type of Robot ?
				if ((CostPrimary[Decision] > Resources[0]) ||
						((Decision) && (CostSecundary[Decision] > Resources[Decision-1])))  continue;
        // If this type of robot was already affordable in the previous moment in Time,
        //   then refuse this pointless stalling
        if ((Choice[Time-1] == 4) &&
            (CostPrimary[Decision] <= ResAtTime[Time-1][Decision]) &&
            ((!Decision) || (CostSecundary[Decision] <= ResAtTime[Time-1][Decision-1])))
          continue;
				// There's no point in buying a Robot in the last minute, as it won't harvest anything
				if ((Time == 23) && (Decision < 4))  continue;
				// For the same reason, there's no point in buying any other than a Geode Robot
				//   in the one-but-last minute
				if ((Time == 22) && (Decision < 3))  continue;
				// OK this seems to be a sane choice
				break;
			}

			// The outcome of the decision-making process can be either:
			// - decision to buy some kind of Robot (Decision 0..3)
			// - decision to do nothing, always possible: Decision 4
			// - all decisions exhausted: Decision 5
			if (Decision < 4)
			{
				Resources[0] -= CostPrimary[Decision];
				if (Decision)  Resources[Decision-1] -= CostSecundary[Decision];
			}
			else if (Decision > 4)
			{
				// Need to go one step back in Time, if possible.
				if (!Time)   break;  // All paths exhausted
				Time--;
				// Restore the situation at that point in Time
				memcpy(Resources, ResAtTime[Time], 4*sizeof(int));
				memcpy(Robots, RobAtTime[Time], 4*sizeof(int));
				continue;
			}

			// A viable decision has been taken at this point in Time, so register it.
			Choice[Time] = Decision;
			// The next step is harvesting the resources from the existing Robots.
			for (Index=0; Index<4; Index++)
				Resources[Index] += Robots[Index];
			// Then, possibly one more Robot will come into existence
			if (Decision < 4)  Robots[Decision]++;

			// Debugging
      if (0)
			printf("Time %2d: Choice %d  Res[%d,%d,%d,%d] Rob[%d,%d,%d,%d]\n",
					Time+1, Choice[Time], Resources[0], Resources[1], Resources[2], Resources[3],
					Robots[0], Robots[1], Robots[2], Robots[3]);

			// Finally, move to the next moment in Time, if possible
			if (++Time >= 24)
			{

				// Debugging
				if ((0) && (Resources[3]>=9))
				{
				for (Index=0; Index<24; Index++)
				{
					putchar('-');
					if (Choice[Index]<4) putchar('0'+Choice[Index]); else putchar(' ');
				}
				printf(" : %d\n", Resources[3]);
				}

				// Reached the end of times: take stock of what we scored
				if (Resources[3] > Score)  Score = Resources[3];
				// And stay with the last moment in Time
				Time--;
				// but that requires restoring the situation from then
				memcpy(Resources, ResAtTime[Time], 4*sizeof(int));
				memcpy(Robots, RobAtTime[Time], 4*sizeof(int));
			}
			else
				// Advancing to the next moment in Time
				// This requires starting with a clean slate
				Choice[Time] = -1;
		} /* for(;;) */

	/*************/
	/* Reporting */
	/*************/
		printf("BlueprintNr %d yields Score %d\n", BlueprintNr, Score);
    SumTotal += (BlueprintNr * Score);
	} /* while (fgets) */
	printf("%d InputLines were read.\n", InputLineNr);
  printf("SumTotal %d\n", SumTotal);
	fclose(InputFile);

	return 0;
}
