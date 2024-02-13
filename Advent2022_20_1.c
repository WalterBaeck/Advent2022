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

  typedef struct {
    int Value;
    int Place;
  } tNum;
  tNum Num[5000];
  tNum* Ptr[5000];

  int Pos,Numbers=0;
  int ZeroPos=-1;

	/******************/
	/* Initialisation */
	/******************/
  for (Pos=0; Pos<5000; Pos++)
  {
    Ptr[Pos] = Num+Pos;
    Num[Pos].Place = Pos;
  }

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
    if (1 != sscanf(InputLine, "%d", &(Num[Numbers].Value)))
    {
      fprintf(stderr, "Could not scan Nr[%d] from InputLine #%d: %s\n",
          Numbers, InputLineNr, InputLine);
      exit(2);
    }
    if (!Num[Numbers].Value)
    {
      ZeroPos = Numbers;
      //printf("Zero found at original position %d\n", ZeroPos);
    }
    Numbers++;
	} /* while (fgets) */
	printf("%d InputLines were read for %d Numbers.\n", InputLineNr, Numbers);
	fclose(InputFile);
  if (ZeroPos < 0)
  {
    fprintf(stderr, "Zero not found in input dataset.\n");
    exit(2);
  }

	/*******************/
	/* Data processing */
	/*******************/
  // The Num[] array never moves, so it keeps the numbers in their original ordering.
  // Instead, the Ptr[] array that points to them, is being shuffled around.
  for (Pos=0; Pos<=Numbers; Pos++)
  {

    // Debugging
    if (0)
    {
    printf("Array situation:");
    for (int Index=0; Index<Numbers; Index++)
      printf(" %d", Ptr[Index]->Value);
    putchar('\n');
    }

    if (Pos == Numbers)  break;

    // Do nothing for Zero
    if (!Num[Pos].Value)  continue;

    // For Num[Pos], find out where its corresponding pointer is currently at
    int CurrentLocation = Num[Pos].Place;
    int TargetLocation = CurrentLocation + Num[Pos].Value;

    // Weird stuff happens when TargetLocation wraps around
    if (TargetLocation <= 0)
		{
			int Quotient = (-TargetLocation) / (Numbers-1);
			TargetLocation += ((Quotient+1) * (Numbers-1));
		}
    else if (TargetLocation >= Numbers)
		{
			int Quotient = TargetLocation / (Numbers-1);
			TargetLocation -= (Quotient * (Numbers-1));
		}

    // Debugging
    printf("Moving number %d, originally at pos %d but now at pos %d, to new location %d\n",
        Num[Pos].Value, Pos, CurrentLocation, TargetLocation);

    // The actual mixing happens by shifting all existing numbers one slot closer
    //   to the CurrentLocation. This happens in the appropriate Direction:
    int Direction = (TargetLocation > CurrentLocation) ? 1 : -1;
    // The contents at CurrentLocation need to be remembered:
    tNum* Overwriting = Ptr[CurrentLocation];
    for (int Location=CurrentLocation; Location != TargetLocation; Location+=Direction)
    {

      // Debugging
      if (0)
      printf("Swap[%4d]: Existing %d Overwriting %d\n",
          Location, Ptr[Location]->Value, Ptr[Location+Direction]->Value);

      Ptr[Location] = Ptr[Location+Direction];
      Ptr[Location]->Place = Location;
    } /* for (Location) */
    // Finally, the TargetLocation should be taken over with its new inhabitant
    Ptr[TargetLocation] = Overwriting;
    Ptr[TargetLocation]->Place = TargetLocation;
  } /* for (Pos) */

	/*************/
	/* Reporting */
	/*************/
  int Location = Num[ZeroPos].Place;
  printf("Zero used to be at pos %d but is now at pos %d\n", ZeroPos, Location);
  int Sum=0;
  for (Pos=1000; Pos<=3000; Pos+=1000)
  {
    printf("At position=%d (or actually %d) we now have number %d\n",
        Location + Pos, (Location + Pos) % Numbers,
        Ptr[(Location + Pos) % Numbers]->Value);
    Sum += Ptr[(Location + Pos) % Numbers]->Value;
  }
  printf("Sum = %d\n", Sum);
	return 0;
}
