/******************************************************************************
* File Name:   WS2812_PSoC6_HAL.h
*
* Description:
* Headerfile related to WS2812_PSoC6_HAL.c
* keeping user related settings and API
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

#define WS2812_CNT 		(1) 	// <<< Define number of WS2812B LEDs
#define WS2812B_TYPE	(1) 	// 0: by default the WS2812B uses 24bit data format [G8R8B8], 1: Use 24bit data format [R8G8B8]

// predefined colors
#define WS2812_BLACK   (0x00000000)
#define WS2812_RED     (0x00FF0000)
#define WS2812_GREEN   (0x0000FF00)
#define WS2812_BLUE    (0x000000FF)
#define WS2812_CYAN    (0x0000FFFF)
#define WS2812_YELLOW  (0x00FFFF00)
#define WS2812_MAGENTA (0x00FF00FF)
#define WS2812_ORANGE  (0x00FF8000)
#define WS2812_WHITE   (0x00FFFFFF)




// APIs

/** Initialise the WS2812B interface using SCB 
 *
 * Initilalizes the SCB interface to be used for driving the WS2812B
 * @param[in]	u32Color 	define the background color, 24-bit R8G8B8, normally WS2812_BLACK   (0x00000000)
 * @return 		none
 */
cy_rslt_t StripLights_Init();

/** Define the brightness 
 *
 * The given value will taken as a divider of the maximum brightness. A value of 1 will be maximimum brightness.
 * @param[in]	u32value 	brighness divider, 0=off, 1=maximum
 * @return 		none
 */
void StripLights_Intensity(uint32_t u32value);

/** Clear the internal LED buffer
 *
 * Clear respectively set the internal LED buffer to one specified color, normally WS2812_BLACK   (0x00000000)
 * @param[in]	u32Color 	define the background color, 24-bit R8G8B8, normally WS2812_BLACK   (0x00000000)
 * @return 		none
 */
void  StripLights_MemClear(uint32_t u32Color);

/** Set color of one LED
 *
 * Set the LED at the given position to the given RGB color
 * @param[in]	u32pos       : Position of LED 0 .. (WS2812_CNT - 1)
 * @param[in]	u32color     : 24-bit color R8G8B8
 * @return 		none
 */
void StripLights_Pixel(uint32_t u32pos, uint32_t u32color);

/** Trigger to update of LED string
 *
 * The internal software LED buffer is transferred tothe external LED string.
 * @param[in]	u32enable 	1: active trigger to send the data stream
 * @return 		none
 */
void StripLights_Trigger(uint32_t u32enable);

/* [] END OF FILE */
