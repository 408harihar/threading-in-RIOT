/*
 * Copyright (C) 2020 TUBA Freiberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "thread.h"



// defined in thread_config.h [THREAD_STACKSIZE_MAIN]
char test_thread_stack[THREAD_STACKSIZE_MAIN];

void* check_grid(void* params);

// Prototype for the check_rows function.
void* check_rows(void* params);

// Prototype for the check_cols function.
void* check_cols(void* params);

int sudoku_checker(int sudoku[9][9]);
int check_line(int input[9]);
int chk_grid(int sudoku[9][9]);

typedef struct
{
    // The starting row.
    int row;
    // The starting column.
    int col;
    // The pointer to the sudoku puzzle.
    int(*board)[9];

} parameters;

kernel_pid_t thread_ID [27] = {0};

static clock_t start[28], end[28];
static double cpu_time_used; 

int main(void)
{
    printf("\nMain thread ID:%" PRIkernel_pid "\n", thread_getpid());

    int sudoku[9][9] =
    {
            {6, 2, 4, 5, 3, 9, 1, 8, 7},
            {5, 1, 9, 7, 2, 8, 6, 3, 4},
            {8, 3, 7, 6, 1, 4, 2, 9, 5},
            {1, 4, 3, 8, 6, 5, 7, 2, 9},
            {9, 5, 8, 2, 4, 7, 3, 6, 1},
            {7, 6, 2, 3, 9, 1, 4, 5, 8},
            {3, 7, 1, 9, 5, 6, 8, 4, 2},
            {4, 9, 6, 1, 8, 2, 5, 7, 3},
            {2, 8, 5, 4, 7, 3, 9, 1, 6}
    };
    
    start[0] = clock();
    bool test_checker_single_thread = sudoku_checker(sudoku);
    end[0] = clock();
    cpu_time_used = ((double) (end[0] - start[0])) / CLOCKS_PER_SEC;
    printf("the time used is: %f\n",cpu_time_used);
    
    if (test_checker_single_thread){
        printf("Sudoku solution is invalid\n");
    }
    else{
        printf("Sudoku solution is valid\n");
    }


    int thread_index = 0;

    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            // ====== Declaration of the parameter for the 3X3 grid check threads =======
            if (i % 3 == 0 && j % 3 == 0)
            {
                parameters* gridData = (parameters*)malloc(sizeof(parameters));
                gridData->row = i;
                gridData->col = j;
                gridData->board = sudoku;
                start[thread_index+1] = clock();
                thread_ID[thread_index] = thread_create(
                    test_thread_stack, 
                    sizeof(test_thread_stack), 
                    THREAD_PRIORITY_MAIN - 1, 
                    THREAD_CREATE_STACKTEST, 
                    check_grid, 
                    gridData, 
                    "Check Grid"
                    );
                end[thread_index+1] = clock();
                thread_index ++;
            }

            if (j == 0)
            {
                parameters* rowData = (parameters*)malloc(sizeof(parameters));
                rowData->row = i;
                rowData->col = j;
                rowData->board = sudoku;
                start[thread_index+1] = clock();
                thread_ID[thread_index] = thread_create(
                    test_thread_stack, 
                    sizeof(test_thread_stack), 
                    THREAD_PRIORITY_MAIN - 1, 
                    THREAD_CREATE_STACKTEST, 
                    check_rows, 
                    rowData, 
                    "Check Rows"
                    );
                end[thread_index+1] = clock();
                thread_index ++;
            }

            if (i == 0)
            {
                parameters* columnData = (parameters*)malloc(sizeof(parameters));
                columnData->row = i;
                columnData->col = j;
                columnData->board = sudoku;
                start[thread_index+1] = clock();
                thread_ID[thread_index] = thread_create(
                    test_thread_stack, 
                    sizeof(test_thread_stack), 
                    THREAD_PRIORITY_MAIN - 1, 
                    THREAD_CREATE_STACKTEST, 
                    check_cols, 
                    columnData, 
                    "Check Columns"
                    );
                end[thread_index+1]  = clock();
                thread_index ++;
            }
        }
    }

    printf("\nTime to run multiple threads\n");
    for (int t=0; t<27; t++)
    {
        printf("thread %d tooke %f time for completion \n", t+3, ((double) (end[t+1] - start[t+1])) / CLOCKS_PER_SEC);
    }

}

// ------------------------------end main------------------
void* check_grid(void* params)
{
    printf("Thread ID: %" PRIkernel_pid " \n",thread_getpid());
    parameters* data = (parameters*)params;
    int startRow = data->row;
    int startCol = data->col;
    int validarray[10] = { 0 };
    for (int i = startRow; i < startRow + 3; ++i)
    {
        for (int j = startCol; j < startCol + 3; ++j)
        {
            int val = data->board[i][j];
            // printf("%d ",val);
            if (validarray[val] != 0)
            {
                // printf("\n***********************\n");
                // printf("Repeatation found inside grid row:[%d] col:[%d] for [REPEATED] number %d \n",startRow+1,startCol+1,val);
                // printf("Validity Check fail at Thread %" PRIkernel_pid "\n \n",thread_getpid());
                thread_zombify();
                thread_kill_zombie(thread_getpid());
                return NULL;
            }
            else
            {
                validarray[val] = 1;
            }
        }
    }

    // If the execution has reached this point, then the 3x3 sub-grid is valid.
    thread_ID[startRow + startCol / 3] = 1; // Maps the 3X3 sub-grid to an index in the first 9 indices of the result array
    thread_zombify();
    thread_kill_zombie(thread_getpid());
    return NULL;
}


void* check_rows(void* params)
{
    printf("Thread ID: %" PRIkernel_pid " \n",thread_getpid());
    parameters* data = (parameters*)params;
    int row = data->row;

    int validarray[10] = { 0 };
    for (int j = 0; j < 9; j++)
    {
        int val = data->board[row][j];
        if (validarray[val] != 0)
        {
            thread_zombify();
            thread_kill_zombie(thread_getpid());
        }
        else
        {
            validarray[val] = 1;
        }
    }

    // If the execution has reached this point, then the row is valid.
    thread_ID[9 + row] = 1; // Maps the row to an index in the second set of 9 indices of the result array
    thread_zombify();
    thread_kill_zombie(thread_getpid());
    return NULL;
}

void* check_cols(void* params)
{
    printf("Thread ID: %" PRIkernel_pid " \n",thread_getpid());
    parameters* data = (parameters*)params;
    //int startRow = data->row;
    int col = data->col;

    int validarray[10] = { 0 };
    for (int i = 0; i < 9; i++)
    {
        int val = data->board[i][col];
        if (validarray[val] != 0)
        {
            thread_zombify();
            thread_kill_zombie(thread_getpid());
        }
        else
            validarray[val] = 1;
    }

    // If the execution has reached this point, then the column is valid.
    thread_ID[18 + col] = 1; // Maps the column to an index in the third set of 9 indices of the result array
    thread_zombify();
    thread_kill_zombie(thread_getpid());
    return NULL;
}


int chk_grid(int sudoku[9][9])
{
    printf("\tThread ID in single thread operation:%" PRIkernel_pid "\n", thread_getpid());
    int temp_row, temp_col;

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            temp_row = 3 * i;
            temp_col = 3 * j;
            int validarray[10] = { 0 };

            for (int p = temp_row; p < temp_row + 3; p++)
            {
                for (int q = temp_col; q < temp_col + 3; q++)
                {
                    int val = sudoku[p][q];
                    if (validarray[val] != 0)
                        return 1;
                    else
                        validarray[val] = 1;
                }
            }
        }
    }
    return 0;
}




int check_line(int input[9])
{
    printf("\thread ID in single thread operation:%" PRIkernel_pid "\n", thread_getpid());
    int validarray[10] = { 0 };
    for (int i = 0; i < 9; i++)
    {
        int val = input[i];
        if (validarray[val] != 0)
            return 1;
        else{
            validarray[val] = 1;
        }
    }
    return 0;
}


int sudoku_checker(int sudoku[9][9])
{
    for (int i = 0; i < 9; i++)
    {
        /* check row */
        if (check_line(sudoku[i]))
            return 1;

        int check_col[9];
        for (int j = 0; j < 9; j++)
            check_col[j] = sudoku[i][j];

        /* check column */
        if (check_line(check_col))
            return 1;

        /* check grid */
        if (chk_grid(sudoku))
            return 1;
    }
    return 0;
}