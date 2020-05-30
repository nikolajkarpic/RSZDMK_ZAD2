#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#define BR_KORISNIKA 10

 char korisnici[BR_KORISNIKA][32] =

{

    "Sundjer Bob Kockalone",

    "Dijego Armando Maradona",

    "Bond. Dzejms bond.",

    "Zoran Kostic Cane",

    "Kim Dzong Un",

	"Zeifon Biblebroks",

	"Goku",

	"Klark Kent",

	"Toni Stark",

	"Vito Korleone"

};

char PIN[BR_KORISNIKA][5] = {"5346", "2133", "7445", "8756", "7435", "4242", "1234", "4637", "2020", "1972"};

//Velicina prijemnog bafera (mora biti 2^n)
#define USART_RX_BUFFER_SIZE 64

char Rx_Buffer[USART_RX_BUFFER_SIZE];			//prijemni FIFO bafer
volatile unsigned char Rx_Buffer_Size = 0;	//broj karaktera u prijemnom baferu
volatile unsigned char Rx_Buffer_First = 0;
volatile unsigned char Rx_Buffer_Last = 0;

ISR(USART_RX_vect)
{
  	Rx_Buffer[Rx_Buffer_Last++] = UDR0;		//ucitavanje primljenog karaktera
	Rx_Buffer_Last &= USART_RX_BUFFER_SIZE - 1;	//povratak na pocetak u slucaju prekoracenja
	if (Rx_Buffer_Size < USART_RX_BUFFER_SIZE)
		Rx_Buffer_Size++;					//inkrement brojaca primljenih karaktera
}

void usartInit(unsigned long baud)
{
	UCSR0A = 0x00;	//inicijalizacija indikatora
					//U2Xn = 0: onemogucena dvostruka brzina
					//MPCMn = 0: onemogucen multiprocesorski rezim

	UCSR0B = 0x98;	//RXCIEn = 1: dozvola prekida izavanog okoncanjem prijema
					//RXENn = 1: dozvola prijema
					//TXENn = 1: dozvola slanja

	UCSR0C = 0x06;	//UMSELn[1:0] = 00: asinroni rezim
					//UPMn[1:0] = 00: bit pariteta se ne koristi
					//USBSn = 0: koristi se jedan stop bit
					//UCSzn[2:0] = 011: 8bitni prenos

	UBRR0 = F_CPU / (16 * baud) - 1;

	sei();	//I = 1 (dozvola prekida)
}

unsigned char usartAvailable()
{
	return Rx_Buffer_Size;		//ocitavanje broja karaktera u prijemnom baferu
}

void usartPutChar(char c)
{
	while(!(UCSR0A & 0x20));	//ceka da se setuje UDREn (indikacija da je predajni bafer prazan)
	UDR0 = c;					//upis karaktera u predajni bafer
}

void usartPutString(char *s)
{
	while(*s != 0)				//petlja se izvrsava do nailaska na nul-terminator
	{
		usartPutChar(*s);		//slanje tekuceg karaktera
		s++;					//azuriranje pokazivaca na tekuci karakter
	}
}

void usartPutString_P(const char *s)
{
	while (1)
	{
		char c = pgm_read_byte(s++);	//citanje sledeceg karaktera iz programske memorije
		if (c == '\0')					//izlazak iz petlje u slucaju
			return;						//nailaska na terminator
		usartPutChar(c);				//slanje karaktera
	}
}

char usartGetChar()
{
	char c;

	if (!Rx_Buffer_Size)						//bafer je prazan?
		return -1;
  	c = Rx_Buffer[Rx_Buffer_First++];			//citanje karaktera iz prijemnog bafera
	Rx_Buffer_First &= USART_RX_BUFFER_SIZE - 1;	//povratak na pocetak u slucaju prekoracenja
	Rx_Buffer_Size--;							//dekrement brojaca karaktera u prijemnom baferu

	return c;
}

unsigned char usartGetString(char *s)
{
	unsigned char len = 0;

	while(Rx_Buffer_Size) 			//ima karaktera u faferu?
		s[len++] = usartGetChar();	//ucitavanje novog karaktera

	s[len] = 0;						//terminacija stringa
	return len;						//vraca broj ocitanih karaktera
}

int main()
{
	int i;
	int k;
	usartInit(9600);
  	char ime[32];
  	char pin[4];
  	char pin_temp;
	while(1)
	{

		usartPutString("Unesite svoje ime.\r\n");
		while(!usartAvailable());
		_delay_ms(100);
		usartGetString(ime);
		for (i = 0; i < BR_KORISNIKA; i++){
			if(strcmp(ime, korisnici[i]) == 0){
				//while(!usartAvailable());
				usartPutString("Unesite svoj pin broj po broj.\r\n");
				for (k = 0; k<4; k++){
					while(!usartAvailable());
					_delay_ms(10);
					pin_temp = usartGetChar();
					pin [k] = pin_temp;
					usartPutChar('*');
					if (k == 3){
						usartPutString({"\r\n"});
						pin [4] = '\0';
					}
				}
				if(strcmp(PIN[i], pin)==0){
					usartPutString("Unet pin je tacan.\r\n");
				}else{
					usartPutString("Unet pin nije tacan.\r\n");

				}

			}
		}

	}

	return 0;
}
