/*
	SB.H	by Pancheri Paolo(980812)
   http://www.geocities.com/SiliconValley/Park/8933
   mailto:DarkAngel@tin.it

   Programming the Sound Blaster
	For DJGPP 2.0
*/

#ifndef __SB_H
#define __SB_H

#include <pc.h>
#include <dpmi.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/nearptr.h>

#include "types.h"

// Error costants
#define	SB_OK       				0	// No errors
#define	SB_RESET_DSP	   		1	// Cannot reset the SB DSP
#define 	SB_DETECT_DSP				2	// Cannot detect the SB DSP I/O base address
#define  SB_NOT_INITIALIZED		3	// SB Library not initialized
#define	SB_INVALID_IRQ				4	// Invalid IRQ number
#define	SB_DOS_MEMORY				5	// Cannot allocate DOS memory buffer
#define	SB_SAMPLE_RATE				6	// Sample rate out of range
#define	SB_OPEN						7	// Cannot open a file
#define	SB_READ						8	// Cannot read from a file
#define	SB_MEMORY					9	// Cannot allocate protected mode memory
#define  SB_TOO_MANY_VOICES		10	// Diver are playing too many voices
#define	SB_INVALID_FILE			11	// Invalid (sound) file
#define  SB_USE_STEREO				12	// Stereo mode not supported

// Defining the offset of the SB I/O ports
#define 	SB_DSP_RESET				0x06		// DSP Reset Port
#define	SB_DSP_READ_DATA			0x0A		// DSP Read Data Port
#define	SB_DSP_WRITE				0x0C		// DSP Write Command/Data (out)
#define	SB_DSP_BUFFER_STATUS		0x0C		// DSP Write Buffer Status (in)
#define	SB_DSP_DATA_AVAILABLE	0x0E		// DSP Data Available

// Sound Blaster DSP Commands
#define	SB_DMA_MODE_8_SC			0x14		// DMA DAC, 8-bit (Single Cycle, Normal Speed)
#define	SB_DMA_MODE_8_AI_NS		0x1C		// Auto-Initialize DMA DAC, 8-bit (Normal Speed)
#define	SB_TIME_COSTANT			0x40		// Set Time Costant
#define	SB_DMA_BLOCK_SIZE			0x48		// Set DMA Block Size
#define  SB_DMA_MODE_8_AI_HS		0x90		// Auto-Initialize DMA DAC, 8-bit (High Speed)
#define	SB_HALT_DMA					0xD0		// Halt DMA Operation, 8-bit
#define 	SB_SPEAKER_ON				0xD1		// Turn Speaker on
#define 	SB_SPEAKER_OFF				0xD3		// Turn Speaker off
#define	SB_EXIT_DMA					0xDA		// Exit Auto-Initialize DMA Operation, 8-bit
#define	SB_DSP_VERSION				0xE1		// Gets the DSP version
#define	SB_IRQ_TRIGGER          0xF2		// Triggers SB interrupt

// IRQ interrupt numbers
#define	SB_IRQ_2     				0x0A
#define	SB_IRQ_3     				0x0B
#define	SB_IRQ_5     				0x0D
#define	SB_IRQ_7     				0x0F
#define	SB_IRQ_10    				0x72

// PIC ports addresses
#define	SB_PIC1_EOI      			0x20     // PIC 1 EOI (End Of Interrupt)
#define	SB_PIC2_EOI      			0xA0     // PIC 2 EOI
#define	SB_PIC_MASK_1    			0x21     // PIC 1 port (master)
#define	SB_PIC_MASK_2    			0xA1     // PIC 2 port (slave)

// IRQ ACK ports
#define	SB_IRQ_ACK_8       		0x0E		// ACK of the 8 bit IRQ
#define	SB_IRQ_ACK_16				0x0F		// ACK of the 16 bit IRQ

// DMA Ports
#define	SB_DMA_MASK_8				0x0A		// 8 bit DMA mask register
#define	SB_DMA_MODE_8				0x0B		// 8 bit DMA mode register
#define	SB_DMA_CLEAR_8				0x0C		// 8 bit DMA clear byte ptr
#define	SB_DMA_MASK_16				0xD4		// 16 bit DMA mask port
#define	SB_DMA_MODE_16				0xD6		// 16 bit DMA mode register
#define	SB_DMA_CLEAR_16			0xD8		// 16 bit DMA clear byte ptr

// Sound Blaster Pro+ Mixer ports and registers
#define	SB_MIXER_INDEX				0x04
#define	SB_MIXER_REGISTER			0x05
#define	SB_MIXER_RESET				0x00
#define	SB_MIXER_OUTPUT			0x0E

// Supported sound files extensions
#define	SB_EXTENSION_RAW			".RAW"
#define	SB_EXTENSION_WAV			".WAV"

// Other definitions
#define	SB_RESET_TIMEOUT			5
#define	SB_IRQ_DETECT_TIMEOUT	5
#define	SB_DOS_ALLOCATE_RETRY	16
#define	SB_HIGH_SPEED_FREQUENCY	23 * 1024
#define	SB_DOS_MEMORY_SIZE		2 * 1024		// Buffer for one channel
#define	SB_MAX_VOICES				8
#define	WAV_CHUNK_LENGTH			12
#define	SB_DRIVER_VERSION_HI		0x01
#define	SB_DRIVER_VERSION_LO		0x05

typedef struct
{
   char riffsign[4] PACKED;   // The RIFF signature (should be 'RIFF')
   int  length PACKED;        // The length of the data in the next chunk
   char wavesign[4] PACKED;   // The WAVE signature (should be 'WAVE')
   char ftmsign[4] PACKED;    // Contains the characters 'fmt'
   int  formatlength PACKED;  // Length of the data in the format chunk
   WORD waveformat PACKED;    // Wave Format
   WORD channels PACKED;      // Number of channels (1=mono, 2=stereo)
   WORD samplespersec PACKED; // Playback Frequency
   WORD averagebytes PACKED;  // average number of bytes a second
   WORD blockalign PACKED;    // block alignment of the data
   WORD formatspecific PACKED;// Format specific data area
} WAVCHUNK;

typedef struct {
   char sign[4] PACKED;   // Contains the characters 'data'
   int  length PACKED;    // Data length
} DATACHUNK;

typedef struct
{
	WORD	min_mono_8;		// Min mono sample for 8 bits
   WORD	max_mono_8;		// Max mono sample for 8 bits
   WORD	min_stereo_8;  // Min stereo sample for 8 bits
   WORD	max_stereo_8;  // Max stereo sample for 8 bits
	WORD	min_mono_16;	// Min mono sample for 16 bits
   WORD	max_mono_16;	// Max mono sample for 16 bits
   WORD	min_stereo_16; // Min stereo sample for 16 bits
   WORD	max_stereo_16; // Max stereo sample for 16 bits
   BOOL	auto_dma;		// Can use auto dma
   BOOL	stereo;			// Can use stereo mode
   BOOL	_16_bit;			// Can use 16 bit
} SB_CAPABILITY;

#define	SAMPLE_BYTE			unsigned char
#define	SAMPLE_CLIP_MIN   0
#define	SAMPLE_CLIP_MAX	255
typedef struct
{
	int				size;				// Size of the sound data
   SAMPLE_BYTE   *data;				// Pointer to the data memory area
  	BOOL				stereo;			// Mono or stereo sample data
} SAMPLE;

BOOL	sb_dsp_reset(WORD base_address);
BOOL	sb_dsp_write(BYTE value);
BOOL	sb_dsp_read(BYTE *value);
BOOL	sb_speaker_on(void);
BOOL	sb_speaker_off(void);
BOOL	sb_dsp_get_version(WORD *version);
BOOL	sb_get_model_name(char *name);
BOOL	sb_interrupt_number(BYTE irq_number, BYTE *interrupt_number);
BOOL	sb_dsp_detect_base_address(WORD *base_address);
BOOL	sb_dsp_detect_irq_number(BYTE *irq_number);
BOOL	sb_dsp_detect_dma_channel_8(BYTE	*dma_channel_8);
void	sb_set_dma_channel_8(BYTE dma_channel_8);

BOOL	sb_install_driver(WORD	frequency, BOOL use_stereo);
BOOL	sb_remove_driver(void);
WORD	sb_get_driver_version(void);

void	sb_set_max_voices(BYTE max_voices);
void	sb_reduce_amplitude(SAMPLE *sample);
void	sb_free_sample(SAMPLE *sample);
BOOL	sb_play_sample(SAMPLE *sample);
BOOL	sb_read_raw(char *filename, SAMPLE *sample);
BOOL	sb_read_wav(char *filename, SAMPLE *sample);
BOOL	sb_read_sample(char *filename, SAMPLE *sample);

BOOL	sb_mixer_register_set(BYTE index, BYTE value);
BOOL	sb_mixer_register_get(BYTE index, BYTE *value);
#endif
