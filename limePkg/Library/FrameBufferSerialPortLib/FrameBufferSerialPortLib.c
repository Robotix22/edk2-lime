/*################################################################################################
#
# added delay after each line printed on screen, 16 feb 2012, SERDELIUK
#
# this library has an unknown author, please let me know if you know the initial author
#
################################################################################################*/


#include <Library/TimerLib.h>
#include <Library/BaseLib.h>
#include <PiDxe.h>
#include <Library/ArmLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/HobLib.h>
#include <Library/SerialPortLib.h>
#include <Library/PcdLib.h>
#include <Resources/font5x12.h>
#include <Resources/FbColor.h>
#include "FrameBufferSerialPortLib.h"

// 1000000 = 1s
//  500000 = 0.5s
//  100000 = 0.1s
UINTN delay = 100000; // 0.1s

FBCON_POSITION m_Position;
FBCON_POSITION hm_Position;
FBCON_POSITION fm_Position;
FBCON_POSITION m_MaxPosition;
FBCON_COLOR m_Color;

BOOLEAN x_Initialized = FALSE;
BOOLEAN m_Initialized = FALSE;

UINTN gWidth = FixedPcdGet32(PcdMipiFrameBufferWidth);
// Reserve half screen for output
UINTN gHeight = FixedPcdGet32(PcdMipiFrameBufferHeight);
UINTN gBpp = FixedPcdGet32(PcdMipiFrameBufferPixelBpp);


// Module-used internal routine
void FbConPutCharWithFactor
(
	char c,
	int type,
	unsigned scale_factor
);

void FbConDrawglyph
(
	char *pixels,
	unsigned stride,
	unsigned bpp,
	unsigned *glyph,
	unsigned scale_factor
);

void FbConReset(void);
void FbConScrollUp(void);
void FbConFlush(void);
void ResetFbHeader(void);
void ResetFbFooter(void);
void UpdateHeader(const CHAR16 *teletext, UINTN zone);
void UpdateFooter(const CHAR16 *teletext);

RETURN_STATUS
EFIAPI
SerialPortInitialize
(
	VOID
)
{

	UINTN InterruptState = 0;

	// Prevent dup initialization
	if (m_Initialized) return RETURN_SUCCESS;

	// Interrupt Disable
	InterruptState = ArmGetInterruptState();
	ArmDisableInterrupts();


	// Reset console
	ResetFb();
	FbConReset();
	FbConFlush();

//	FbConFlush();
//	FbConReset();
//	ResetFb();

	ResetFbHeader();
	ResetFbFooter();
	UpdateHeader(L"EDK2 RedmiNote7Pkg by SERDELIUK",1);
//	    MicroSecondDelay( delay/3 ); 
//	UpdateHeader(L"    by SERDELIUK ",1);
//	    MicroSecondDelay( delay/3 ); 
//	UpdateFooter(L"    TESTING, TESTING, 1, 2, 3 ");
//	    MicroSecondDelay( delay/3 ); 
	// Set flag
	m_Initialized = TRUE;

	if (InterruptState) ArmEnableInterrupts();
	return RETURN_SUCCESS;
}

void ResetFb(void)
{
	// Clear current screen.
	char* Pixels = (void*)FixedPcdGet32(PcdMipiFrameBufferAddress);
	UINTN BgColor = FB_BGRA8888_BLACK;

	// Set to black color.
	for (UINTN i = 0; i < gWidth; i++)
	{
		for (UINTN j = 0; j < gHeight; j++)
		{
			BgColor = FB_BGRA8888_BLACK;
			// Set pixel bit
			for (UINTN p = 0; p < (gBpp / 8); p++)
			{
				*Pixels = (unsigned char)BgColor;
				BgColor = BgColor >> 8;
				Pixels++;
			}
		}
	}
}

void UpdateHeader(const CHAR16 *teletext, UINTN zone)
{
	BOOLEAN intstate = ArmGetInterruptState();
	ArmDisableInterrupts();

	m_Color.Background = FB_BGRA8888_BLUE;
	char* hPixels;
	hPixels = (void*)FixedPcdGet32(PcdMipiFrameBufferHeaderAddress);

	if ( zone == 1 )
	{ 
	    hm_Position.y = 3;
	    hm_Position.x = 4;
	    hPixels += hm_Position.y * ((gBpp / 8) * FONT_HEIGHT * gWidth);
	    hPixels += hm_Position.x * 2 * ((gBpp / 8) * (FONT_WIDTH + 2));
	} else {
	    hm_Position.y = 3;
	    hm_Position.x = 76;
	    hPixels += hm_Position.y * ((gBpp / 8) * FONT_HEIGHT * gWidth);
	    hPixels += hm_Position.x * 2 * ((gBpp / 8) * (FONT_WIDTH + 2));
	}

	for (UINTN i = 0; i < StrLen(teletext); i++)
	{
	    FbConDrawglyph(hPixels, gWidth, (gBpp / 8), font5x12 + (teletext[i] - 32) * 2, 2);
	    hPixels += hm_Position.x * ((gBpp / 8) + (FONT_WIDTH + 2));
	    //  delay....
//	    MicroSecondDelay( delay/4 );
	}
	if (intstate) ArmEnableInterrupts();
	    hPixels = (void*)FixedPcdGet32(PcdMipiFrameBufferHeaderAddress);
	    m_Color.Background = FB_BGRA8888_BLACK;
}

void UpdateFooter(const CHAR16 *teletext)
{
	BOOLEAN intstate = ArmGetInterruptState();
	ArmDisableInterrupts();

	m_Color.Background = FB_BGRA8888_BLUE;
	char* fPixels;
	fPixels = (void*)FixedPcdGet32(PcdMipiFrameBufferFooterAddress);

	fm_Position.y = 3;
	fm_Position.x = 4;
	fPixels += fm_Position.y * ((gBpp / 8) * FONT_HEIGHT * gWidth);
	fPixels += fm_Position.x * 2 * ((gBpp / 8) * FONT_WIDTH);

	for (UINTN i = 0; i < StrLen(teletext); i++)
	{
	    FbConDrawglyph(fPixels, gWidth, (gBpp / 8), font5x12 + (teletext[i] - 32) * 2, 2);
	    fPixels += fm_Position.x * ((gBpp / 8) + FONT_WIDTH);
	    //  delay....
	    //MicroSecondDelay( delay/2 );
	}
	if (intstate) ArmEnableInterrupts();
	    fPixels = (void*)FixedPcdGet32(PcdMipiFrameBufferFooterAddress);
	    m_Color.Background = FB_BGRA8888_BLACK;
}

void ResetFbHeader(void)
{

	// Clear current screen.
	char* hPixels = (void*)FixedPcdGet32(PcdMipiFrameBufferHeaderAddress);
	UINTN hBgColor = FB_BGRA8888_BLUE;

	// Set to black color.
	for (UINTN i = 0; i < gWidth; i++)
	{
		for (UINTN j = 0; j < 84; j++) // font is 12px, means 7 lines of text
		{
			hBgColor = FB_BGRA8888_BLUE;
			// Set pixel bit
			for (UINTN p = 0; p < (gBpp / 8); p++)
			{
				*hPixels = (unsigned char)hBgColor;
				hBgColor = hBgColor >> 8;
				hPixels++;
			}
		}
	}
}

void ResetFbFooter(void)
{

	// Clear current screen.
	char* fPixels = (void*)FixedPcdGet32(PcdMipiFrameBufferFooterAddress);
	UINTN fBgColor = FB_BGRA8888_BLUE;

	// Set to black color.
	for (UINTN i = 0; i < gWidth; i++)
	{
		for (UINTN j = 0; j < 84; j++) // font is 12px, means 7 lines of text
		{
			fBgColor = FB_BGRA8888_BLUE;
			// Set pixel bit
			for (UINTN p = 0; p < (gBpp / 8); p++)
			{
				*fPixels = (unsigned char)fBgColor;
				fBgColor = fBgColor >> 8;
				fPixels++;
			}
		}
	}
}

void FbConReset(void)
{
	// Reset position.
	m_Position.x = 0;
	m_Position.y = 0;

	// Calc max position.
	m_MaxPosition.x = gWidth / (FONT_WIDTH + 1);
	m_MaxPosition.y = (gHeight - 1) / FONT_HEIGHT;

	// Reset color.
	m_Color.Foreground = FB_BGRA8888_WHITE;
	m_Color.Background = FB_BGRA8888_BLACK;
}

void FbConPutCharWithFactor
(
	char c,
	int type,
	unsigned scale_factor
)
{
	char* Pixels;

	if (!m_Initialized) return;

paint:

	if ((unsigned char)c > 127) return;

	if ((unsigned char)c < 32)
	{
		if (c == '\n')
		{
			goto newline;
		}
		else if (c == '\r')
		{
//			m_Position.x = 0;
			return;
		}
		else if (c == '\a')
		{
			if (m_Color.Foreground == FB_BGRA8888_WHITE)
			{
			    m_Color.Foreground = FB_BGRA8888_RED;
			} else {
			    m_Color.Foreground = FB_BGRA8888_WHITE;
			}
		    return;
		}
		else if (c == '\v')
		{
			if (m_Color.Foreground == FB_BGRA8888_WHITE)
			{
			    m_Color.Foreground = FB_BGRA8888_GREEN;
			} else {
			    m_Color.Foreground = FB_BGRA8888_WHITE;
			}
		    return;
		}
		else
		{
			return;
		}
	}

	// Save some space
//	if (m_Position.x == 0 && (unsigned char)c == ' ' &&
//		type != FBCON_SUBTITLE_MSG &&
//		type != FBCON_TITLE_MSG)
//		return;

	BOOLEAN intstate = ArmGetInterruptState();
	ArmDisableInterrupts();

	Pixels = (void*)FixedPcdGet32(PcdMipiFrameBufferAddress);
	Pixels += m_Position.y * ((gBpp / 8) * FONT_HEIGHT * gWidth);
	Pixels += m_Position.x * scale_factor * ((gBpp / 8) * (FONT_WIDTH + 1));

/*        for (UINTN g = 0; g < 3; g++)
        {
        // blink cursor
            m_Color.Background = FB_BGRA8888_RED;
            FbConDrawglyph(Pixels, gWidth, (gBpp / 8), font5x12 + (0x20 - 32) * 2, 2); // full block cursor
            MicroSecondDelay( 2000 );
            m_Color.Background = FB_BGRA8888_BLACK;
            FbConDrawglyph(Pixels, gWidth, (gBpp / 8), font5x12 + (0x20 - 32) * 2, 2); // full block cursor
            MicroSecondDelay( 1000 );
        }
*/
	FbConDrawglyph(
		Pixels,
		gWidth,
		(gBpp / 8),
		font5x12 + (c - 32) * 2,
		scale_factor);

	m_Position.x++;

	if (m_Position.x >= (int)(m_MaxPosition.x / scale_factor)) goto newline2;

	if (intstate) ArmEnableInterrupts();
	return;

newline:
	//  delay....

	// blank padding whole line before switching to the new line
//	if (m_Position.x < (int)(m_MaxPosition.x / scale_factor))
//	{
//	    for (int z = m_Position.x; z < (int)(m_MaxPosition.x / scale_factor); z++)
//		{
////		    Pixels += m_Position.x * 2 * ((gBpp / 8) * FONT_WIDTH );
////        	    Pixels += m_Position.x;// * ((gBpp / 8) + (FONT_WIDTH + 4));
//		    Pixels += m_Position.x * 2 * ((gBpp / 8) * (FONT_WIDTH + 1));
//        	    FbConDrawglyph(Pixels, gWidth, (gBpp / 8), font5x12 + ( 0x20 - 32) * 2, 2);
//		    m_Position.x++;
//		}
//	}

	MicroSecondDelay( delay ); 
newline2:

	m_Position.y += scale_factor;
	m_Position.x = 0;
	if (m_Position.y >= m_MaxPosition.y - scale_factor)
	{
//		FbConScrollUp();
		ResetFb();
//		ResetFbHeader();
//		ResetFbFooter();
		FbConFlush();
		m_Position.y = 0;

		if (intstate) ArmEnableInterrupts();
		goto paint;
	}
	else
	{
		FbConFlush();
		if (intstate) ArmEnableInterrupts();
	}

}

void FbConDrawglyph
(
	char *pixels,
	unsigned stride,
	unsigned bpp,
	unsigned *glyph,
	unsigned scale_factor
)
{
	char *bg_pixels = pixels;
	unsigned x, y, i, j, k;
	unsigned data, temp;
	unsigned int fg_color = m_Color.Foreground;
	unsigned int bg_color = m_Color.Background;
	stride -= FONT_WIDTH * scale_factor;

	for (y = 0; y < FONT_HEIGHT / 2; ++y)
	{
		for (i = 0; i < scale_factor; i++)
		{
			for (x = 0; x < FONT_WIDTH; ++x)
			{
				for (j = 0; j < scale_factor; j++)
				{
					bg_color = m_Color.Background;
					for (k = 0; k < bpp; k++)
					{
						*bg_pixels = (unsigned char)bg_color;
						bg_color = bg_color >> 8;
						bg_pixels++;
					}
				}
			}
			bg_pixels += (stride * bpp);
		}
	}

	for (y = 0; y < FONT_HEIGHT / 2; ++y)
	{
		for (i = 0; i < scale_factor; i++)
		{
			for (x = 0; x < FONT_WIDTH; ++x)
			{
				for (j = 0; j < scale_factor; j++)
				{
					bg_color = m_Color.Background;
					for (k = 0; k < bpp; k++)
					{
						*bg_pixels = (unsigned char)bg_color;
						bg_color = bg_color >> 8;
						bg_pixels++;
					}
				}
			}
			bg_pixels += (stride * bpp);
		}
	}

	data = glyph[0];
	for (y = 0; y < FONT_HEIGHT / 2; ++y)
	{
		temp = data;
		for (i = 0; i < scale_factor; i++)
		{
			data = temp;
			for (x = 0; x < FONT_WIDTH; ++x)
			{
				if (data & 1)
				{
					for (j = 0; j < scale_factor; j++)
					{
						fg_color = m_Color.Foreground;
						for (k = 0; k < bpp; k++)
						{
							*pixels = (unsigned char)fg_color;
							fg_color = fg_color >> 8;
							pixels++;
						}
					}
				}
				else
				{
					for (j = 0; j < scale_factor; j++)
					{
						pixels = pixels + bpp;
					}
				}
				data >>= 1;
			}
			pixels += (stride * bpp);
		}
	}

	data = glyph[1];
	for (y = 0; y < FONT_HEIGHT / 2; ++y)
	{
		temp = data;
		for (i = 0; i < scale_factor; i++)
		{
			data = temp;
			for (x = 0; x < FONT_WIDTH; ++x)
			{
				if (data & 1)
				{
					for (j = 0; j < scale_factor; j++)
					{
						fg_color = m_Color.Foreground;
						for (k = 0; k < bpp; k++)
						{
							*pixels = (unsigned char)fg_color;
							fg_color = fg_color >> 8;
							pixels++;
						}
					}
				}
				else
				{
					for (j = 0; j < scale_factor; j++)
					{
						pixels = pixels + bpp;
					}
				}
				data >>= 1;
			}
			pixels += (stride * bpp);
		}
	}
}

/* TODO: Take stride into account */
void FbConScrollUp(void)
{
	unsigned short *dst = (void*)FixedPcdGet32(PcdMipiFrameBufferAddress);
	unsigned short *src = dst + (gWidth * FONT_HEIGHT);
	unsigned count = gWidth * (gHeight - FONT_HEIGHT);

	while (count--)
	{
		*dst++ = *src++;
	}

	count = gWidth * FONT_HEIGHT;
	while (count--)
	{
		*dst++ = m_Color.Background;
	}

	FbConFlush();
}

void FbConFlush(void)
{
	unsigned total_x, total_y;
	unsigned bytes_per_bpp;

	total_x = gWidth;
	total_y = gHeight;
	bytes_per_bpp = (gBpp / 8);

	WriteBackInvalidateDataCacheRange(
		(void*)FixedPcdGet32(PcdMipiFrameBufferAddress),
		(total_x * total_y * bytes_per_bpp)
	);
}

UINTN
EFIAPI
SerialPortWrite
(
	IN UINT8     *Buffer,
	IN UINTN     NumberOfBytes
)
{
	UINT8* CONST Final = &Buffer[NumberOfBytes];
	UINTN  InterruptState = ArmGetInterruptState();
	ArmDisableInterrupts();

	while (Buffer < Final)
	{
		FbConPutCharWithFactor(*Buffer++, FBCON_COMMON_MSG, SCALE_FACTOR);
	}

	if (InterruptState) ArmEnableInterrupts();
	return NumberOfBytes;
}

UINTN
EFIAPI
SerialPortWriteCritical
(
	IN UINT8     *Buffer,
	IN UINTN     NumberOfBytes
)
{
	UINT8* CONST Final = &Buffer[NumberOfBytes];
	UINTN  CurrentForeground = m_Color.Foreground;
	UINTN  InterruptState = ArmGetInterruptState();

	ArmDisableInterrupts();

//	ResetFb();
//	FbConReset();
//	FbConFlush();

	m_Color.Foreground = FB_BGRA8888_YELLOW;

	while (Buffer < Final)
	{
		FbConPutCharWithFactor(*Buffer++, FBCON_COMMON_MSG, SCALE_FACTOR);
	}

	m_Color.Foreground = CurrentForeground;

	if (InterruptState) ArmEnableInterrupts();
	return NumberOfBytes;
}

UINTN
EFIAPI
SerialPortRead
(
	OUT UINT8     *Buffer,
	IN  UINTN     NumberOfBytes
)
{
	return 0;
}

BOOLEAN
EFIAPI
SerialPortPoll
(
	VOID
)
{
	return FALSE;
}

RETURN_STATUS
EFIAPI
SerialPortSetControl
(
	IN UINT32 Control
)
{
	return RETURN_UNSUPPORTED;
}

RETURN_STATUS
EFIAPI
SerialPortGetControl
(
	OUT UINT32 *Control
)
{
	return RETURN_UNSUPPORTED;
}

RETURN_STATUS
EFIAPI
SerialPortSetAttributes
(
	IN OUT UINT64             *BaudRate,
	IN OUT UINT32             *ReceiveFifoDepth,
	IN OUT UINT32             *Timeout,
	IN OUT EFI_PARITY_TYPE    *Parity,
	IN OUT UINT8              *DataBits,
	IN OUT EFI_STOP_BITS_TYPE *StopBits
)
{
	return RETURN_UNSUPPORTED;
}

UINTN SerialPortFlush(VOID)
{
	return 0;
}

VOID
EnableSynchronousSerialPortIO(VOID)
{
	// Already synchronous
}
