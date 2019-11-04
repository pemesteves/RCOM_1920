// Serial port settings
#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

// General macros
#define MAX_REJECTS 3
#define BIT(n) 1 << n

// State machine macros
#define START_STATE     0
#define RECEIVED_FLAG   1
#define RECEIVED_A      2
#define RECEIVED_CTRL   3
#define CORRECT_BCC     4
#define STOP_STATE      5


// Plot macros
#define FLAG        0x7e
#define ESCAPE      0x7D
#define A           0x03
#define C_SET       0x03
#define C_DISC      0x0B
#define C_UA        0x07
#define C_RR        0x05
#define C_SEND_0    0x00
#define C_SEND_1    0x40
#define C_RR_0      0x05
#define C_RR_1      0x85
#define C_REJ_0     0x01
#define C_REJ_1     0x81