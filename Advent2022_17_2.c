#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char Block[5][4][5] = {{
"....",
"....",
"....",
"####"
  },{
"....",
".#..",
"###.",
".#.."
  },{
"....",
"..#.",
"..#.",
"###."
  },{
"#...",
"#...",
"#...",
"#..."
  },{
"....",
"....",
"##..",
"##.."
}};

char* Grid[7];

int IsValidPosition(int BlockNr, int OffsetX, int OffsetY)
{
  if (OffsetY < 0)  return 0;
  if (OffsetX < 0)  return 0;

  int X,Y;
  for (Y=0; Y<4; Y++)
    for (X=0; X<4; X++)
      if (Block[BlockNr][3-Y][X] == '#')
      {
        if (OffsetX+X >= 7)  return 0;
        if (Grid[OffsetX+X][OffsetY+Y] == '#')  return 0;
      }
  return 1;
}

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

  int WindNr=0, NrOfWind;
  char Wind[11000];
  if (!(fread(Wind, sizeof(char), 11000, InputFile)))
  {
    fprintf(stderr, "Cannot read Wind movements\n");
    exit(2);
  }
  while ((Wind[strlen(Wind)-1]=='\n') || (Wind[strlen(Wind)-1]=='\r'))
    Wind[strlen(Wind)-1] = '\0';
  NrOfWind = strlen(Wind);

  int X,Y, BlockX, BlockY, TryX,TryY, MaxY=-1;
  for (X=0; X<7; X++)
  {
    Grid[X] = (char*)malloc(1000004 * sizeof(char));
    memset(Grid[X], '.', 1000004);
  }

  int BlockNr, FallNr;
  int Interesting;
  // Pattern detection results
  int PeriodSumBlocks=0, PeriodSumHeight=0;
  int PeriodFound=0, MaxYAtPeriodFound=0, RemainingBlocks=0;
  unsigned long long HeightReached=0;
  for (FallNr=0; ; FallNr++)
  {
    BlockNr = FallNr%5;
    //if ((!BlockNr) && (!WindNr))  Interesting=1; else Interesting=0;
    //if (!WindNr)  Interesting=1; else Interesting=0;
    //if ((FallNr%1710)==1089) Interesting=1; else Interesting=0;
    Interesting = 0;  // On a per-Fall basis, so will usually consume multiple WindNrs

    // The new block appears at X=2, Y=MaxY+4
    BlockX=2; BlockY=MaxY+4;

    // Simulate the entire fall of this block
    for(;;)
    {
      // Attempt to move sideways
      if (Wind[WindNr]=='<')
        TryX = BlockX-1;
      else if (Wind[WindNr]=='>')
        TryX = BlockX+1;
      else
      {
        fprintf(stderr, "Error: Wind[%d]=%c ??\n", WindNr, Wind[WindNr]);
        exit(2);
      }
      if (IsValidPosition(BlockNr, TryX, BlockY))
        BlockX = TryX;  // Sideways movement happens

      // This Wind has been dealt with
      if (++WindNr >= NrOfWind)
      {
        WindNr = 0;
        printf("WindNr rollover at FallNr=%d (Block #%d)\n", FallNr, BlockNr+1);
        Interesting = 1;
      }

      // Attempt to fall down
      TryY = BlockY-1;
      if (IsValidPosition(BlockNr, BlockX, TryY))
        BlockY = TryY;  // Downward movement happens
      else
        break;
    } /* for(;;) */

    //printf("Block %d comes to rest at X=%d Y=%d\n", FallNr+1, BlockX, BlockY);

    // Place the block permanently where it has come to rest
    for (Y=0; Y<4; Y++)
      for (X=0; X<4; X++)
        if (Block[BlockNr][3-Y][X] == '#')
          Grid[BlockX+X][BlockY+Y] = '#';

    // Determine the new MaxY
    for(;;)
    {
      MaxY++;
      for (X=0; X<7; X++)
        if (Grid[X][MaxY] != '.')  break;
      if (X>=7) { MaxY--;  break; }
    } /* for (;;) */

    // This FallNr may have gotten the Interesting attribute because WindNr rolled over
    // - but chances are, that WindNr has advanced by several units by the time
    //   that this Block has come to rest.
    if (Interesting)
    {
      static int PrevFallNr=0, PrevMaxY=0;
      static int DeltaBlocks[10], DeltaHeight[10];
      static int ArrayLevel=0;
      // Note that MaxY=0 means that row Y=0 is occupied, so then the tower is 1 high
      printf("After Block %d (delta=%d), MaxY=%d (delta=%d)\n",
          FallNr+1, FallNr-PrevFallNr, MaxY+1, MaxY-PrevMaxY);

      // Keep track of recent TowerHeight increases, in the hope of finding a rhythm.
      // This should be accompanied by the amount of used Blocks during each period.
      int Index;
      // Make room in the array for a new value
      for (Index=0; Index<10-1; Index++)
      {
        DeltaBlocks[Index] = DeltaBlocks[Index+1];
        DeltaHeight[Index] = DeltaHeight[Index+1];
      }
      DeltaBlocks[10-1] = FallNr-PrevFallNr;
      DeltaHeight[10-1] = MaxY-PrevMaxY;
      // Only now can the bookkeeping be done
      PrevFallNr = FallNr;  PrevMaxY = MaxY;

      // Only attempt to detect a pattern when the array has been filled
      if (ArrayLevel < 10)
        ArrayLevel++;
      else
      {
        static int Period;
        for (Period=1; Period<=5; Period++)
        {
          //printf("Attempting repeat period #%d ..\n", Period);
          for (Index=0; Index<Period; Index++)
          {
            if (0)
            printf("DeltaHeight[%d]=%d ?= DeltaHeight[%d]=%d\n",
                10-1-Index, DeltaHeight[10-1-Index],
                10-1-Index-Period, DeltaHeight[10-1-Index-Period]);
            if (DeltaHeight[10-1-Index] != DeltaHeight[10-1-Index-Period])  break;
          }
          if (Index >= Period)
            // Found a repeating pattern
            break;
        }
        if (Period <= 5)
        {
          // Determine the fixed amount of FallNr vs. Height per Period
          for (Index=0; Index<Period; Index++)
          {
            PeriodSumBlocks += DeltaBlocks[10-1-Index];
            PeriodSumHeight += DeltaHeight[10-1-Index];
          }
          printf("Found a repeating period of %d: for every %d Blocks, Height rises %d\n",
              Period, PeriodSumBlocks, PeriodSumHeight);
          PeriodFound = 1;
          MaxYAtPeriodFound = MaxY;
          // We have already reached a certain height, extrapolate how much more is needed
          unsigned long long FallsNeeded=((unsigned long long)1e12)-FallNr-1;
          HeightReached = MaxY+1;
          unsigned long long NrOfPeriods = FallsNeeded/PeriodSumBlocks;
          HeightReached += (NrOfPeriods * PeriodSumHeight);
          // Determine how many more blocks we're gonna need
          RemainingBlocks = (int)(FallsNeeded % PeriodSumBlocks);
          printf("Therefore, still need %llu Blocks, use %llu Periods, will add %llu Height\n",
              FallsNeeded, NrOfPeriods, NrOfPeriods * PeriodSumHeight);
          printf("Total Heigth then is %llu, still need to simulate %d more Blocks..\n",
              HeightReached, RemainingBlocks);
        }
      } /* if (ArrayLevel >= 10) */
    } /* if (Interesting) */

    //if (FallNr >= 5)  break;
    //if (FallNr >= 500500)  break;
    //if (MaxY >= 1000000) break;
    if (PeriodFound)
      if (!(RemainingBlocks--))
      {
        printf("Reached Height %llu\n", HeightReached + MaxY - MaxYAtPeriodFound);
        break;
      }
  } /* for (FallNr) */

  return 0;
}
