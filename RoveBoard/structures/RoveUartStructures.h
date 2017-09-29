#ifndef ROVEUARTSTRUCTURES_H_
#define ROVEUARTSTRUCTURES_H_

typedef enum {
    ROVE_BOARD_ERROR_SUCCESS = 0,
    ROVE_BOARD_ERROR_UNKNOWN = -1
} roveBoard_ERROR;

typedef struct roveUART_Handle
{
	unsigned int uart_index;
} roveUART_Handle;

#endif
