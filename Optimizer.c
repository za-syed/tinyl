/*
 *********************************************
 *  314 Principles of Programming Languages  *
 *  Fall 2016                              *
 *********************************************
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "InstrUtils.h"
#include "Utils.h"

int outputAI(Instruction *, int);
void aiCall(Instruction *, int);
void opCall(Instruction *, int);
void freeReg(Instruction *);


//int main()
//{
//	Instruction *head;
//
//	head = ReadInstructionList(stdin);
//	if (!head) {
//		WARNING("No instructions\n");
//		exit(EXIT_FAILURE);
//	}
//	/* YOUR CODE GOES HERE */
//	if (head) {
//		PrintInstructionList(stdout, head);
//		DestroyInstructionList(head);
//	}
//	return EXIT_SUCCESS;
//}
// Prototypes

int main()
{
	Instruction *head, *current, *temp;
	int storeNum = 0, index = 0;
	int inputSet, outputSet, aiFlag = -1, loadFlag = -1;
	int regOne, regTwo;
	//char* filename = "tinyL.out";
	//FILE *infile;
    //infile = fopen(filename, "r");
    PrintInstructionList(stdout, head);
   // head = ReadInstructionList(infile);
    if (!head) {
	WARNING("No instructions\n");
	exit(EXIT_FAILURE);
	}   
	
	// Point to head
	current = head;

	/* YOUR CODE GOES HERE */
	while(current != NULL)
	{
		// Get number of variables
		if(current->opcode == STORE)
			storeNum++;

		// Move to next node
		current = current->next;
	}

	// Holds number of store statements
	int storeSet[storeNum], storeReg[storeNum];

	// Move back to head
	current = head;

	// Travere to get field1 and offset in every STOREAI
	while(current != NULL && current->next != NULL)
	{
		if(current->opcode == STORE)
		{
			storeReg[index] = current->field1;
			storeSet[index++] = current->field3;
		}

		current = current->next;
	}

	// Check
	while(current != NULL)
	{
		switch(current->opcode)
		{
			case WRITE:
				// Save offset of OUTPUTAI, go to previous node and set to critical
				outputSet = current->field2;
				current->critical = 'c';

				// Previous opcode doesn't match OUTPUTAI
				// Get index of previous STOREAI
				if(!outputAI(current->prev, outputSet))
					index = index - 1;
				break;
        	case READ:
				// Save offset of READ, go to previous node and set to critical
				inputSet = current->field2;
				current->critical = 'c';

				// Previous opcode doesn't match WRITE
				// Get index of previous STORE
				if(!inputAI(current->prev, inputSet))
					index = index - 1;
				break;

			case STORE:
				// Get index of STOREAI
				index = index - 1;
				temp = current;

				current = current->prev;

				// Check for LOADI
				while(current != NULL && current->opcode != STORE && aiFlag == -1)
				{
					// LOADI found, break
					if(current->opcode == LOADI)
						loadFlag = 1;
					else
						loadFlag = 2;

					// Go back to check
					current = current->prev;
				}

				// Set pointer
				if(loadFlag == 1)
					current = temp;
				else
				{
					aiFlag = 0;
					
					if(current->prev != NULL)
					{
						current = current->prev;
						current->critical = 'x';
					}
				}

				// If marked not critical remove everything above it
				if(current->critical == 'x' || loadFlag == 2)
				{
					while(current->prev != NULL && current->prev->opcode != STORE)
					{
						if(current->prev != NULL)
							current = current->prev;

						current->critical = 'x';
					}
				}
				else if(current->critical == 'c')
				{
					temp = current;
					current = current->prev;

					while(current != NULL)
					{
						if(current->opcode == STORE && current->field3 == temp->field3)
							current->critical = 'x';

						current = current->prev;
					}

					current = temp;
				}

				break;

			case LOADI:
				if(current->next->opcode == STORE && current->field2 == current->next->field1)
					current->critical = 'c';
				else if(current->next->opcode == STORE && current->field2 != current->next->field1)
					current->critical = 'x';
				break;
			case LOAD:
				// Loop above til you find storeai and check offset
				aiCall(current->prev, current->field2);
				break;
			case ADD:
			case SUB:		
			case MUL:
				// Save fields
				temp = current;
				regOne = current->field1;
				regTwo = current->field2;

				// Mark as critical since it matches STORE
				if(current->next->opcode == STORE && current->field3 == storeReg[index])
					current->critical = 'x';

				// Recursive call to check each register
				opCall(current->prev, regOne);
				current = temp;
				opCall(current->prev, regTwo);
				current = temp;

				break;

			default:
				break;
		}

		// Now lean back
		current = current->prev;
	}
    	// Free the nodes
	if(head!=NULL){	
	freeReg(head);
	}
	// Print optimized code
	if (head != NULL)
	{
	    PrintInstructionList(stdout, head);
		DestroyInstructionList(head);
	}


	return EXIT_SUCCESS;
}

int outputAI(Instruction *current, int outputSet)
{
	// Check offset with STORE
	if(current->opcode == STORE && current->field3 == outputSet)
	{
		// Set STORE to critical
		current->critical = 'c';
		return 1;
	}
	else
	{
		// WRITE and STORE do not match, or
		// STORE does not come before OUTPUTAI
		do {
			current->critical = 'x';
			current = current->prev;
		} while(current->opcode != STORE);
	}

	return 0;
}
int inputAI(Instruction *current, int inputSet)
{
	// Check offset with STORE
	if(current->opcode == STORE && current->field3 == inputSet)
	{
		// Set STORE to critical
		current->critical = 'c';
		return 1;
	}
	else
	{
		// INPUT and STORE do not match, or
		// STORE does not come before INPUT
		do {
			current->critical = 'x';
			current = current->prev;
		} while(current->opcode != STORE);
	}

	return 0;
}


void aiCall(Instruction *current, int offset)
{
	if(current->critical != 'c')
	{
		if(current->opcode == STORE)
		{
			if(current->field3 == offset)
				current->critical = 'c';
			else
				current->critical = 'x';
		}
	}

	if(current->prev != NULL)
		aiCall(current->prev, offset);
}

void opCall(Instruction *current, int reg)
{
	// Only check if not critical
	if(current->prev != NULL && current->critical != 'c')
	{
		switch(current->opcode)
		{
			case LOADI:
				// Check if number is being used
				if(current->field2 == reg)
					current->critical = 'c';
				else
					current->critical = 'x';
				break;

			case LOAD:
				if(current->field3 == reg)
					current->critical = 'c';
				else
					current->critical = 'x';
				break;

			case ADD:
			case SUB:
			case MUL:			

			default:
				break;
		}
	}

	// Only go back if it's not STORE or OUTPUTAI
	if(current->prev != NULL && current->prev->opcode != STORE && current->prev->opcode != WRITE)
		opCall(current->prev, reg);
}

void freeReg(Instruction *head)
{
	// Temp nodes
	Instruction *temp, *current;
	current = head;
	temp = current;	
		//to remove the instructions that are marked x
		// we need to be check the following:
		// 1. If the current node is the fist node,
		//     then no need to point to the next of the previous of the
		//     current node to the next of the current node becuase there is no node 
		//     previous to the current node.
	if(current->prev ==NULL)
	   {
	   	if(current->critical != 'x'){
		
	    current->next->prev=NULL;
	    free(temp);
	    			    						
	    }
		}
	   	//2. If the current node is the last node,
		//   then no need to point the next of the previous node of the current node to 
		//    to the next of the current node. 
		 
	if(current->next ==NULL)
	{
if(current->critical != 'x') {
		current->prev->next=NULL;
	    free(temp);
					    						
		}
		}
		
	while(current != NULL)
	{		
		if(current->critical != 'x')
		{
		// Remove only not critical
		//set the current node in a temp node and
			temp = current;			       		    						
//			}			
		//3. If the current node is not the first or last then change the links.
			// Adjust links
			if(current->prev!=NULL && current->next != NULL)
			{			
			current->next->prev = current->prev;
   			current->prev->next = current->next;
            }
			// Move to next node then free temp
			current = current->next;
			free(temp);
		}
		else
			current = current->next;
	}
}

