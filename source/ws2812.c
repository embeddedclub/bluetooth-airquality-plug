/******************************************************************************
* File Name:   WS2812_PSoC6_HAL.c
*
* Description:
* Using SCB/SPI to drive WS2812B LEDs
*
******************************************************************************
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
* SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
* FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"
#include "bt_app.h"
#include "ws2812.h"
//#include "cy_result.h"

/*******************************************************************************
* Macros
*******************************************************************************/
/* SPI baud rate in Hz */
#define SPI_FREQ_HZ                (2400000UL)

#define WS2812_ZERO (4) // Do not change WS2812 '0' bit sequence
#define WS2812_ONE  (6) // Do not change WS2812 '1' bit sequence

cyhal_spi_t mSPI;

uint8_t u8WS2812_BUFFER[1 + (WS2812_CNT * 9)];
uint32_t u32LedIntensity;

uint32_t WS2812_byte2binary(uint8_t u8Byte)
{
    uint8_t u8i;
    uint32_t u32result;

    // '0' -> 100
    // '1' -> 110

    u32result = 0;

    // Prozess 7 upper bits
    for (u8i=0; u8i<7; u8i++)
    {
        if  (u8Byte & 0x80)
        {
            u32result =  u32result + WS2812_ONE; // '1'
        }
        else
        {
            u32result =  u32result + WS2812_ZERO; // '0'
        }

        u32result = u32result << 3; // next WS2812 bit
        u8Byte = u8Byte << 1; // source byte -> bit shift left

    } // for (u8i ...)

    // LSB without final shift
    if  (u8Byte & 0x80)
    {
        u32result =  u32result + WS2812_ONE; // '1'
    }
    else
    {
        u32result =  u32result + WS2812_ZERO; // '0'
    }

    return(u32result);
}

cy_rslt_t StripLights_Init()
{
	cy_rslt_t result;

	result = cyhal_spi_init(&mSPI, CYBSP_SPI_MOSI, NC, NC, NC, NULL, 8, CYHAL_SPI_MODE_00_MSB, false);

	if (CY_RSLT_SUCCESS == result)
	{
		result = cyhal_spi_set_frequency(&mSPI, SPI_FREQ_HZ);
	}
	return (result);
}

void StripLights_Intensity(uint32_t u32value)
{
		u32LedIntensity = u32value;
}

void StripLights_Trigger(uint32_t u32enable)
{
    if (u32enable)
    {
        cyhal_spi_transfer(&mSPI, &u8WS2812_BUFFER[0], 1+(WS2812_CNT * 9), 0, 0, 0);
        CyDelayUs(70); // WS2812 Reset
    }
}

void  StripLights_MemClear(uint32_t u32color)
{
    uint16_t u16cnt;
    uint8_t *pu8buffer;
    uint32_t u32R, u32G, u32B;
    uint8_t u8Rhigh, u8Rmid, u8Rlow; // high-, mid-, low-nibble
    uint8_t u8Ghigh, u8Gmid, u8Glow; // high-, mid-, low-nibble
    uint8_t u8Bhigh, u8Bmid, u8Blow; // high-, mid-, low-nibble

    if (u32LedIntensity == 0)
    {
        u32R = WS2812_byte2binary(0);
        u8Rhigh = u32R >> 16; u8Ghigh = u8Rhigh; u8Bhigh = u8Rhigh;
        u8Rmid  = u32R >> 8;  u8Gmid  = u8Rmid;  u8Bmid  = u8Rmid;
        u8Rlow  = u32R >> 0;  u8Glow  = u8Rlow;  u8Blow  = u8Rlow;
    }
    else
	{
		#if WS2812B_TYPE == 0

			u32R = WS2812_byte2binary(((u32color>>16)&0xFF)/u32LedIntensity);
			u8Rhigh = u32R >> 16;
			u8Rmid  = u32R >> 8;
			u8Rlow  = u32R >> 0;

			u32G = WS2812_byte2binary(((u32color>>8)&0xFF)/u32LedIntensity);
			u8Ghigh = u32G >> 16;
			u8Gmid  = u32G >> 8;
			u8Glow  = u32G >> 0;

			u32B = WS2812_byte2binary(((u32color>>0)&0xFF)/u32LedIntensity);
			u8Bhigh = u32B >> 16;
			u8Bmid  = u32B >> 8;
			u8Blow  = u32B >> 0;

		#elif WS2812B_TYPE == 1

			u32R = WS2812_byte2binary(((u32color>>8)&0xFF)/u32LedIntensity);
			u8Rhigh = u32R >> 16;
			u8Rmid  = u32R >> 8;
			u8Rlow  = u32R >> 0;

			u32G = WS2812_byte2binary(((u32color>>16)&0xFF)/u32LedIntensity);
			u8Ghigh = u32G >> 16;
			u8Gmid  = u32G >> 8;
			u8Glow  = u32G >> 0;

			u32B = WS2812_byte2binary(((u32color>>0)&0xFF)/u32LedIntensity);
			u8Bhigh = u32B >> 16;
			u8Bmid  = u32B >> 8;
			u8Blow  = u32B >> 0;

		#else

			#error "WS2812B_TYPE not defined"

		#endif
	}

    u8WS2812_BUFFER[0] = 0x00; // pre-byte for preparing a synced SPI stream
    pu8buffer = &u8WS2812_BUFFER[1]; u16cnt =  WS2812_CNT;
    while (u16cnt--)
    {
        *pu8buffer++ = u8Ghigh;
        *pu8buffer++ = u8Gmid;
        *pu8buffer++ = u8Glow;
        *pu8buffer++ = u8Rhigh;
        *pu8buffer++ = u8Rmid;
        *pu8buffer++ = u8Rlow;
        *pu8buffer++ = u8Bhigh;
        *pu8buffer++ = u8Bmid;
        *pu8buffer++ = u8Blow;
    }
}

void StripLights_Pixel(uint32_t u32pos, uint32_t u32color)
{
    uint8_t *pu8buffer;
    uint32_t u32R, u32G, u32B;
    uint8_t u8Rhigh, u8Rmid, u8Rlow; // high-, mid-, low-nibble
    uint8_t u8Ghigh, u8Gmid, u8Glow; // high-, mid-, low-nibble
    uint8_t u8Bhigh, u8Bmid, u8Blow; // high-, mid-, low-nibble

    if (u32LedIntensity == 0)
    {
        u32R = WS2812_byte2binary(0);
        u8Rhigh = u32R >> 16; u8Ghigh = u8Rhigh; u8Bhigh = u8Rhigh;
        u8Rmid  = u32R >> 8;  u8Gmid  = u8Rmid;  u8Bmid  = u8Rmid;
        u8Rlow  = u32R >> 0;  u8Glow  = u8Rlow;  u8Blow  = u8Rlow;
    }
    else
    {
		#if WS2812B_TYPE == 0

			u32R = WS2812_byte2binary(((u32color>>16)&0xFF)/u32LedIntensity);
			u8Rhigh = u32R >> 16;
			u8Rmid  = u32R >> 8;
			u8Rlow  = u32R >> 0;

			u32G = WS2812_byte2binary(((u32color>>8)&0xFF)/u32LedIntensity);
			u8Ghigh = u32G >> 16;
			u8Gmid  = u32G >> 8;
			u8Glow  = u32G >> 0;

			u32B = WS2812_byte2binary(((u32color>>0)&0xFF)/u32LedIntensity);
			u8Bhigh = u32B >> 16;
			u8Bmid  = u32B >> 8;
			u8Blow  = u32B >> 0;

		#elif WS2812B_TYPE == 1

			u32R = WS2812_byte2binary(((u32color>>8)&0xFF)/u32LedIntensity);
			u8Rhigh = u32R >> 16;
			u8Rmid  = u32R >> 8;
			u8Rlow  = u32R >> 0;

			u32G = WS2812_byte2binary(((u32color>>16)&0xFF)/u32LedIntensity);
			u8Ghigh = u32G >> 16;
			u8Gmid  = u32G >> 8;
			u8Glow  = u32G >> 0;

			u32B = WS2812_byte2binary(((u32color>>0)&0xFF)/u32LedIntensity);
			u8Bhigh = u32B >> 16;
			u8Bmid  = u32B >> 8;
			u8Blow  = u32B >> 0;

		#else

			#error "WS2812B_TYPE not defined"

		#endif
    }

    pu8buffer = &u8WS2812_BUFFER[1+(9*u32pos)];

    *pu8buffer++ = u8Ghigh;
    *pu8buffer++ = u8Gmid;
    *pu8buffer++ = u8Glow;
    *pu8buffer++ = u8Rhigh;
    *pu8buffer++ = u8Rmid;
    *pu8buffer++ = u8Rlow;
    *pu8buffer++ = u8Bhigh;
    *pu8buffer++ = u8Bmid;
    *pu8buffer++ = u8Blow;
}

/* [] END OF FILE */
