#include <sb.h>

int base16(char **str, unsigned *val)
/* Takes a double pointer to a string, interprets the characters as a
 * base-16 number, and advances the pointer.
 * Returns 0 if successful, 1 if not.
 */
{
   char c;
   int digit;
   *val = 0;

   while ( **str != ' ') {
      c = toupper(**str);
      if (c >= '0' && c <= '9')
         digit = c - '0';
      else if (c >= 'A' && c <= 'F')
         digit = c - 'A'  + 10;
      else
         return 1;          // error in string

      *val = *val * 16 + digit;
      (*str)++;
   }
   return 0;
}



int base10(char **str, unsigned *val)
/* Takes a double pointer to a string, interprets the characters as a
 * base-10 number, and advances the pointer.
 * Returns 0 if successful, 1 if not.
 */
{
   char c;
   int digit;
   *val = 0;

   while ( **str != ' ') {
      c = toupper(**str);
      if (c >= '0' && c <= '9')
         digit = c - '0';
      else
         return 1;          // error in string

      *val = *val * 10 + digit;
      (*str)++;
   }
   return 0;
}

unsigned sb_read_blaster_env(unsigned *port, unsigned *irq, unsigned *dma8, unsigned *dma16)
/* Gets the Blaster environment statement and stores the values in the
 * variables whose addresses were passed to it.
 * Returns:
 *   0  if successful
 *   1  if there was an error reading the port address.
 *   2  if there was an error reading the IRQ number.
 *   3  if there was an error reading the 8-bit DMA channel.
 *   4  if there was an error reading the 16-bit DMA channel.
 */
{
   char     *env;
   unsigned val;
   int      digit;

   env = getenv("BLASTER");

   while (*env) {
      switch(toupper( *(env++) )) {
         case 'A':
            if (base16(&env, port))     // interpret port value as hex
               return 1;                // error
            break;
         case 'I':
            if (base10(&env, irq))      // interpret IRQ as decimal
               return 2;                // error
            break;
         case 'D':
            if (base10(&env, dma8))     // 8-bit DMA channel is decimal
               return 3;
            break;
         case 'H':
            if (base10(&env, dma16))    // 16-bit DMA channel is decimal
               return 4;
            break;
         default:
            break;
      }
   }

   return 0;
}

int main(int argc, char *argv[])
{
	int result;
	unsigned short int	version;
	unsigned char irq_number;
	unsigned short int base_address;
	unsigned char dma_channel_8;
	char sb_model_name[25] = "";

	version = sb_get_driver_version();
	printf("Sound Blaster Driver v%i.%i\n", version >> 8, version & 0xFF);

	if(result = sb_dsp_detect_base_address(&base_address)) {
		printf("Sound Blaster not detected (%i)!\n", result);
		return(result);
	}

	if(result = sb_dsp_detect_irq_number(&irq_number)) {
		printf("Cannot detected IRQ number (%i)!\n", result);
		return(result);
	}

	if(result = sb_dsp_detect_dma_channel_8(&dma_channel_8)) {
		printf("Cannot detected DMA channel (%i)!\n", result);
		return(result);
	}
	
	sb_get_model_name(sb_model_name);

	printf("%s detected at I/O base address 0x%X, IRQ %i, DMA %i.\n", 
			sb_model_name, base_address, irq_number, dma_channel_8);


// ftp://ftp.mindcandydvd.com/pub/drivers/Creative/Sound%20Blaster%20Series%20Hardware%20Programming%20Guide/samples
	unsigned port, irq, dma8, dma16;
	sb_read_blaster_env(&port, &irq, &dma8, &dma16);
	
	printf("detected at I/O base address 0x%X, IRQ %i, 8bit DMA %i, 16bit DMA %i.\n", 
			port, irq, dma8, dma16);
}

