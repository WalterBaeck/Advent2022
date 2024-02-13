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
    Grid[X] = (char*)malloc(11000 * sizeof(char));
    memset(Grid[X], '.', 11000);
  }

  int BlockNr, FallNr;
  for (FallNr=0; FallNr<2022; FallNr++)
  {
    BlockNr = FallNr%5;

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
      if (++WindNr >= NrOfWind)  WindNr = 0;

      // Attempt to fall down
      TryY = BlockY-1;
      if (IsValidPosition(BlockNr, BlockX, TryY))
        BlockY = TryY;  // Downward movement happens
      else
        break;
    } /* for(;;) */

    printf("Block %d comes to rest at X=%d Y=%d\n", FallNr+1, BlockX, BlockY);

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
    printf("After Block %d, MaxY=%d\n", FallNr+1, MaxY);

    //if (FallNr >= 5)  break;
  } /* for (FallNr) */

  printf("After %d blocks, the tower is %d tall\n", FallNr, MaxY+1);

  return 0;
}
