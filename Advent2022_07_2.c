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
#define MAX_CHILDREN 20
	typedef enum {eDir, eFile} tType;
	typedef struct sNode {
		tType Type;
		char Name[50];
		int Size;
		int NrOfChildren;
		struct sNode *Child[MAX_CHILDREN];
		struct sNode *Parent;
	} tNode;

	tNode Root, *Node, *ChildNode;
	int ChildNr, SearchNr;

	// Start with minimal tree
	Root.Type = eDir;
	sprintf(Root.Name, "/");
	Root.Size = 0;
	Root.NrOfChildren = 0;
	Root.Parent = NULL;

	// Parse the input
	while (fgets(InputLine, 100, InputFile))
	{
		// Bookkeeping
		InputLineNr++;
		// Strip line ending
		while ((InputLine[strlen(InputLine)-1]=='\n')
				|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1]='\0';
		// InputLine can be either a command, or part of listing output
		if (*InputLine == '$')
		{
			// Command should have a space in front of it
			if (InputLine[1] != ' ')
			{
				fprintf(stderr, "Command should start with \"$ \" on InputLine #%d: %s\n",
						InputLineNr, InputLine);
				exit(3);
			}
			if (!strncmp(InputLine+2, "cd ", 3))
			{
				// Changing directory can have 3 kinds of argument:
				if (InputLine[5] == '/')
					// Go to root
					Node = &Root;
				else if (!strcmp(InputLine+5, ".."))
				{
					// Go up one level
					if (!Node->Parent)
					{
						fprintf(stderr, "Command cd.. impossible from orphan node %s\n", Node->Name);
						exit(4);
					}
					Node = Node->Parent;
				}
				else
				{
					// Go down one level to specific name
					for (SearchNr=0; SearchNr<Node->NrOfChildren; SearchNr++)
						if (!strcmp(InputLine+5, Node->Child[SearchNr]->Name))  break;
					if (SearchNr<Node->NrOfChildren)
					{
						if (Node->Child[SearchNr]->Type != eDir)
						{
							fprintf(stderr, "Command \"cd %s\" does not lead to a directory-type\n", InputLine+5);
							exit(4);
						}
						Node = Node->Child[SearchNr];
					}
					else
					{
						fprintf(stderr, "Could not find Child directory %s from Node %s for InputLine #%d: %s\n",
								InputLine+5, Node->Name, InputLineNr, InputLine);
						exit(3);
					}
				}
			}  // "$ cd " command
			else if (!strcmp(InputLine+2, "ls"))
				ChildNr = 0;
			else
			{
				fprintf(stderr, "Unrecognized command on InputLine #%d: %s\n", InputLineNr, InputLine);
				exit(3);
			}
		} /* command */
		else  /* listing */
		{
			// Every line of listing output creates a new Node
			if (!(ChildNode = (tNode*)malloc(sizeof(tNode))))
			{
				fprintf(stderr, "Could not malloc a new node for InputLine #%d: %s\n", InputLineNr, InputLine);
				exit(5);
			}
			Node->Child[ChildNr] = ChildNode;
			ChildNode->Parent = Node;
			// The new node can be either a directory or a file
			if (!strncmp(InputLine, "dir ", 4))
			{
				ChildNode->Type = eDir;
				strncpy(ChildNode->Name, InputLine+4, 50);
				ChildNode->Size = 0;
				ChildNode->NrOfChildren = 0;
				// printf("Creating directory %s for InputLine #%d\n", ChildNode->Name,  InputLineNr);
			}
			else /* file */
			{
				ChildNode->Type = eFile;
				if (2 != sscanf(InputLine, "%d %s", &ChildNode->Size, &ChildNode->Name))
				{
					fprintf(stderr, "Could not scan Size, Name for file at InputLine #%d: %s\n",
							InputLineNr, InputLine);
					exit(3);
				}
			}
			if (++ChildNr > MAX_CHILDREN)
			{
				fprintf(stderr, "Too many Children of dir %s at InputLine #%d: %s\n",
						Node->Name, InputLineNr, InputLine);
				exit(4);
			}
			Node->NrOfChildren = ChildNr;
		} /* listing */
	} /* while (fgets) */
	printf("%d InputLines were read.\n", InputLineNr);
	fclose(InputFile);

	/*******************/
	/* Data processing */
	/*******************/
	// Traverse the entire tree, summing up all filesizes into their parent directory
	Node = &Root;
	ChildNr = 0;
	int DirSize[1000], DirNr=0, NrOfDirs;
	for (;;)
	{
		// No more Children to handle here ?
		if (ChildNr >= Node->NrOfChildren)
		{
			// Will need to rise up, but first compile the sum at this Node
			Node->Size = 0;
			for (SearchNr=0; SearchNr<Node->NrOfChildren; SearchNr++)
				Node->Size += Node->Child[SearchNr]->Size;
			// Debugging
			printf("dir %s has total size %d\n", Node->Name, Node->Size);
			DirSize[DirNr] = Node->Size;
			if (++DirNr > 1000)
			{
				fprintf(stderr, "Going over MaxNrOfDirs !\n");
				exit(4);
			}
			// When rising up becomes impossible, we have completed the tree traversal
			if (!Node->Parent)  break;
			// Rise up, but remember which ChildNr we came from
			ChildNode = Node;
			Node = Node->Parent;
			for (ChildNr=0; ChildNr<Node->NrOfChildren; ChildNr++)
				if (ChildNode == Node->Child[ChildNr])  break;
			if (ChildNr >= Node->NrOfChildren)
			{
				fprintf(stderr, "Could not find ChildNr when rising from %s to dir %s\n", ChildNode->Name, Node->Name);
				exit(4);
			}
			ChildNr++;
			continue;
		}
		// Handle this Child here
		ChildNode = Node->Child[ChildNr];
		if (ChildNode->Type == eDir)
		{
			// Descend into any directory encountered
			Node = ChildNode;
			ChildNr = 0;
			continue;
		}
		else /* file */
			// Do nothing particular with a file; it's sum will be totaled later
			ChildNr++;
	} /* for (;;) */
	NrOfDirs = DirNr;

	/*************/
	/* Reporting */
	/*************/
	int MinDirSize = Root.Size;
	for (DirNr=0; DirNr<NrOfDirs; DirNr++)
		if ((DirSize[DirNr] >= Root.Size-40000000) && (DirSize[DirNr] < MinDirSize))
			MinDirSize = DirSize[DirNr];
	printf("Found MinDirSize=%d\n", MinDirSize);
	return 0;
}
