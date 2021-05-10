#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <sys/unistd.h> // STDOUT_FILENO, STDERR_FILENO
#include <errno.h>
#include <msp430f5529.h>

void uart_write_char(char c);
void uart_init();

//if defined: the macros HIROM_FUNC and HIROM_CONST_DATA will put the function/data into HIROM.
//if not defined: lets the compiler decide. (based on options it's given, like -mcode-region=either, -mdata-region=either)
//^ see "Table 4-2. MSP430 GCC Command Options" in https://www.ti.com/lit/ug/slau646f/slau646f.pdf
#define FORCE_HIGHMEM

#ifdef FORCE_HIGHMEM
/* https://gcc.gnu.org/onlinedocs/gcc-3.2/gcc/Variable-Attributes.html */
#define HIROM_FUNC __attribute__((section(".upper.text")))
#define HIROM_CONST_DATA __attribute__((section(".upper.rodata")))
#else
#define HIROM_FUNC
#define HIROM_CONST_DATA
#endif

/* create test function in HIROM */
int HIROM_FUNC my_function_in_hirom(int a, int b)
{
	printf("Entry params: %d, %d\n", a, b);
	return a + b;
}

/* place constant in HIROM */
/* volatile only to prevent premature optimization / force read */
volatile const int constant_in_hirom HIROM_CONST_DATA = 123;

int main()
{
	uart_init();
	my_function_in_hirom(1, 2);
	#ifdef FORCE_HIGHMEM
	printf("FORCE_HIGHMEM is activated. The following functions and constants should be in high-ROM >= 0x10000.\n");
	#else
	printf("FORCE_HIGHMEM is deactivated. The following functions and constants will probably be placed in low-ROM, depending on compiler settings.\n");
	#endif	
	printf("Constant test: value %d, address %p\n", constant_in_hirom, &constant_in_hirom);
	printf("my_function_in_hirom address: %p\n", &my_function_in_hirom);

	return 0;
}

/* ===== UART init and write stuff ======= */
//per "5.4 Using printf with MSP430 GCC", https://www.ti.com/lit/ug/slau646f/slau646f.pdf
//bridge between printf() and UART output

void uart_write_char(char c)
{
	while (!(UCA0IFG & UCTXIFG))
		;		   //wait for transmit ready
	UCA0TXBUF = c; //transmit
}

void uart_write_data(const char *data, size_t len)
{
	for (size_t i = 0; i < len; i++)
	{
		uart_write_char(data[i]);
	}
}

void uart_init()
{
	//mostly from https://e2e.ti.com/support/microcontrollers/msp-low-power-microcontrollers-group/msp430/f/msp-low-power-microcontroller-forum/698353/ccs-msp430f5529-msp430f5529-uart-configuration-issues
	WDTCTL = WDTPW + WDTHOLD; //stop watchdog
	//P1.1 = RXD, P1.2=TXD, 9600 baud
	P1SEL = BIT1 + BIT2;
	P2SEL = BIT1 + BIT2;
	UCA0CTL1 |= UCSSEL_2; // SMCLK, UART clock source
	//baud rate calculation: MCU runs at 25Mhz, as per F_CPU
	const uint16_t baud = (uint16_t)((long)F_CPU / (long)9600);
	UCA0BR0 = (uint8_t)(baud & 0xff);
	UCA0BR1 = (uint8_t)(baud >> 8);
	UCA0MCTL = UCBRS0;	  // Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
	//UCB0IE |= UCRXIE;	  // activate USCI_A0 RX-Interrupt
}

//correction: documentation wants us to write "int write (int fd, const char *buf, int len);""
//but actual definition is with void* and size_t..
int write(int fd, const void *buf, size_t len)
{
	const char *data = (const char *)buf;
	//if this is for neither stdout nor stderr, it's invalid
	if ((fd != STDOUT_FILENO) && (fd != STDERR_FILENO))
	{
		errno = EBADF;
		return -1;
	}
	//write chars to UART
	uart_write_data(data, len);
	return len;
}