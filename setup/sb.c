/*
   SB.C	by Pancheri Paolo (980898)
http://www.geocities.com/SiliconValley/Park/8933
mailto:DarkAngel@tin.it

Programming the Sound Blaster
For DJGPP 2.0
*/
#include "sb.h"

PRIVATE	WORD	dsp_version = 0;			// Version of the Digital Sound Processor
PRIVATE	WORD	dsp_base_address = 0;	// I/O Base Address of the DSP
PRIVATE	BYTE	dsp_irq_number = 0;		// IRQ use for DMA transfer
PRIVATE	BYTE	dsp_interrupt_number;	// Interrupt number of the IRQ
PRIVATE	BYTE	dsp_dma_channel_8 = 1;	// DMA channel in 8 bit mode

// Auto-detection variables
PRIVATE	BYTE	detect_irq_number = 0;	// Used when the driver detect the IRQ number
PRIVATE	BYTE	detect_save_pic_mask_1;	// Save the mask of PIC 1 when autodetect
PRIVATE	BYTE	detect_save_pic_mask_2; // Save the mask of PIC 1 when autodetect
PRIVATE	_go32_dpmi_seginfo 	detect_irq_handler_2_new, detect_irq_handler_3_new,
		detect_irq_handler_5_new, detect_irq_handler_7_new,
		detect_irq_handler_10_new;
PRIVATE	_go32_dpmi_seginfo 	detect_irq_handler_2_old, detect_irq_handler_3_old,
		detect_irq_handler_5_old, detect_irq_handler_7_old,
		detect_irq_handler_10_old;

// Driver variables
PRIVATE	int  *driver_memory_buffer_right;// Protected mode memory buffer (right channel)
PRIVATE	int  *driver_memory_buffer_left;// Protected mode memory buffer (left channel)
PRIVATE	BOOL	driver_buffer_switch;		// In A/I mode, switch two buffers
PRIVATE	BYTE	driver_save_pic_mask_1;		// Save the mask of PIC 1 when installed
PRIVATE	BYTE	driver_save_pic_mask_2;    // Save the mask of PIC 1 when installed
PRIVATE	WORD	driver_dos_memory_size;		// Size of the memory buffer in DOS space
PRIVATE	BYTE	driver_dos_memory_page;		// Page of the 20 bit DOS buffer address
PRIVATE	BOOL  driver_installed = FALSE;	// TRUE when the driver is installed
PRIVATE	WORD	driver_dos_memory_offset;	// Offset of the 20 bit DOS buffer address
PRIVATE	WORD	driver_dos_memory_segment;	// Segment of the DOS memory buffer
PRIVATE	int	driver_protected_memory_size;	// Memory buffer size in protected mode
PRIVATE	char *driver_dos_memory_linear_address;	// 20 bit DOS buffer address
PRIVATE  _go32_dpmi_seginfo		driver_dos_memory_info;	// Info about DOS buffer
PRIVATE	_go32_dpmi_seginfo		driver_irq_handler_new;	// New IRQ handler (driver)
PRIVATE	_go32_dpmi_seginfo		driver_irq_handler_old;	// Old IRQ handler (?)
PRIVATE	BYTE	driver_dma_page[4]		= {0x87, 0x83, 0x81, 0x82};
PRIVATE	BYTE	driver_dma_length[4]		= {0x01, 0x03, 0x05, 0x07};
PRIVATE	BYTE	driver_dma_address[4]	= {0x00, 0x02, 0x04, 0x06};

// Driver capabilityes
PRIVATE	BOOL	driver_use_stereo;				// TRUE if the driver use stereo mode
PRIVATE	BOOL	driver_use_auto_dma;				// TRUE if the driver use A/I DMA
PRIVATE	BOOL  driver_use_high_speed;			// TRUE if sampling rate > 23 Hz
PRIVATE	SB_CAPABILITY	driver_capability;	// Depends on the SB card capability

// Voice playback
typedef struct
{
	int				size;			// Totale size of the sample
	int				played;		// Number of bytes played
	SAMPLE_BYTE	  *data;			// Pointer to the sample data
	BOOL				stereo;		// TRUE if the sample is stereo
} VOICE;
PRIVATE	int	driver_num_voices = 0;					// Number of voices currently playing
PRIVATE	VOICE	driver_voices[SB_MAX_VOICES];			// List of voices playing
PRIVATE	BYTE	driver_max_voices = SB_MAX_VOICES;	// Max number of voices

// Capabilities of the Sound Blaster cards
// Sound Blaster 1.0/1.5: no A/I DMA, no High Speed, no stereo
PRIVATE	SB_CAPABILITY	capability_sb_10 =
{ min_mono_8: 		4000, 	max_mono_8: 	22222,
min_stereo_8: 	0, 		max_stereo_8: 	0,
				min_mono_16: 	0, 		max_mono_16: 	0,
				min_stereo_16: 	0, 		max_stereo_16: 0,
				auto_dma: 		FALSE, 	stereo: 			FALSE,
				_16_bit: 			FALSE };

// Sound Blaster 2.0: support A/I DMA, High Speed. No stereo
PRIVATE	SB_CAPABILITY	capability_sb_20 =
{ min_mono_8: 		4000, 	max_mono_8: 	45454,
min_stereo_8: 	0, 		max_stereo_8: 	0,
				min_mono_16: 	0, 		max_mono_16: 	0,
				min_stereo_16: 	0, 		max_stereo_16: 0,
				auto_dma: 		TRUE, 	stereo: 			FALSE,
				_16_bit:			FALSE };

// Sound Blaster Pro: support A/I DMA, High Speed, Stereo
PRIVATE	SB_CAPABILITY	capability_sb_pro =
{ min_mono_8: 		4000, 	max_mono_8: 	45454,
min_stereo_8: 	4000, 	max_stereo_8: 	22727,
				min_mono_16: 	0, 		max_mono_16: 	0,
				min_stereo_16: 	0, 		max_stereo_16: 0,
				auto_dma: 		TRUE, 	stereo: 			TRUE,
				_16_bit:			FALSE };

// Sound Blaster 16: support A/I DMA, High Speed, Stereo, 16 bit
PRIVATE	SB_CAPABILITY	capability_sb_16 =
{ min_mono_8: 		4000, 	max_mono_8: 	45454,
min_stereo_8: 	4000, 	max_stereo_8: 	45454,
				min_mono_16: 	4000,		max_mono_16: 	45454,
				min_stereo_16: 	4000,		max_stereo_16: 45454,
				auto_dma: 		TRUE, 	stereo: 			TRUE,
				_16_bit:			TRUE };

// Sound Blaster AWE32: support A/I DMA, High Speed, Stereo, 16 bit
PRIVATE	SB_CAPABILITY	capability_sb_awe32 =
{ min_mono_8: 		5000, 	max_mono_8: 	45454,
min_stereo_8: 	5000, 	max_stereo_8: 	45454,
				min_mono_16: 	5000,		max_mono_16: 	45454,
				min_stereo_16: 	5000,		max_stereo_16: 45454,
				auto_dma: 		TRUE, 	stereo: 			TRUE,
				_16_bit:			TRUE };

// Resets the SB DSP
// 'base_address'(in): the base address of the SB I/O ports
// 'return'(out): result code
BOOL sb_dsp_reset(WORD base_address)
{
	int	i;
	int	start_time;

	// Write a 1 to the SB RESET port
	outportb(base_address + SB_DSP_RESET, 1);

	// Wait for 3.3ms
	for(i = 0; i < 6; i++)
		inportb(base_address + SB_DSP_BUFFER_STATUS);

	// Write a 0 to the SB RESET port
	outportb(base_address + SB_DSP_RESET, 0);

	// Read the byte from the DATA_AVAILABLE port until bit 7 = 1
	start_time = clock();
	while( (inportb(base_address + SB_DSP_DATA_AVAILABLE) < 0x80) &&
			(clock() - start_time < SB_RESET_TIMEOUT) );

	// Poll for a ready byte (AAh) from the READ DATA port
	start_time = clock();
	while( ((i = inportb(base_address + SB_DSP_READ_DATA)) != 0xAA) &&
			(clock() - start_time < SB_RESET_TIMEOUT) );

	if(i != 0xAA) return(SB_RESET_DSP);
	return(SB_OK);
}

// Writes a value to the DSP
// 'value'(in): value to write
// 'result'(out): result code
BOOL sb_dsp_write(BYTE value)
{
	if(!dsp_base_address) return(SB_NOT_INITIALIZED);

	// Read the DSP's BUFFER_STATUS port until bit 7 = 0
	while(inportb(dsp_base_address + SB_DSP_BUFFER_STATUS) > 0x80);

	// Write the value to the WRITE command/data port
	outportb(dsp_base_address + SB_DSP_WRITE, value);
	return(SB_OK);
}

// Read a value from the DSP
// 'value'(out): value readed if successfull
// 'return'(out): result code
BOOL sb_dsp_read(BYTE *value)
{
	if(!dsp_base_address) return(SB_NOT_INITIALIZED);

	// Read the DSP's DATA_AVAILABLE port until bit 7 = 1
	while(inportb(dsp_base_address + SB_DSP_DATA_AVAILABLE) < 0x80);

	// Read the data from the READ_DATA port
	*value = inportb(dsp_base_address + SB_DSP_READ_DATA);
	return(SB_OK);
}

// Turns the speaker on
// 'return'(off): result code
BOOL sb_speaker_on(void)
{  return(sb_dsp_write(SB_SPEAKER_ON)); }

// Turns the speaker off
// 'return'(off): result code
BOOL sb_speaker_off(void)
{  return(sb_dsp_write(SB_SPEAKER_OFF)); }

// Read the DSP version and sets capabilities
// 'version'(out): the version of the DSP
// 'return'(out): result code
// Sound-Blaster AWE32   >=0x040C
// Sound-Blaster 16      >=0x0400
// Sound-Blaster Pro     >=0x0300
// Sound-Blaster 2.0     >=0x0201
// Sound-Blaster 1.0/1.5 else
BOOL sb_dsp_get_version(WORD *version)
{
	BYTE	value;
	BOOL	result;

	if(result = sb_dsp_write(SB_DSP_VERSION)) return(result);
	if(result = sb_dsp_read(&value)) return(result);
	dsp_version = (WORD) value << 8;
	if(result = sb_dsp_read(&value)) return(result);
	dsp_version += value;
	*version = dsp_version;

	if(dsp_version >= 0x040C)
		driver_capability = capability_sb_awe32;
	else if(dsp_version >= 0x0400)
		driver_capability = capability_sb_16;
	else if(dsp_version >= 0x0300)
		driver_capability = capability_sb_pro;
	else if(dsp_version >= 0x0201)
		driver_capability = capability_sb_20;
	else
		driver_capability = capability_sb_10;
	return(SB_OK);
}

// Returns the name of the Sound Blaster model
// 'name'(out): the name of the Sound Blaster model
// 'return'(out): result code
BOOL sb_get_model_name(char *name)
{
	BOOL 	result;
	WORD	version;

	if(result = sb_dsp_get_version(&version)) return(result);

	if(version < 0x0201) strcpy(name, "Sound Blaster 1.0/1.5");
	else if(version < 0x0300) strcpy(name, "Sound Blaster 2.0");
	else if(version < 0x0301) strcpy(name, "Sound Blaster Pro");
	else if(version < 0x0400) strcpy(name, "Sound Blaster Pro 2");
	else if(version < 0x040A) strcpy(name, "Sound Blaster 16");
	else if(version < 0x040C) strcpy(name, "Sound Blaster 16 SCSI-2");
	else strcpy(name, "Sound Blaster AWE32 or better");

	return(SB_OK);
}

// What's the interrupt number for the IRQ?
// 'irq_number'(in): the number of the IRQ
// 'interrupt_number'(out): the interrupt number, if the IRQ is valid
// 'return'(out): result code
BOOL sb_interrupt_number(BYTE irq_number, BYTE *interrupt_number)
{
	dsp_interrupt_number = 0;
	switch (dsp_irq_number)
	{
		case 2: dsp_interrupt_number = SB_IRQ_2; break;
		case 3: dsp_interrupt_number = SB_IRQ_3; break;
		case 5: dsp_interrupt_number = SB_IRQ_5; break;
		case 7: dsp_interrupt_number = SB_IRQ_7; break;
		case 10: dsp_interrupt_number = SB_IRQ_10; break;
	}
	if(dsp_interrupt_number) return(SB_OK);
	return(SB_INVALID_IRQ);
}

// Resets the SB DSP
// 'base_address'(out): the base address of the SB I/O ports if successfull
// 'return'(out): result code
BOOL sb_dsp_detect_base_address(WORD *base_address)
{
	WORD	address;

	for(address = 0x210; address < 0x290; address += 0x010)
		if(!sb_dsp_reset(address))
		{
			*base_address = address;
			dsp_base_address = address;
			return(SB_OK);
		}

	return(SB_DETECT_DSP);
}

PRIVATE void detect_irq_handler_2(void)
{
	inportb(dsp_base_address + SB_IRQ_ACK_8);
	outportb(SB_PIC1_EOI, 0x20);
	detect_irq_number = 2;
}
END_OF_FUNCTION(detect_irq_handler_2);

PRIVATE void detect_irq_handler_3(void)
{
	inportb(dsp_base_address + SB_IRQ_ACK_8);
	outportb(SB_PIC1_EOI, 0x20);
	detect_irq_number = 3;
}
END_OF_FUNCTION(detect_irq_handler_3);

PRIVATE void detect_irq_handler_5(void)
{
	inportb(dsp_base_address + SB_IRQ_ACK_8);
	outportb(SB_PIC1_EOI, 0x20);
	detect_irq_number = 5;
}
END_OF_FUNCTION(detect_irq_handler_5);

PRIVATE void detect_irq_handler_7(void)
{
	inportb(dsp_base_address + SB_IRQ_ACK_8);
	outportb(SB_PIC1_EOI, 0x20);
	detect_irq_number = 7;
}
END_OF_FUNCTION(detect_irq_handler_7);

PRIVATE void detect_irq_handler_10(void)
{
	inportb(dsp_base_address + SB_IRQ_ACK_8);
	outportb(SB_PIC2_EOI, 0x20);
	outportb(SB_PIC1_EOI, 0x20);
	detect_irq_number = 10;
}
END_OF_FUNCTION(detect_irq_handler_10);

PRIVATE	void	driver_setup_dma_transfer(void)
{
	outportb(SB_DMA_MASK_8, 0x04 + dsp_dma_channel_8);
	outportb(SB_DMA_CLEAR_8, 0);
	if(driver_use_auto_dma)
		outportb(SB_DMA_MODE_8, 0x58 + dsp_dma_channel_8);
	else
		outportb(SB_DMA_MODE_8, 0x48 + dsp_dma_channel_8);
	outportb(driver_dma_page[dsp_dma_channel_8], driver_dos_memory_page);
	outportb(driver_dma_address[dsp_dma_channel_8], driver_dos_memory_offset);
	outportb(driver_dma_address[dsp_dma_channel_8], driver_dos_memory_offset >> 8);
	outportb(driver_dma_length[dsp_dma_channel_8], driver_dos_memory_size - 1);
	outportb(driver_dma_length[dsp_dma_channel_8], (driver_dos_memory_size - 1) >> 8);
	outportb(SB_DMA_MASK_8, 0x00 + dsp_dma_channel_8);
}
END_OF_FUNCTION(driver_setup_dma_transfer);

PRIVATE	void	driver_setup_dsp_transfer(void)
{
	if(driver_use_auto_dma)
	{
		if((driver_use_stereo) && (dsp_version >= 0x0400))
		{
			sb_dsp_write(0xC6);
			sb_dsp_write(0x20);
			sb_dsp_write((driver_protected_memory_size * 2 - 1) & 0xFF);
			sb_dsp_write((driver_protected_memory_size * 2 - 1) >> 8);
		} else {
			sb_dsp_write(SB_DMA_BLOCK_SIZE);
			sb_dsp_write((driver_protected_memory_size - 1) & 0xFF);
			sb_dsp_write((driver_protected_memory_size - 1) >> 8);
			if(driver_use_high_speed)
				sb_dsp_write(SB_DMA_MODE_8_AI_HS);
			else
				sb_dsp_write(SB_DMA_MODE_8_AI_NS);
		}
	} else {
		sb_dsp_write(SB_DMA_MODE_8_SC);
		sb_dsp_write((driver_dos_memory_size - 1) & 0xFF);
		sb_dsp_write((driver_dos_memory_size - 1) >> 8);
	}
}
END_OF_FUNCTION(driver_setup_dsp_transfer);

PRIVATE	void	driver_mix_mono_voices(void)
{
	int voice, i, y, add_voices;

	memset(driver_memory_buffer_left, 0, driver_protected_memory_size * sizeof(int));
	for(i = 0; i < driver_protected_memory_size; i++)
	{
		add_voices = driver_num_voices;
		for(voice = 0; voice < driver_num_voices; voice++)
		{
			if(driver_voices[voice].stereo)
			{
				driver_memory_buffer_left[i] += (
						(int)driver_voices[voice].data[driver_voices[voice].played] +
						(int)driver_voices[voice].data[driver_voices[voice].played + 1]) >> 1;
				driver_voices[voice].played++;
			} else
				driver_memory_buffer_left[i] += (int)driver_voices[voice].data[driver_voices[voice].played];
			driver_voices[voice].played++;

			if(driver_voices[voice].played >= driver_voices[voice].size)
			{
				for(y = driver_num_voices - 1; y > voice; y--)
					driver_voices[y - 1] = driver_voices[y];
				driver_num_voices--;
				voice--;
			}
		}
		if(add_voices) driver_memory_buffer_left[i] /= add_voices;
	}
}
END_OF_FUNCTION(driver_mix_mono_voices);

PRIVATE	void	driver_mix_stereo_voices(void)
{
	int voice, i, y, add_voices;

	memset(driver_memory_buffer_left, 0, driver_protected_memory_size * sizeof(int));
	memset(driver_memory_buffer_right, 0, driver_protected_memory_size * sizeof(int));
	for(i = 0; i < driver_protected_memory_size; i++)
	{
		add_voices = driver_num_voices;
		for(voice = 0; voice < driver_num_voices; voice++)
		{
			driver_memory_buffer_left[i] += (int)driver_voices[voice].data[driver_voices[voice].played];
			driver_voices[voice].played++;

			if(driver_voices[voice].stereo)
			{
				driver_memory_buffer_right[i] += (int)driver_voices[voice].data[driver_voices[voice].played];
				driver_voices[voice].played++;
			} else
				driver_memory_buffer_right[i] += (int)driver_voices[voice].data[driver_voices[voice].played - 1];

			if(driver_voices[voice].played >= driver_voices[voice].size)
			{
				for(y = driver_num_voices - 1; y > voice; y--)
					driver_voices[y - 1] = driver_voices[y];
				driver_num_voices--;
				voice--;
			}
		}
		if(add_voices) {
			driver_memory_buffer_right[i] /= add_voices;
			driver_memory_buffer_left[i] /= add_voices;
		}
	}
}
END_OF_FUNCTION(driver_mix_stereo_voices);

PRIVATE	void	driver_mix_voices(void)
{
	if(driver_use_stereo) driver_mix_stereo_voices();
	else driver_mix_mono_voices();
}
END_OF_FUNCTION(driver_mix_voices);

PRIVATE	void  driver_irq_handler(void)
{
	int 	i;
	SAMPLE_BYTE *buffer;

	driver_mix_voices();
	if(driver_use_auto_dma && driver_buffer_switch)
		if(driver_use_stereo)
			buffer = driver_dos_memory_linear_address + driver_protected_memory_size * 2;
		else
			buffer = driver_dos_memory_linear_address + driver_protected_memory_size;
	else buffer = driver_dos_memory_linear_address;
	driver_buffer_switch = !driver_buffer_switch;

	if(driver_use_stereo)
	{
		for(i = 0; i < driver_protected_memory_size; i++)
		{
			if(driver_memory_buffer_left[i] > SAMPLE_CLIP_MAX)
				buffer[i * 2] = SAMPLE_CLIP_MAX;
			else if(driver_memory_buffer_left[i] < SAMPLE_CLIP_MIN)
				buffer[i * 2] = SAMPLE_CLIP_MIN;
			else
				buffer[i * 2] = (SAMPLE_BYTE)driver_memory_buffer_left[i];

			if(driver_memory_buffer_right[i] > SAMPLE_CLIP_MAX)
				buffer[i * 2 + 1] = SAMPLE_CLIP_MAX;
			else if(driver_memory_buffer_right[i] < SAMPLE_CLIP_MIN)
				buffer[i * 2 + 1] = SAMPLE_CLIP_MIN;
			else
				buffer[i * 2 + 1] = (SAMPLE_BYTE)driver_memory_buffer_right[i];
		}
	} else {
		for(i = 0; i < driver_protected_memory_size; i++)
			if(driver_memory_buffer_left[i] > SAMPLE_CLIP_MAX)
				buffer[i] = SAMPLE_CLIP_MAX;
			else if(driver_memory_buffer_left[i] < SAMPLE_CLIP_MIN)
				buffer[i] = SAMPLE_CLIP_MIN;
			else
				buffer[i] = (SAMPLE_BYTE)driver_memory_buffer_left[i];
	}

	if(!driver_use_auto_dma)
	{
		driver_setup_dma_transfer();
		driver_setup_dsp_transfer();
	}

	inportb(dsp_base_address + SB_IRQ_ACK_8);
	if(dsp_irq_number == 10) outportb(SB_PIC2_EOI, 0x20);
	outportb(SB_PIC1_EOI, 0x20);
}
END_OF_FUNCTION(driver_irq_handler);

// Install the new IRQ handlers and enable PIC interrupt mask
PRIVATE void detect_install_handlers(void)
{
	INSTALL_HANDLER(SB_IRQ_2, detect_irq_handler_2);
	INSTALL_HANDLER(SB_IRQ_3, detect_irq_handler_3);
	INSTALL_HANDLER(SB_IRQ_5, detect_irq_handler_5);
	INSTALL_HANDLER(SB_IRQ_7, detect_irq_handler_7);
	INSTALL_HANDLER(SB_IRQ_10, detect_irq_handler_10);

	// Enables PIC interrupt mask
	detect_save_pic_mask_1 = inportb(SB_PIC_MASK_1);
	detect_save_pic_mask_2 = inportb(SB_PIC_MASK_2);
	outportb(SB_PIC_MASK_1, detect_save_pic_mask_1 & 0x53);  // Enable IRQ 2,3,5,7
	outportb(SB_PIC_MASK_2, detect_save_pic_mask_2 & 0xFB);  // Enable IRQ 10
}

// Restore old PIC mask and IRQ handlers
PRIVATE void detect_remove_handlers(void)
{
	// Restore old PIC masks
	outportb(SB_PIC_MASK_2, detect_save_pic_mask_2);
	outportb(SB_PIC_MASK_1, detect_save_pic_mask_1);

	REMOVE_HANDLER(SB_IRQ_10, detect_irq_handler_10);
	REMOVE_HANDLER(SB_IRQ_7, detect_irq_handler_7);
	REMOVE_HANDLER(SB_IRQ_5, detect_irq_handler_5);
	REMOVE_HANDLER(SB_IRQ_3, detect_irq_handler_3);
	REMOVE_HANDLER(SB_IRQ_2, detect_irq_handler_2);
}

PRIVATE	void	lock_memory(void)
{
	LOCK_FUNCTION(detect_irq_handler_2);
	LOCK_FUNCTION(detect_irq_handler_3);
	LOCK_FUNCTION(detect_irq_handler_5);
	LOCK_FUNCTION(detect_irq_handler_7);
	LOCK_FUNCTION(detect_irq_handler_10);

	LOCK_FUNCTION(driver_irq_handler);
	LOCK_FUNCTION(driver_setup_dma_transfer);
	LOCK_FUNCTION(driver_setup_dsp_transfer);

	LOCK_FUNCTION(driver_mix_voices);
	LOCK_FUNCTION(driver_mix_mono_voices);
	LOCK_FUNCTION(driver_mix_stereo_voices);

	LOCK_VARIABLE(dsp_version);
	LOCK_VARIABLE(dsp_base_address);
	LOCK_VARIABLE(dsp_dma_channel_8);
	LOCK_VARIABLE(detect_irq_number);

	LOCK_VARIABLE(driver_dma_page);
	LOCK_VARIABLE(driver_dma_length);
	LOCK_VARIABLE(driver_dma_address);
	LOCK_VARIABLE(driver_dos_memory_page);
	LOCK_VARIABLE(driver_dos_memory_size);
	LOCK_VARIABLE(driver_dos_memory_offset);

	LOCK_VARIABLE(driver_voices);
	LOCK_VARIABLE(driver_max_voices);
	LOCK_VARIABLE(driver_num_voices);
	LOCK_VARIABLE(driver_buffer_switch);
	LOCK_VARIABLE(driver_protected_memory_size);

	LOCK_VARIABLE(driver_use_stereo);
	LOCK_VARIABLE(driver_use_auto_dma);
	LOCK_VARIABLE(driver_use_high_speed);
}

// Detects the SB IRQ number
// 'irq_number'(out): the IRQ number if successfull
// 'return'(out): result code
BOOL sb_dsp_detect_irq_number(BYTE *irq_number)
{
	BOOL result;
	clock_t start_time;

	lock_memory();
	detect_install_handlers();
	dsp_irq_number = detect_irq_number = 0;

	// Trigger the interrupt
	start_time = clock();
	sb_dsp_write(SB_IRQ_TRIGGER);
	while((detect_irq_number == 0) &&
			(clock() - start_time < SB_IRQ_DETECT_TIMEOUT));
	dsp_irq_number = detect_irq_number;
	detect_remove_handlers();

	// IRQ number -> INT number
	if(result = sb_interrupt_number(dsp_irq_number, &dsp_interrupt_number)) return(result);
	*irq_number = dsp_irq_number;
	return(SB_OK);
}

BOOL	sb_dsp_detect_dma_channel_8(BYTE	*dma_channel_8)
{
	*dma_channel_8 = dsp_dma_channel_8;
	return(SB_OK);
}

// Change the default value of the DMA channel
void	sb_set_dma_channel_8(BYTE dma_channel_8)
{ dsp_dma_channel_8 = dma_channel_8; }

PRIVATE	BOOL	detect_auto_configure(void)
{
	BOOL	result;
	WORD 	base_address;
	BYTE	irq_number;
	BYTE	dma_channel_8;
	WORD	version;

	if(result = sb_dsp_detect_base_address(&base_address)) return(result);
	if(result = sb_dsp_detect_irq_number(&irq_number)) return(result);
	if(result = sb_dsp_detect_dma_channel_8(&dma_channel_8)) return(result);
	if(result = sb_dsp_get_version(&version)) return(result);

	return(SB_OK);
}

// Allocate a dos memory buffer that doesn't cross 64K page boundries
// 'return'(out): result code
BOOL driver_allocate_dos_memory(int size)
{
	int counter;
	int end_page;
	int start_page;
	int linear_address;
	_go32_dpmi_seginfo buffers[SB_DOS_ALLOCATE_RETRY];

	for(counter = 0; counter < SB_DOS_ALLOCATE_RETRY; counter++)
	{
		// Try to allocate DOS memory
		buffers[counter].size = (size + 15) >> 4;
		if(_go32_dpmi_allocate_dos_memory(&(buffers[counter])))
			return(SB_DOS_MEMORY);

		// Check if the start and the end page of the memory buffer
		// are the same (so the buffer doesn't cross the 64K page boundries)
		linear_address = buffers[counter].rm_segment << 4;
		start_page = linear_address >> 16;
		end_page = (linear_address + size - 1) >> 16;
		if(start_page == end_page) break;
	}
	if(counter >= SB_DOS_ALLOCATE_RETRY) return(SB_DOS_MEMORY);

	driver_dos_memory_info = buffers[counter];
	driver_dos_memory_segment = buffers[counter].rm_segment;
	driver_dos_memory_linear_address = (char *)((driver_dos_memory_segment << 4) + __djgpp_conventional_base);
	driver_dos_memory_offset = (driver_dos_memory_segment << 4) & 0xFFFF;
	driver_dos_memory_page = (driver_dos_memory_segment << 4) >> 16;
	memset(driver_dos_memory_linear_address, 0, driver_dos_memory_size);
	_go32_dpmi_lock_data(driver_dos_memory_linear_address, driver_dos_memory_size);
	_go32_dpmi_lock_data(&driver_dos_memory_linear_address, sizeof(driver_dos_memory_linear_address));

	// Release the unused memory
	for(--counter; counter >=0; counter--)
		_go32_dpmi_free_dos_memory(&(buffers[counter]));

	return(SB_OK);
}

// Sets the Time Costant
// 'frequency'(in): the sampling rate
// 'return'(out): result code
PRIVATE	BOOL	driver_set_time_costant(WORD	frequency)
{
	// If the card is SB Pro or better, reset the mixer
	if(dsp_version >= 0x0300)
		sb_mixer_register_set(SB_MIXER_RESET, 0);

	if(driver_use_stereo)
	{
		if((frequency < driver_capability.min_stereo_8) ||
				(frequency > driver_capability.max_stereo_8)) return(SB_SAMPLE_RATE);

		// Set stereo mode
		sb_mixer_register_set(SB_MIXER_OUTPUT, 0x13);
	} else {
		if((frequency < driver_capability.min_mono_8) ||
				(frequency > driver_capability.max_mono_8)) return(SB_SAMPLE_RATE);

		// Set mono mode
		sb_mixer_register_set(SB_MIXER_OUTPUT, 0x13);
	}

	if(frequency > 23 * 1024)
	{
		WORD	time_costant;

		// Set High Speed Time Costant
		time_costant = 65536 - 256000000 / frequency;
		sb_dsp_write(SB_TIME_COSTANT);
		sb_dsp_write(time_costant >> 8);
		driver_use_high_speed = TRUE;
	} else {
		BYTE	time_costant;

		// Set Normal Speed Time Costant
		time_costant = 256 - 1000000 / frequency;
		sb_dsp_write(SB_TIME_COSTANT);
		sb_dsp_write(time_costant);
		driver_use_high_speed = FALSE;
	}

	return(SB_OK);
}

// Install the Sound Blaster Driver
// 'frequency'(in): sound driver frequency (Hz)
// 'stereo'(in): use stereo mode (if supported by the sound card)
// 'return'(out): result code
BOOL	sb_install_driver(WORD	frequency, BOOL use_stereo)
{
	BOOL	result;

	// Auto-detect base address, IRQ, (8 bit DMA channel), DSP version
	if(result = detect_auto_configure()) return(result);
	driver_use_auto_dma = driver_capability.auto_dma;
	driver_buffer_switch = 0;
	driver_num_voices = 0;

	// Check if should and can use stereo
	if(use_stereo)
		if(!driver_capability.stereo) return(SB_USE_STEREO);
	driver_use_stereo = use_stereo;

	// Allocate DOS memory buffer
	__djgpp_nearptr_enable();
	driver_dos_memory_size = SB_DOS_MEMORY_SIZE;
	if(driver_use_stereo) driver_dos_memory_size *= 2;
	if(frequency > 23 * 1024) driver_dos_memory_size *= 2;
	if(result = driver_allocate_dos_memory(driver_dos_memory_size)) return(result);

	// Allocate protected memory buffer (for the left channel in stereo mode)
	driver_protected_memory_size = driver_dos_memory_size;
	if(driver_use_auto_dma) driver_protected_memory_size /= 2;
	if(driver_use_stereo) driver_protected_memory_size /= 2;
	if((driver_memory_buffer_left = (int *) malloc(driver_protected_memory_size * sizeof(int))) == NULL) return(SB_MEMORY);
	_go32_dpmi_lock_data(driver_memory_buffer_left, driver_protected_memory_size * sizeof(int));
	_go32_dpmi_lock_data(&driver_memory_buffer_left, sizeof(int));

	// Allocate protected memory buffer for the right channel in stereo mode
	if(driver_use_stereo)
	{
		if((driver_memory_buffer_right = (int *) malloc(driver_protected_memory_size * sizeof(int))) == NULL) return(SB_MEMORY);
		_go32_dpmi_lock_data(driver_memory_buffer_right, driver_protected_memory_size * sizeof(int));
		_go32_dpmi_lock_data(&driver_memory_buffer_right, sizeof(int));
	}

	// Installs the IRQ handler
	INSTALL_HANDLER(dsp_interrupt_number, driver_irq_handler);

	// Enable PIC mask
	driver_save_pic_mask_1 = inportb(SB_PIC_MASK_1);
	driver_save_pic_mask_2 = inportb(SB_PIC_MASK_2);
	if(dsp_irq_number != 10) outportb(SB_PIC_MASK_1, driver_save_pic_mask_1 & (!(1 << dsp_irq_number)));
	else outportb(SB_PIC_MASK_2, driver_save_pic_mask_2 & 0xFB);

	// Sets the Time Costant
	sb_dsp_reset(dsp_base_address);
	if(result = driver_set_time_costant(frequency)) return(result);

	// Setup DMA transfer
	driver_setup_dma_transfer();

	// Setup DSP transfer
	driver_setup_dsp_transfer();

	driver_installed = TRUE;
	return(SB_OK);
}

// Remove the Sound Blaster driver
// 'return'(out): result code
BOOL	sb_remove_driver(void)
{
	if(!driver_installed) return(SB_NOT_INITIALIZED);

	__djgpp_nearptr_disable();
	// Exit Auto-Initialize DMA Operation
	if(driver_use_auto_dma)
	{
		sb_dsp_write(SB_HALT_DMA);
		sb_dsp_write(SB_EXIT_DMA);
		sb_dsp_write(SB_HALT_DMA);
	}

	// Restore old PIC masks
	outportb(SB_PIC_MASK_1, driver_save_pic_mask_1);
	outportb(SB_PIC_MASK_2, driver_save_pic_mask_2);

	// Restore old IRQ handler
	REMOVE_HANDLER(dsp_interrupt_number, driver_irq_handler);

	// Release DOS memory
	_go32_dpmi_free_dos_memory(&driver_dos_memory_info);
	driver_installed = FALSE;
	return(SB_OK);
}

WORD	sb_get_driver_version(void)
{ return((SB_DRIVER_VERSION_HI << 8) | SB_DRIVER_VERSION_LO); }

// Change the default value of the max number of voices
void	sb_set_max_voices(BYTE max_voices)
{ driver_max_voices = max_voices; }

// Reduce the amplitude of the sample
// 'sample'(in): the sample to reduce
void	sb_reduce_amplitude(SAMPLE *sample)
{
	int i;

	for(i = 0; i < sample->size; i++)
		sample->data[i] >>= 1;
}

// Release the memory used by a sample
void	sb_free_sample(SAMPLE *sample)
{
	free(sample->data);
	sample->data = NULL;
}

// Play a sample
// 'sample'(in): sample to play
BOOL	sb_play_sample(SAMPLE *sample)
{
	if(!driver_installed) return(SB_NOT_INITIALIZED);
	if(driver_num_voices >= driver_max_voices - 1) return(SB_TOO_MANY_VOICES);

	driver_voices[driver_num_voices].played = 0;
	driver_voices[driver_num_voices].size = sample->size;
	driver_voices[driver_num_voices].data = sample->data;
	driver_voices[driver_num_voices].stereo = sample->stereo;
	driver_num_voices++;
	return(SB_OK);
}

// Allocate memory and read the data from a raw audio file
// 'filename'(in): the name of the file that contains the RAW data
// 'sample'(out): sample structure
// 'return'(out): result code
BOOL sb_read_raw(char *filename, SAMPLE *sample)
{
	FILE *stream;

	// Try to open the raw file
	if((stream = fopen(filename, "rb")) == NULL) return(SB_OPEN);

	// Read the length of the data
	sample->size = filelength(fileno(stream));

	// Allocates and lock the memory
	if((sample->data = (char *) malloc(sample->size)) == NULL)
	{
		fclose(stream);
		return(SB_MEMORY);
	}
	_go32_dpmi_lock_data((void *)sample->data, sample->size);

	// Read the data
	if(fread(sample->data, sample->size, 1, stream) != 1)
	{
		fclose(stream);
		return(SB_READ);
	}
	fclose(stream);

	// The default RAW mode is mono
	sample->stereo = FALSE;

	return(SB_OK);
}

// Read the header of a WAV (RIFF file) and the data
// 'filename'(in): name of the .WAF file
// 'sample'(out): sample structure
// 'return'(out): result code
BOOL sb_read_wav(char *filename, SAMPLE *sample)
{
	FILE 		  *stream;
	WAVCHUNK 	wavchunk;
	DATACHUNK 	datachunk;

	if((stream=fopen(filename, "rb"))==NULL) return(SB_OPEN);
	if(fread(&wavchunk, sizeof(WAVCHUNK), 1, stream)!=1) {
		fclose(stream);
		return(SB_READ);
	}
	// Check signatures
	if(wavchunk.riffsign[0]!='R'||wavchunk.riffsign[1]!='I'||
			wavchunk.riffsign[2]!='F'||wavchunk.riffsign[3]!='F') {
		fclose(stream);
		return(SB_INVALID_FILE);
	}
	if(wavchunk.wavesign[0]!='W'||wavchunk.wavesign[1]!='A'||
			wavchunk.wavesign[2]!='V'||wavchunk.wavesign[3]!='E') {
		fclose(stream);
		return(SB_INVALID_FILE);
	}

	// The WAVCHUNK have dynamic size!
	fseek(stream, wavchunk.formatlength - WAV_CHUNK_LENGTH, SEEK_CUR);
	if(fread(&datachunk, sizeof(DATACHUNK), 1, stream)!=1) {
		fclose(stream);
		return(SB_READ);
	}

	if(datachunk.sign[0]!='d'||datachunk.sign[1]!='a'||
			datachunk.sign[2]!='t'||datachunk.sign[3]!='a') {
		fclose(stream);
		return(SB_INVALID_FILE);
	}

	sample->size = datachunk.length;
	if((sample->data = (char *) malloc(sample->size))==NULL) {
		fclose(stream);
		return(SB_MEMORY);
	}
	_go32_dpmi_lock_data((void *)sample->data, sample->size);

	if(fread(sample->data, sample->size, 1, stream)!=1) {
		fclose(stream);
		return(SB_READ);
	}
	fclose(stream);

	// Set the mono or stereo mode
	sample->stereo = wavchunk.channels == 2;

	return(SB_OK);
}

// Read RAW or WAV files, depending on the filename extension
BOOL sb_read_sample(char *filename, SAMPLE *sample)
{
	char *extension;

	if((extension = strchr(filename, '.')) == NULL) return(SB_INVALID_FILE);
	if(!strcasecmp(extension, SB_EXTENSION_WAV)) return(sb_read_wav(filename, sample));
	else if(!strcasecmp(extension, SB_EXTENSION_RAW)) return(sb_read_raw(filename, sample));
	else return(SB_INVALID_FILE);
}

// Write a value to the mixer register
// 'index'(in): the index of the register
// 'value'(in): value to write
// 'return'(out): result code
BOOL	sb_mixer_register_set(BYTE index, BYTE value)
{
	if(!dsp_base_address) return(SB_NOT_INITIALIZED);

	// Set the register index
	outportb(dsp_base_address + SB_MIXER_INDEX, index);

	// Write the value to the mixer register
	outportb(dsp_base_address + SB_MIXER_REGISTER, value);
	return(SB_OK);
}

// Read a value from the mixer register
// 'index'(in): the index of the register
// 'value'(out): the value of the register
// 'return'(out): result code
BOOL	sb_mixer_register_get(BYTE index, BYTE *value)
{
	if(!dsp_base_address) return(SB_NOT_INITIALIZED);

	// Set the register index
	outportb(dsp_base_address + SB_MIXER_INDEX, index);

	// Read the value of the mixer register
	*value = inportb(dsp_base_address + SB_MIXER_REGISTER);
	return(SB_OK);
}
