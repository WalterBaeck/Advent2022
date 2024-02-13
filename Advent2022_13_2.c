#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// = = = = = = = = = = = = = =       D a t a t y p e s        = = = = = = = = = = = = = =

#define MAX_CHILDREN 20
	typedef enum {eNum, eList} tType;
	typedef struct sNode {
		tType Type;
		int Num;
		int NrOfChildren;
		struct sNode *Child[MAX_CHILDREN];
		struct sNode *Parent;
	} tNode;

// = = = = = = = = = = = = = =       V a r i a b l e s        = = = = = = = = = = = = = =

	char InputLine[1000];
	char* InputPtr;
	int InputLineNr=0, InputPos, InputLen;

	int ChildNr;
	tNode *Node, *ChildNode;

// = = = = = = = = = = = = = =       F u n c t i o n s        = = = = = = = = = = = = = =

tNode* ParseInputLine(void)
{
    // Start with minimal tree
    Node = (tNode*)malloc(sizeof(tNode));
    Node->Type = eList;
    Node->NrOfChildren = 0;
    Node->Parent = NULL;
    InputPtr = InputLine+1;

    // Debugging
    char Indent[100];
    *Indent = '\0';

    while (*InputPtr)
      if (*InputPtr == '[')
      {
        // Starting a new List as child of the current list
        ChildNode = (tNode*)malloc(sizeof(tNode));
        ChildNode->Type = eList;
        ChildNode->NrOfChildren = 0;
        ChildNode->Parent = Node;
        Node->Child[Node->NrOfChildren++] = ChildNode;
        // Overflow check
        if (Node->NrOfChildren >= MAX_CHILDREN)
        {
          fprintf(stderr, "InputLine #%d pos %d overflow MaxChildren=%d\n",
              InputLineNr, InputPtr-InputLine, MAX_CHILDREN);
          exit(4);
        }
        // And descend into that child now
        Node = ChildNode;
        InputPtr++;
        strcat(Indent, "  ");
      }
      else if (*InputPtr == ']')
      {
if (1)
{
        // Debugging
        printf("%sClosing with [%d] elements\n", Indent, Node->NrOfChildren);
}
        // Closing the ongoing List : just rise to Parent node
        InputPtr++;
        if (!Node->Parent)  break;
        Node = Node->Parent;
        Indent[strlen(Indent)-2] = '\0';
      }
      else if (*InputPtr == ',')
      {
        // As a continuation character within the ongoing list, a comma requires no further action
        InputPtr++;
      }
      else if ((*InputPtr >= '0') && (*InputPtr <= '9'))
      {
        // Numbers should always end on a comma or a closing brace
        char* EndPtr = strpbrk(InputPtr, ",]");
        if (!EndPtr)
        {
          fprintf(stderr, "Unfinished number on InputLine #%d pos %d\n",
              InputLineNr, InputPtr-InputLine);
          exit(3);
        }
        // Starting a new child of the current list, to provide room for this number
        ChildNode = (tNode*)malloc(sizeof(tNode));
        ChildNode->Type = eNum;
        ChildNode->NrOfChildren = 0;
        ChildNode->Parent = Node;
        Node->Child[Node->NrOfChildren++] = ChildNode;
        // Overflow check
        if (Node->NrOfChildren >= MAX_CHILDREN)
        {
          fprintf(stderr, "InputLine #%d pos %d overflow MaxChildren=%d\n",
              InputLineNr, InputPtr-InputLine, MAX_CHILDREN);
          exit(4);
        }
        if (1 != sscanf(InputPtr, "%d", &(ChildNode->Num)))
        {
          fprintf(stderr, "Could not scan number on InputLine #%d pos %d\n",
              InputLineNr, InputPtr-InputLine);
          exit(3);
        }
if (1)
{
        // Debugging
        printf("%sParsed Num: %d\n", Indent, ChildNode->Num);
        InputPtr = EndPtr;
}
      }
      else
      {
        fprintf(stderr, "Unrecognized char on InputLine #%d pos %d\n",
            InputLineNr, InputPtr-InputLine);
        exit(3);
      }
    // At the end of (while (*InputPtr)), or having broken out of the loop
    if (*InputPtr)
    {
      fprintf(stderr, "InputLine #%d completed but at pos %d still has char '%c'\n",
          InputLineNr, InputPtr-InputLine, *InputPtr);
      exit(3);
    }

    return Node;
} /* ParseInputLine() */

int LeftSmallerThanRight(tNode *LeftNode, tNode *RightNode)
{
    // Traverse both trees to compare them
    ChildNr = 0;
    for (;;)
    {
      // No more Children to handle at Left side ?
      if (ChildNr >= LeftNode->NrOfChildren)
      {
        // Will need to rise up, but first check about Right side
        if (ChildNr < RightNode->NrOfChildren)    return 1;
        // When rising up becomes impossible, we have completed the tree traversal
        if (!LeftNode->Parent)                    return 0;
        // Rise up, but remember which ChildNr we came from
        ChildNode = LeftNode;
        LeftNode = LeftNode->Parent;
        RightNode = RightNode->Parent;
        for (ChildNr=0; ChildNr<LeftNode->NrOfChildren; ChildNr++)
          if (ChildNode == LeftNode->Child[ChildNr])  break;
        ChildNr++;
        continue;
      }
      // So the Left still goes on strong, but perhaps the Right side has run out of Children
      if (ChildNr >= RightNode->NrOfChildren)     return 0;
      // Both Left and Right have a Child here, compare now
      LeftNode = LeftNode->Child[ChildNr];
      RightNode = RightNode->Child[ChildNr];

    // Debugging
if (0)
{
    printf("----- After descending -----\n");
    printf("LeftNode[%d] RightNode[%d]\n", LeftNode->Parent->NrOfChildren, RightNode->Parent->NrOfChildren);
    if ((LeftNode->Parent->NrOfChildren > 2) && (LeftNode->Parent->Child[2]->Type==eNum))
      printf("LeftNode Child #2 = %d\n", LeftNode->Parent->Child[2]->Num);
    if ((RightNode->Parent->NrOfChildren > 2) && (RightNode->Parent->Child[2]->Type==eNum))
      printf("RightNode Child #2 = %d\n", RightNode->Parent->Child[2]->Num);
}

      // Four possible comparison scenarios
      if ((LeftNode->Type==eNum) && (RightNode->Type==eNum))
      {
        // Numeric comparison is possible here
        printf("Comparing LeftNum %d to RightNum %d\n", LeftNode->Num, RightNode->Num);
        if (LeftNode->Num < RightNode->Num)       return 1;
        else if (LeftNode->Num > RightNode->Num)  return 0;
        else
        {
          // Both sides rise back up a level (guaranteed to exist) ..
          LeftNode = LeftNode->Parent;
          RightNode = RightNode->Parent;

    // Debugging
if (0)
{
    printf("+++++ After rising +++++\n");
    printf("LeftNode[%d] RightNode[%d]\n", LeftNode->NrOfChildren, RightNode->NrOfChildren);
    if ((LeftNode->NrOfChildren > 2) && (LeftNode->Child[2]->Type==eNum))
      printf("LeftNode Child #2 = %d\n", LeftNode->Child[2]->Num);
    if ((RightNode->NrOfChildren > 2) && (RightNode->Child[2]->Type==eNum))
      printf("RightNode Child #2 = %d\n", RightNode->Child[2]->Num);
}

          // .. and continue to the next Child on that level
          //    (which is a sibling to the identical Children just compared)
          ChildNr++;
        }
      }
      else if ((LeftNode->Type==eList) && (RightNode->Type==eList))
        // So, we'll need to descend into these at both sides
        ChildNr = 0;
      else if (LeftNode->Type==eNum)
      {
        // Right side has a list here : convert Left side into a single Num as well
        ChildNode = (tNode*)malloc(sizeof(tNode));
        ChildNode->Type = eNum;
        ChildNode->Num = LeftNode->Num;
        ChildNode->NrOfChildren = 0;
        ChildNode->Parent = LeftNode;
        LeftNode->Type = eList;
        LeftNode->NrOfChildren = 1;
        LeftNode->Child[0] = ChildNode;
        // Now we can descend into both sides equally
        ChildNr = 0;
      }
      else if (RightNode->Type==eNum)
      {
        // Right side has a list here : convert Left side into a single Num as well
        ChildNode = (tNode*)malloc(sizeof(tNode));
        ChildNode->Type = eNum;
        ChildNode->Num = RightNode->Num;
        ChildNode->NrOfChildren = 0;
        ChildNode->Parent = RightNode;
        RightNode->Type = eList;
        RightNode->NrOfChildren = 1;
        RightNode->Child[0] = ChildNode;
        // Now we can descend into both sides equally
        ChildNr = 0;
      }
    } /* for (;;) */
} /* int LeftSmallerThanRight(LeftNode, RightNode) */

// = = = = = = = = = = = = = =      i n t   M a i n ( )     = = = = = = = = = = = = = =

int main(int Arguments, char* Argument[])
{
	FILE* InputFile = stdin;

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

  int NodeNr,NrOfNodes=0;
  tNode *SortNode[500], *NewNode, *TempNode;

	char Divider[2][10] = {"[[2]]", "[[6]]"};
	int DividerNr=0,NrOfDividers=2;
	int DividerProd=1;

	// Parse the input
	while ((fgets(InputLine, 1000, InputFile)) ||
			((DividerNr < NrOfDividers) && (strcpy(InputLine, Divider[DividerNr++]))))
	{
		// Bookkeeping
		InputLineNr++;
		// Strip line ending
		while ((InputLine[strlen(InputLine)-1]=='\n')
				|| (InputLine[strlen(InputLine)-1]=='\r'))  InputLine[strlen(InputLine)-1]='\0';
		if (!strlen(InputLine)) continue;

    /******************/
    /* Data gathering */
    /******************/
		printf("== Node %d ==\n", NrOfNodes+1);
    NewNode = ParseInputLine();

    /*******************/
    /* Data processing */
    /*******************/
    for (NodeNr=0; NodeNr<NrOfNodes; NodeNr++)
    // Perform a comparison to see where NewNode fits in
      if (LeftSmallerThanRight(NewNode, SortNode[NodeNr]))  break;
    // NewNode needs to be inserted at this spot,
    //   all nodes from here will need to be bumped one spot down.
    for (int TempNodeNr=NrOfNodes; TempNodeNr>NodeNr; TempNodeNr--)
      SortNode[TempNodeNr] = SortNode[TempNodeNr-1];
    SortNode[NodeNr] = NewNode;
if (1)
{
		// Debugging
		printf("Inserted NewNode[%d] at array position %d\n", NewNode->NrOfChildren, NodeNr);
}
    NrOfNodes++;
		if (DividerNr)  DividerProd *= (NodeNr+1);
	} /* while (fgets) */
	printf("%d InputLines were read.\n", InputLineNr);
	fclose(InputFile);

	/*************/
	/* Reporting */
	/*************/
  for (NodeNr=0; NodeNr<NrOfNodes; NodeNr++)
    printf("#%2d: Node[%d]\n", NodeNr, SortNode[NodeNr]->NrOfChildren);
	printf("Divider positions product: %d\n", DividerProd);
	return 0;
}
