#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sudoku_board.h"
#include "doubly_linked_list.h"
#include "game_logic.h"
#include "auxiliary_functions.h"
#include "SPBufferset.h"
#define COMMAND_LENGTH 257
/*parses user commands and calls the functions from game_logic (maybe)*/
/*take strcmp at the beginning once, check if it's possible*/
int parseCommand(char* userInput, Board* board){
	char *endptr1, *endptr2, *endptr3, *firstArgument, *command;
	int firstArgumentAsInt, secondArgumentAsInt, thirdArgumentAsInt, retVal;
	command = strtok(userInput," \t\r\n");
	firstArgument = strtok(NULL," \t\r\n");
	firstArgumentAsInt = (int)strtol(firstArgument,&endptr1,10);
	secondArgumentAsInt = (int)strtol(strtok(NULL," \t\r\n"),&endptr2,10);
	thirdArgumentAsInt = (int)strtol(strtok(NULL," \t\r\n"),&endptr3,10);
	if (strcmp(command,"solve") == 0) {
		if (firstArgument == NULL) return 0; /*check validity of argument: argument is not NULL*/
		return solve(firstArgument, board); /*if we got here argument is valid, call the command's function*/
	}
	else if (strcmp(command,"edit") == 0) {
		return edit(firstArgument, board); /*no need to check validity, if NULL then solve knows its with 0 argument*/
	}
	else if (strcmp(command,"mark_errors") == 0) {
		if (board->mode != 1) return 0;
		if (endptr1 == NULL) return -3; /*check validity: check if argument is zero (atoi function problem)*/
		else return mark_errors(firstArgumentAsInt, board); /*mark errors will check if it is 0/1 or not*/
	}
	else if (strcmp(command,"print_board") == 0) {
		if (board->mode == 0) return 0; /*not allowed in init mode*/
		return print_board(board);
	}
	else if (strcmp(command, "set") == 0) {
		if (board->mode == 0) return 0; /*not allowed in init mode*/
		if (endptr1 == NULL || endptr2 == NULL || endptr3 == NULL) return -4; /*verify that all arguments are integers*/
		return set(secondArgumentAsInt-1,firstArgumentAsInt-1,thirdArgumentAsInt,board); /*set will check if the arguments are in range*/
	}
	else if (strcmp(command,"validate") == 0) {
		if (board->mode == 0) return 0; /*not allowed in init mode*/
		return validate(board);
	}
	else if (strcmp(command,"generate") == 0) {
		if (board->mode != 2) return 0; /*only allowed in edit mode*/
		if (endptr1 == NULL || endptr2 == NULL || firstArgumentAsInt > numberOfBlankCells(board)) return -8;
		/*asked in the forum if need to check y is a valid int (same we did for x). in the instructions they say only for x but
		 * we think its a mistake and should be checked for y's value also*/
		if (numberOfFilledCells(board) != 0) return -9;
		return generate(board,firstArgumentAsInt,secondArgumentAsInt);
	}
	else if (strcmp(command,"undo") == 0) {
		if (board->mode == 0) return 0; /*not allowed in init mode*/
		return undo(board);
	}
	else if (strcmp(command,"redo") == 0) {
		if (board->mode == 0) return 0; /*not allowed in init mode*/
		return redo(board);
	}
	else if (strcmp(command,"save") == 0) {
		if (board->mode == 0) return 0; /*not allowed in init mode*/
		retVal = save(board,firstArgument);
		if (retVal==1) printf("Save to: %s\n", firstArgument);
		return retVal;
	}
	else if (strcmp(command, "hint") == 0) {
		if (board->mode != 1) return 0; /*only allowed in solve mode*/
		if (endptr1 == NULL || endptr2 == NULL) return -4; /*verify that all arguments are integers*/
		return hint(board,secondArgumentAsInt-1,firstArgumentAsInt-1);
	}
	else if (strcmp(command, "num_solutions") == 0) {
		if (board->mode == 0) return 0; /*not allowed in init mode*/
		return num_solutions(board);
	}
	else if (strcmp(command, "autofill") == 0) {
		if (board->mode != 1) return 0; /*only allowed in solve mode*/
		return autofill(board,1,0,0,0);
	}
	else if (strcmp(command, "reset") == 0) {
		if (board->mode == 0) return 0; /*not allowed in init mode*/
		return reset(board);
	}
	else if (strcmp(command, "exit") == 0) {
		return userExit(board); /*available in all modes*/
	}

	return 1;
}

int interact(Board* board){
	Node* autofillCurrNode;
	char userInput[COMMAND_LENGTH] = {0};
	printf("Enter your command\n");
	fgets(userInput,COMMAND_LENGTH,stdin);
	if (userInput[COMMAND_LENGTH-1] != '\0') {
		printf("ERROR: invalid command\n");
	}

	/*calls read_command and prints errors if necessary (board->autofillCounter < board->autofillNumberOfCells)*/
	switch(parseCommand(userInput,board)) {
		case(11):
			/* the last node added is a list of the autofill set moves. we start with the head of the list
			 * and iterate on the entire list, in each iteration performing an autofill-set move, printing
			 * a message to the user and moving to next node */
			autofillCurrNode = board->movesList->tail->step->list->head;
			while (autofillCurrNode != NULL) {
				autofill(board,0,autofillCurrNode->step->i,
						autofillCurrNode->step->j,
						autofillCurrNode->step->new);

				printf("Cell <%d,%d> set to %d\n", autofillCurrNode->step->j,
											autofillCurrNode->step->i,
											autofillCurrNode->step->new);

				autofillCurrNode = autofillCurrNode->next;
			}
			print_board(board);
			break;
		case(10):
			printf("Puzzle solved successfully\n");
			break;
		case (9):
			printf("Puzzle solution erroneous\n");
			break;
		case (8):
			printf("Exiting...\n");
			return -2;
			break;
		case (7):
			printf("Board reset\n");
			break;
		case (6):
			printf("Number of solutions: %d\n", board->lastNumOfSolutions);
			if (board->lastNumOfSolutions == 1) printf("This is a good board!\n");
			else if (board->lastNumOfSolutions > 1) printf("The puzzle has more than 1 solution, try to edit it further\n");
			break;
		case (5):
			printf("Hint: set cell to %d\n", board->lastHint);
			break;
		case (4):
			/*check if the last move was set (and not autofill)*/
			if (board->currNode->next->step->list == NULL) {
				printSetUndo(board, 1);
			}
			else {
				printAutofillUndo(board, 1);
			}
			break;
		case (3):
			/*check if the last undo was set (and not autofill)*/
			if (board->currNode->next->step->list == NULL) {
				printSetUndo(board, 0);
			}
			else {
				printAutofillUndo(board, 0);
			}
			break;
		case (2):
			printf("Validation passed: board is solvable\n");
			break;
		case (0):
			printf("ERROR: invalid command\n");
			break;
		case (-1):
			printf("Error: File doesn't exist or cannot be opened\n");
			break;
		case (-2):
			printf("Error: File cannot be opened\n");
			break;
		case (-3):
			printf("Error: the error should be 0 or 1\n");
			break;
		case (-4):
			printf("Error: value not in range 0-%d\n",board->edgeSize);
			break;
		case (-5):
			printf("Error: cell is fixed\n");
			break;
		case (-6):
			printf("Error: board contains erroneous values\n");
			break;
		case (-7):
			printf("Validation failed: board is unsolvable\n");
			break;
		case (-8):
			printf("Error: value not in range 0-%d\n",numberOfBlankCells(board));
			break;
		case (-9):
			printf("Error: board is not empty\n"); /*in the instructions file (in generate) there is a probably mistaken extra space */
			break;
		case (-10):
			printf("Error: puzzle generator failed\n");
			break;
		case (-11):
			printf("Error: no moves to undo\n");
			break;
		case (-12):
			printf("Error: no moves to redo\n");
			break;
		case (-13):
			printf("Error: board validation failed\n");
			break;
		case (-14):
			printf("Error: cell already contains a value\n");
			break;
		case (-15):
			printf("Error: board is unsolvable\n");
			break;
	}
	return 1;
}



