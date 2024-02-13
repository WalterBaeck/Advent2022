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

#define SCAN_MIN -1000000
#define SCAN_MAX  9000000
#define LENGTH   (SCAN_MAX - SCAN_MIN)

	char *Scanline;
	int X,Y, cScanX, cScanY=2000000;  // =10 for Test
	int DistX, DistY, Manhattan, Remainder;

	/******************/
	/* Initialisation */
	/******************/
	if (!(Scanline = (char*)malloc(LENGTH * sizeof(char))))
	{
		fprintf(stderr, "Could not allocate %d chars for Scanline\n", LENGTH);
		exit(4);
	}
	memset(Scanline, 0, LENGTH*sizeof(char));

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
		if ((SensorX[SensorNr] < SCAN_MIN) || (SensorX[SensorNr] >= SCAN_MAX) ||
				(BeaconX[SensorNr] < SCAN_MIN) || (BeaconX[SensorNr] >= SCAN_MAX))
		{
			fprintf(stderr, "SensorX %d   or    BeaconX %d  outside grid on InputLine #%d: %s\n",
					SensorX[SensorNr], BeaconX[SensorNr], InputLineNr, InputLine);
			exit(3);
		}
		// Catch the tricky situation where a beacon is located ON the scanline
		if (BeaconY[SensorNr] == cScanY)  cScanX = BeaconX[SensorNr];
		// Determine the Manhattan distance from this sensor to its closest beacon
		DistX = BeaconX[SensorNr] - SensorX[SensorNr];		DistY = BeaconY[SensorNr] - SensorY[SensorNr];
		if (DistX < 0)  DistX = -DistX;		if (DistY < 0)  DistY = -DistY;
		Manhattan = DistX + DistY;
		// Now establish the vertical distance from this sensor to the scanline under consideration
		DistY = cScanY - SensorY[SensorNr];   if (DistY < 0)  DistY = -DistY;
		if (DistY <= Manhattan)
		{
			// There's still some action radius to spare after covering this vertical distance
			Remainder = Manhattan - DistY;
			// This means that a series of locations on the scanline are covered
			// Sanity check
			if (SensorX[SensorNr] - Remainder < SCAN_MIN)
			{
				fprintf(stderr, "SensorX[%d]=%d - Remainder=%d goes below SCAN_MIN=%d\n",
						SensorNr, SensorX[SensorNr], Remainder, SCAN_MIN);
				exit(3);
			}
			if (SensorX[SensorNr] + Remainder >= SCAN_MAX)
			{
				fprintf(stderr, "SensorX[%d]=%d + Remainder=%d goes beyond SCAN_MAX=%d\n",
						SensorNr, SensorX[SensorNr], Remainder, SCAN_MAX);
				exit(3);
			}
			// Now mark the covered positions on the scanline
			memset(Scanline + SensorX[SensorNr] - Remainder - SCAN_MIN, 1, 2*Remainder + 1);
		}
		SensorNr++;
	} /* while (fgets) */
	printf("%d InputLines were read.\n", InputLineNr);
	fclose(InputFile);
	NrOfSensors = SensorNr;

	/*******************/
	/* Data processing */
	/*******************/
	int CoveredNr=0;
	for (X=0; X<LENGTH; X++)
		if (Scanline[X])  CoveredNr++;
	// Compensate for the beacon ON the scanline
	if (Scanline[cScanX - SCAN_MIN])  CoveredNr--; // Doesn't count

	/*************/
	/* Reporting */
	/*************/
	printf("Scanline at Y=%d has %d positions covered\n", cScanY, CoveredNr);
	return 0;
}
