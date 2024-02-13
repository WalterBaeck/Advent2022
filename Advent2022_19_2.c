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
	int CostPrimary[4];   // The Ore cost to manufacture each type of robot
	int CostSecundary[4]; // The supplementary cost of the 'previous' resource to manufacture a robot type

#define MAX_TIME 32
#define MAX_ARRAY 100000000
#define PRUNING_THRESHOLD (MAX_ARRAY / 10)

	// A choice for any of the 4 Robot types is denoted by the above constants.
	// Furthermore, the choice to buy no Robot at all is denoted by constant 4,
	// and when a choice has not been established yet, this is constant -1.
	int Mineral, Decision, Time;
  int *TrailRes[4], *TrailRob[4], *NewTrailRes[4], *NewTrailRob[4], *TrailChoice, *NewTrailChoice;
  int TrailNr, NrOfTrails, NewTrailNr;
  int *Heuristic, HeurMin, HeurMax, HeurThreshold;

	int BlueprintNr, Score, Product=1;

  // One-time memory allocation
  for (Mineral=0; Mineral<4; Mineral++)
  {
    TrailRes[Mineral] = (int*)malloc(MAX_ARRAY * sizeof(int));
    TrailRob[Mineral] = (int*)malloc(MAX_ARRAY * sizeof(int));
    NewTrailRes[Mineral] = (int*)malloc(MAX_ARRAY * sizeof(int));
    NewTrailRob[Mineral] = (int*)malloc(MAX_ARRAY * sizeof(int));
  }
  TrailChoice = (int*)malloc(MAX_ARRAY * sizeof(int));
  NewTrailChoice = (int*)malloc(MAX_ARRAY * sizeof(int));
  Heuristic = (int*)malloc(MAX_ARRAY * sizeof(int));

	while (fgets(InputLine, 1000, InputFile))
	{
		// Bookkeeping
		InputLineNr++;

	/******************/
	/* Initialisation */
	/******************/
    // This is repeated for every parameter set
		memset(CostPrimary, 0, 4*sizeof(int));
		memset(CostSecundary, 0, 4*sizeof(int));
    Score = 0;
    for (Mineral=0; Mineral<4; Mineral++)
    {  TrailRes[Mineral][0] = 0;  TrailRob[Mineral][0] = 0;  }
    TrailRob[0][0] = 1;
    TrailChoice[0] = 0;
    NrOfTrails = 1;

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
    // For each new moment in time, see how the existing set of Trails can be extended into NewTrails
		for (Time=0; Time<MAX_TIME; Time++)
		{
      NewTrailNr = 0;
      for (TrailNr=0; TrailNr<NrOfTrails; TrailNr++)
      {

			// Debugging
      if (0)
			printf("Time %2d: Start %d Res[%d,%d,%d,%d] Rob[%d,%d,%d,%d]\n",
					Time+1, TrailChoice[TrailNr],
          TrailRes[0][TrailNr], TrailRes[1][TrailNr], TrailRes[2][TrailNr], TrailRes[3][TrailNr],
					TrailRob[0][TrailNr], TrailRob[1][TrailNr], TrailRob[2][TrailNr], TrailRob[3][TrailNr]);

			for (Decision=0; Decision<=4; Decision++)
			{
        if (Decision < 4)
        {
				// Is it affordable to buy this type of Robot ?
				if ((CostPrimary[Decision] > TrailRes[0][TrailNr]) ||
						((Decision) && (CostSecundary[Decision] > TrailRes[Decision-1][TrailNr])))  continue;
        // If this type of robot was already affordable in the previous moment in Time,
        //   then refuse this pointless stalling
        if ((TrailChoice[TrailNr-1] == 4) &&
            (CostPrimary[Decision] <= TrailRes[0][TrailNr] - TrailRob[0][TrailNr]) &&
            ((!Decision) || (CostSecundary[Decision] <= TrailRes[Decision-1][TrailNr] - TrailRob[Decision-1][TrailNr])))
          continue;
        } /* if (Decision < 4) */ 
				// There's no point in buying a Robot in the last minute, as it won't harvest anything
				if ((Time == MAX_TIME-1) && (Decision < 4))  continue;
				// For the same reason, there's no point in buying any other than a Geode Robot
				//   in the one-but-last minute
				if ((Time == MAX_TIME-2) && (Decision < 3))  continue;
        // If all these criteria have been met, then this is a valid choice.
        // Register a new Trailhead, and register this decision.
        for (Mineral=0; Mineral<4; Mineral++)
        {
          // Immediately compute the harvest of this moment in Time, even though the Robot purchase
          //   is supposed to come first in the order of events.
          NewTrailRes[Mineral][NewTrailNr] = TrailRes[Mineral][TrailNr] + TrailRob[Mineral][TrailNr];
          NewTrailRob[Mineral][NewTrailNr] = TrailRob[Mineral][TrailNr];
        }
        NewTrailChoice[NewTrailNr] = Decision;
        // The outcome of the decision-making process can be either:
        // - decision to buy some kind of Robot (Decision 0..3)
        // - decision to do nothing, always possible: Decision 4
        // Decrease the Resources with any Robot that may just have been ordered.
        if (Decision < 4)
        {
          NewTrailRes[0][NewTrailNr] -= CostPrimary[Decision];
          if (Decision)  NewTrailRes[Decision-1][NewTrailNr] -= CostSecundary[Decision];
        }
        // Then, possibly one more Robot will come into existence
        if (Decision < 4)  NewTrailRob[Decision][NewTrailNr]++;

        if (Time == MAX_TIME-1)
          // Reached the end of times: take stock of what we scored
          if (NewTrailRes[3][NewTrailNr] > Score)  Score = NewTrailRes[3][NewTrailNr];

        if (++NewTrailNr > MAX_ARRAY)
        {
          fprintf(stderr, "No room for %d NewTrails\n", NewTrailNr);
          exit(3);
        }
			} /* for (Decision) */
      } /* for (TrailNr) */

      // When the set of NewTrails is growing too big, we need to prune, based on a heuristic appraisal
      if (NewTrailNr > PRUNING_THRESHOLD)
      {
        // Survey all the NewTrails and determine a target threshold for survival
        HeurMin = 1000000; HeurMax = 0;
        for (TrailNr=0; TrailNr<NewTrailNr; TrailNr++)
        {
          Heuristic[TrailNr] = 0;
          for (Mineral=0; Mineral<4; Mineral++)
          {
            // More advanced minerals in the inventory weigh 4x higher in the score
            Heuristic[TrailNr] += (NewTrailRes[Mineral][TrailNr] << (2*Mineral));
            // A robot in the inventory is good for so many more minerals until the end of Time
            Heuristic[TrailNr] +=
              ((NewTrailRob[Mineral][TrailNr] << (2*Mineral)) * (MAX_TIME - Time));
          }
          if (Heuristic[TrailNr] < HeurMin)  HeurMin = Heuristic[TrailNr];
          if (Heuristic[TrailNr] > HeurMax)  HeurMax = Heuristic[TrailNr];
        } /* for (TrailNr) */
        // Now determine the threshold as 25% between Min and Max heuristic score.
        // If the distribution were linear, this would result in pruning down to 75% of the NewTrails.
        HeurThreshold = (2*HeurMin + 2*HeurMax)/4;
        // Go over all the NewTrails again, and copy just the survivors back into the Trails set.
        NrOfTrails = 0;
        for (TrailNr=0; TrailNr<NewTrailNr; TrailNr++)
          if (Heuristic[TrailNr] > HeurThreshold)
          {
            for (Mineral=0; Mineral<4; Mineral++)
            {
              TrailRes[Mineral][NrOfTrails] = NewTrailRes[Mineral][TrailNr];
              TrailRob[Mineral][NrOfTrails] = NewTrailRob[Mineral][TrailNr];
            }
            TrailChoice[NrOfTrails] = NewTrailChoice[TrailNr];
            NrOfTrails++;
          }

        // Debugging
        printf("Time %2d: pruned %7d NewTrails down to %5d Trails\n", Time+1, NewTrailNr, NrOfTrails);
      }
      else
      {
        // The full set of NewTrails can stay
        // The old set of Trails has become useless now; replace it with the NewTrail set
        for (Mineral=0; Mineral<4; Mineral++)
        {
          memcpy(TrailRes[Mineral], NewTrailRes[Mineral], NewTrailNr * sizeof(int));
          memcpy(TrailRob[Mineral], NewTrailRob[Mineral], NewTrailNr * sizeof(int));
        }
        memcpy(TrailChoice, NewTrailChoice, NewTrailNr * sizeof(int));
        NrOfTrails = NewTrailNr;

        // Debugging
        printf("Time %2d: found %7d Trails\n", Time+1, NrOfTrails);
      } /* if (NewTrailNr > 64000) */
		} /* for (Time) */

	/*************/
	/* Reporting */
	/*************/
		printf("BlueprintNr %d yields Score %d\n", BlueprintNr, Score);
    Product *= Score;
    if (InputLineNr >= 3)  break;
	} /* while (fgets) */
	printf("%d InputLines were read.\n", InputLineNr);
  printf("Product %d\n", Product);
	fclose(InputFile);

	return 0;
}
