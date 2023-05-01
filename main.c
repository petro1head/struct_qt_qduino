#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "uart.h"
#include "chip_vm014.h"
#include "devdescr.h"
#include "chip.h"
#include "io.h"
#include "burk_function.h"
#include "stdint.h"

int _getpid_r(struct _reent *a) {
	return 0;
}

int _kill_r(struct _reent *a, int b, int c) {
	return 0;
}

_PTR _calloc_r(struct _reent *a, size_t b, size_t c) {
	return 0;
}

void _free_r(struct _reent * a, _PTR v) {}

_PTR _malloc_r (struct _reent * a, size_t c)
{
	return 0;
}

_PTR _realloc_r (struct _reent * q, _PTR e, size_t t)
{
	return 0;
}

int _close_r(struct _reent * q, int w)
{
	return 0;
}

_off_t _lseek_r (struct _reent * q, int r, _off_t u, int i)
{
	return 0;
}
_ssize_t _read_r (struct _reent *q, int w, void * e, size_t t)
{
	return 0;
}
_ssize_t _write_r  (struct _reent * y, int i, const void * p, size_t x)
{
	return 0;
}

//typedef struct Buffer {
//	char str[100];
//	int len;
//} Buffer;

#pragma pack(push, 1)
typedef struct Data {
  unsigned long t;
  double speed;
} Data;
#pragma pack(pop)



void uart_send_double(int uart_n, double value) {
    // Преобразуем число в последовательность байт
	unsigned char* bytes = (unsigned char*)&value;
    int len = sizeof(double);

    // Отправляем каждый байт по отдельности
    for (int i = 0; i < len; i++) // младшим вперед 
		// for (int i = len - 1; i >= 0; i--) // старший бит вперед
	{
        uart_send_byte(uart_n, bytes[i]);
    }
	
	
}

void outPrintDouble(MySystem *ps) {
	uart_send_double(0, SystemOut(ps));
//	// Serial.println(SystemOut());
//	// uart_send_number(0, SystemOut(ps));
//	char string [30];
//	sprintf (string, "%lf\n",SystemOut(ps));
//	uart_send_string(0, (unsigned char*) string);
}

unsigned char readed = 0;
// получаем структуру
Data data;
int get_data() {
	char* bWrite = (char*)&data;
			bWrite += readed;
			while (uart_check_in(0))
			{
			    if (readed >= sizeof(Data))
			    {
			      readed = 0;
			      return 1; // true
			    }
			    *bWrite++ = uart_read_byte(0);
			    readed++;
			}
			  return 0; // false
}

int main(void) {

/*------------uart-------------*/
	volatile UART_UNIT *uart0 = (UART_UNIT*) 0xBFF00100;
	uart0->IIRF = 0xff; //fifo
	uart0->IIRF = 0xff;
	uart_baud_set(0, 13); // Baud115200  (24000000/16/115200)
	uart0->LCR = 3; //  8 bit data, no parity, 1 stop bit // uart1->LCR = (1<<0)|(1<<1)

	//unsigned long t = 0;
	//double speed = 0;

//	Buffer buf;
//	// заполнение 0 полей структуры Buffer
//	memset(buf.str,0,sizeof(buf.str));
//	buf.len = 0;
//
//	char qt_data[100]= {0};
//	// flag that we readed data
//	int readed_data = 0;
//	// char for read data
//	char c = 0;
data.speed=0;
data.t=0;
	MySystem s;
	set_default(&s);

	char ok_string[5] = "OK";
	strcat(ok_string, "\n");
	uart_send_string(0, (unsigned char*)ok_string);

	//uart_send_string(0, (unsigned char*) "OK");

	while (1) {
			if (get_data())
			{
				SystemRun(&s, data.t, data.speed);
				// GET end value
				// send the string with a signal U
				outPrintDouble(&s);
			}
	}


//	while (1) {
//		if (uart_check_in(0))
//		{
//			c = uart_read_byte(0);
//			if (c == '\n')
//			{
//				// Cut the data from QT
//				buf.str[buf.len] = '\0';
//				// Copy data from buffer
//				strcpy(qt_data, buf.str);
//				// Clear buffer
//				buf.str[0] = '\0';
//				buf.len = 0;
//				// up flag that we readed data
//				readed_data = 1;
//
//			} else {
//				// data don't ended
//				// add symbol to the string
//				buf.str[buf.len] = c;
//				buf.len++;
//			}
//		}
//
//		if (readed_data)
//		{
//			// down flag for new read data
//			readed_data = 0;
//			if (sscanf(qt_data, "%lu;%lf", &t, &speed))
//			{
//				SystemRun(&s, t, speed);
//				// GET end value
//				// send the string with a signal U
//				outPrintln(&s);
//			}
//
//			else
//			{
//				//char bad_string[100] = "bad";
//				char bad_string[20];
//				sprintf (bad_string, "bad%s\n", qt_data);
//				uart_send_string(0, (unsigned char*)bad_string);
//			}
//		}
//
//	}
	return 0;
}

