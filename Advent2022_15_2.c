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

	int SensorX[25],SensorY[25], BeaconX[25],BeaconY[25];
	int SensorNr=0, NrOfSensors;

	int X,Y;
	int DistX, DistY, Manhattan;

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
		if (4 != sscanf(InputLine, "Sensor at x=%d, y=%d: closest beacon is at x=%d, y=%d",
						&SensorX[SensorNr], &SensorY[SensorNr], &BeaconX[SensorNr], &BeaconY[SensorNr]))
		{
			fprintf(stderr, "Could not scan from InputLine #%d: %s\n", InputLineNr, InputLine);
			exit(3);
		}
		SensorNr++;
	} /* while (fgets) */
	printf("%d InputLines were read.\n", InputLineNr);
	fclose(InputFile);
	NrOfSensors = SensorNr;

	/*******************/
	/* Data processing */
	/*******************/

	// Just like in 2021/day22, we find ourselves keeping track of intersecting bodies.
	// Here it's about 2-D intersections between square diamonds
	//   (although the split-off fragments will probably not be square anymore)
	//
	// It's easiest to consider the situation from a 45degree view and see all fragments
	// as ordinary rectangles. The problem then becomes that the bounding box,
	// which was originally the square (0,0)-(4M,4M), now becomes a diamond itself.
	//
	// That need not be a problem, however. We can imagine a HUGE diamond bounding box
	// around the original problem, and compute rectangular intersections under
	// the 45degree view inside it. The end collection of leftover rectangles is unimportant;
	// we will then search for that one rectangle somewhere near the origin, of size 1x1.

	// We'll use a coarse transformation of the original coordinates to the 45degree view,
	// not bothering with the 1/V2 factor: (Xdiag, Ydiag) = (X-Y,X+Y)

	// Just as in 2021/day22, each intersection boils down to repeating a line intersection
	// in as many dimensions as needed. Every situation with two finite line segments
	// leads to 3 new segments, delimited by all of the 4 one-dimensional coordinates involved.
	// For the cube intersecions of 2021/day22, each intersection between two (rectangular) cubes
	// gave rise to 3^3=27 new cube volumes being formed. Here, every 2-D intersection will
	// yield 3^2=9 new rectangles, but not each of these offspring is viable.
	//
	// An increasing array of rectangles will need to be maintained.
	// Just like a rectangular cube in 2021/day22 could be denoted by its 'lower left near'
	// and its 'upper right far' coordinates, we denote a rectangle by its two extreme corners
	// 'Low' and 'High'. Each happens to have just 2 dimensions in this 2-D puzzle.

	typedef struct { int Low[2], High[2]; } tRect;   // This means {{Xlow,Ylow}, {Xhigh,Yhigh}}
	tRect *Rect,*NewRect;
#define MAX_RECT 100000000
	if (!(Rect = (tRect*)malloc(MAX_RECT * sizeof(tRect))))
	{
		fprintf(stderr, "Could not allocate array of %d Rectangles\n", MAX_RECT);
		exit(4);
	}
	if (!(NewRect = (tRect*)malloc(MAX_RECT * sizeof(tRect))))
	{
		fprintf(stderr, "Could not allocate New array of %d Rectangles\n", MAX_RECT);
		exit(4);
	}
	// Initialise with just 1 huge rectangle as bounding box.
	// This whole shape is not yet covered by any sensor.
	tRect Huge = {{-100000000, -100000000}, {100000000, 100000000}};
	Rect[0] = Huge;
	int RectNr, NrOfRect=1;
	int Dim;

	// For each successive sensor, cut away the intersecting part of its diamond-shaped coverage
	//   with the known list of remaining uncovered 'rectangles'
	for (SensorNr=0; SensorNr<NrOfSensors; SensorNr++)
	{
		// Determine the Manhattan distance from this sensor to its closest beacon
		DistX = BeaconX[SensorNr] - SensorX[SensorNr];		DistY = BeaconY[SensorNr] - SensorY[SensorNr];
		if (DistX < 0)  DistX = -DistX;		if (DistY < 0)  DistY = -DistY;
		Manhattan = DistX + DistY;

		// In the 45degree view, the 'lower left' and 'upper right' corners, will be
		//   what are in the original grid the 'left' and 'right' corners of the diamond shape.
		// These 'left' and 'right' corners have (in the original grid) coordinates
		//   (X=SensorX-ManhattanDist, Y=SensorY) and (X=SensorX+ManhattanDist, Y=SensorY)
		tRect Intersect =
			{{SensorX[SensorNr]-Manhattan-SensorY[SensorNr], SensorX[SensorNr]-Manhattan+SensorY[SensorNr]},
			 {SensorX[SensorNr]+Manhattan-SensorY[SensorNr], SensorX[SensorNr]+Manhattan+SensorY[SensorNr]}};

		// Debugging
		if (0)
		printf("#%2d: Orig[%d,%d]<->[%d,%d] Diag[%d,%d]<->[%d,%d]\n", SensorNr+1,
				SensorX[SensorNr]-Manhattan, SensorY[SensorNr], SensorX[SensorNr]+Manhattan, SensorY[SensorNr],
				SensorX[SensorNr]-Manhattan-SensorY[SensorNr], SensorX[SensorNr]-Manhattan+SensorY[SensorNr],
				SensorX[SensorNr]+Manhattan-SensorY[SensorNr], SensorX[SensorNr]+Manhattan+SensorY[SensorNr]);

    // For each of the remaining uncovered 'rectangles', Intersect could cover some part,
    // thereby taking away some fragments of this rectangle.
    // Keep the resulting fragments in a new array.
    int NewNr=0;
    tRect* RectPtr;
    for (RectNr=0,RectPtr=Rect; RectNr<NrOfRect; RectNr++,RectPtr++)
    {
      // In every dimension of the intersecting space, the zone of interested is delineated by
      // 4 coordinates. Sort these first.
      int Boundary[2][4];
      for (Dim=0; Dim<2; Dim++)
      {
        Boundary[Dim][0] = Intersect.Low[Dim];
        Boundary[Dim][1] = Intersect.High[Dim];
        // Merge the Low coordinate of the existing rectangle
        Boundary[Dim][2] = RectPtr->Low[Dim];
        for (int Pos=2; Pos; Pos--)
          if (Boundary[Dim][Pos] < Boundary[Dim][Pos-1])
          {
            int temp = Boundary[Dim][Pos-1];
            Boundary[Dim][Pos-1] = Boundary[Dim][Pos];
            Boundary[Dim][Pos] = temp;
          }
        // Merge the High coordinate of the existing rectangle
        Boundary[Dim][3] = RectPtr->High[Dim];
        for (int Pos=3; Pos; Pos--)
          if (Boundary[Dim][Pos] < Boundary[Dim][Pos-1])
          {
            int temp = Boundary[Dim][Pos-1];
            Boundary[Dim][Pos-1] = Boundary[Dim][Pos];
            Boundary[Dim][Pos] = temp;
          }
      } /* for (Dim) */

      // There are now 4 boundary coordinates in every dimension, marking 3 adjoining segments.
      // By picking a SegmentNr in every dimension, one individual fragment is selected.
      int SegmentNr[2]={0};
      for(;;)
      {
        // For this fragment, these are the conditions to survive the intersection:
        // 1. the existing rectangle needs to occupy this segment in ALL dimensions
        // 2. the intersecting shape does not occupy this segment in ANY dimension
        int Survival=0;
        for (Dim=0; Dim<2; Dim++)
        {
          // Don't consider 0-width segments
          if (Boundary[Dim][SegmentNr[Dim]] == Boundary[Dim][SegmentNr[Dim]+1])  break;
          // Condition 1: existing rectangle needs to occupy this segment
          if (RectPtr->Low[Dim] >= Boundary[Dim][SegmentNr[Dim]+1])  break;
          if (RectPtr->High[Dim] <= Boundary[Dim][SegmentNr[Dim]])   break;
          // Condition 2: intersector needs to miss this segment in at least one dimension
          if (Intersect.Low[Dim] >= Boundary[Dim][SegmentNr[Dim]+1]) Survival = 1;
          if (Intersect.High[Dim] <= Boundary[Dim][SegmentNr[Dim]])  Survival = 1;
        }
        if ((Dim >= 2) && (Survival))
        {
          // The fragment should remain
          for (Dim=0; Dim<2; Dim++)
          {
            NewRect[NewNr].Low[Dim] = Boundary[Dim][SegmentNr[Dim]];
            NewRect[NewNr].High[Dim] = Boundary[Dim][SegmentNr[Dim]+1];
          }
          // Overflow check
          if (++NewNr > MAX_RECT)
          {
            fprintf(stderr, "Obtaining NewRectangle #%d when Sensor %d intersects with Rect %d\n",
                NewNr, SensorNr+1, RectNr);
            exit(4);
          }
        }
        // After having considered this fragment, it's time to select the next fragment
        for (Dim=0; Dim<2; Dim++)
          if (SegmentNr[Dim]++ <= 2)  break;
          else SegmentNr[Dim] = 0;
        if (Dim >= 2)  break;  /* from for(;;) */
      } /* for(;;) */
    } /* for (RectNr) */

    // After all New fragments have been stored, these should become the new list of rectangles
    memcpy(Rect, NewRect, NewNr*sizeof(tRect));
    NrOfRect = NewNr;

    printf("%5d rectangles remaining after sensor %d\n", NrOfRect, SensorNr+1);

	} /* for (SensorNr) */

	/*************/
	/* Reporting */
	/*************/
	// As demonstrated by Advent2022_15.jpg the example case does NOT lead to proper detection
	//   because the diamond around X=11,Y=14 (a 2x2 square in the Diagonal coordinate system)
	//   gets misfortunately cut in half by the intersection handling of the "Red" diamond.
	// The actual task did yield the proper result by chance, as the target diamond was intact.
	//
	// A proper full solution therefore requires to recompose the list of remaining diamonds
	//  into as large contiguous blocks as possible. Only then can a search for any remaining
	//  diamonds of size exactly 2x2 in the Diagonal coordinate system, be sure to succeed.

	int OtherRectNr, MergeHappened;
	for (;;)
	{
		MergeHappened = 0;
		for (RectNr=0; RectNr<NrOfRect; RectNr++)
			for (OtherRectNr=0; OtherRectNr<NrOfRect; OtherRectNr++)
			{
				if (OtherRectNr == RectNr)  continue;
				// The coordinates in all dimensions except one, need to be exactly identical
				//   to make a merge possible.
				int theDim = -1;
				for (Dim=0; Dim<2; Dim++)
					if ((Rect[RectNr].Low[Dim]  != Rect[OtherRectNr].Low[Dim] ) ||
							(Rect[RectNr].High[Dim] != Rect[OtherRectNr].High[Dim]))
						// Mismatch : this could become that one dimension where coordinates differ
						if (theDim < 0)
							theDim = Dim;
						else
							break;
				if (Dim < 2)  continue;  // Coordinates differ in too many dimensions
				// In that one dimension where coordinates differ, they need to be adjacent
				//   to make a merge possible.
				if (Rect[RectNr].Low[theDim] == Rect[OtherRectNr].High[theDim])
				{
					if (0)
					printf("Merge Rect[%d] (X=%d,Y=%d)-(X=%d,Y=%d)  with  OtherRect[%d] (X=%d,Y=%d)-(X=%d,Y=%d)\n",
							RectNr,
							Rect[RectNr].Low[0], Rect[RectNr].Low[1], Rect[RectNr].High[0], Rect[RectNr].High[1],
							OtherRectNr,
							Rect[OtherRectNr].Low[0], Rect[OtherRectNr].Low[1], Rect[OtherRectNr].High[0], Rect[OtherRectNr].High[1]);
					Rect[OtherRectNr].High[theDim] = Rect[RectNr].High[theDim];  // extend OtherRect to Higher
				}
				else if (Rect[RectNr].High[theDim] == Rect[OtherRectNr].Low[theDim])
				{
					if (0)
					printf("Merge OtherRect[%d] (X=%d,Y=%d)-(X=%d,Y=%d)  with  Rect[%d] (X=%d,Y=%d)-(X=%d,Y=%d)\n",
							OtherRectNr,
							Rect[OtherRectNr].Low[0], Rect[OtherRectNr].Low[1], Rect[OtherRectNr].High[0], Rect[OtherRectNr].High[1],
							RectNr,
							Rect[RectNr].Low[0], Rect[RectNr].Low[1], Rect[RectNr].High[0], Rect[RectNr].High[1]);
					Rect[OtherRectNr].Low[theDim]  = Rect[RectNr].Low[theDim];   // extend OtherRect to Lower
				}
				else
					continue;
				// Arriving here, means that a merge has happened
				MergeHappened++;
				// OtherRect has grown to usurp Rect, so Rect can now be erased.
				for (OtherRectNr=RectNr; OtherRectNr<NrOfRect-1; OtherRectNr++)
					memcpy(&Rect[OtherRectNr], &Rect[OtherRectNr+1], sizeof(tRect));
				NrOfRect--;
				RectNr--;
				// After this messing with OtherRectNr variable, make sure to break from original loop
				break;
			} /* for (OtherRectNr) */
		if (!MergeHappened)  break;
		printf("%3d merges performed\n", MergeHappened);
	} /* for(;;) */
	printf("%5d rectangles remaining after merges\n", NrOfRect);

  for (RectNr=0; RectNr<NrOfRect; RectNr++)
    if ((Rect[RectNr].High[0] == Rect[RectNr].Low[0]+2) &&
        (Rect[RectNr].High[1] == Rect[RectNr].Low[1]+2))
    {
      printf("Rect[%3d] from (%d,%d) to (%d,%d)    ", RectNr,
          Rect[RectNr].Low[0], Rect[RectNr].Low[1], Rect[RectNr].High[0], Rect[RectNr].High[1]);
      int Mid[2];
      for (int Dim=0; Dim<2; Dim++)  Mid[Dim] = (Rect[RectNr].Low[Dim] + Rect[RectNr].High[Dim])/2;
      X = (Mid[0]+Mid[1])/2;  Y = (Mid[1] - Mid[0])/2;
      printf("Mid[%d,%d] back-transformed[%d,%d]\n", Mid[0], Mid[1], X, Y);
      printf("Tuning frequency %lld\n", (long long)X * 4000000 + Y);
    }

	return 0;
}
