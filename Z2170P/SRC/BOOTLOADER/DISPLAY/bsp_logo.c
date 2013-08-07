// All rights reserved ADENEO EMBEDDED 2010
// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File:  bsp_logo.c
//


//------------------------------------------------------------------------------
//
// includes
//

#include "bsp.h"
#include "bsp_logo.h"
#include "omap_dss_regs.h"
#include "lcd.h"
#include "oalex.h"
#include "eboot.h"
#include "sdk_gpio.h"

#include "twl.h"
#include "triton.h"
#include "tps659xx_internals.h"
//------------------------------------------------------------------------------
//
// prototypes
//
void reset_display_controller( void );
void disable_dss( void );
UINT32 enable_lcd_power( void );
UINT32 enable_lcd_backlight( void );
void configure_dss( UINT32 framebuffer );
void display_lcd_image( void );
void lcd_config(UINT32 framebuffer);
void lcd_shutdown(void);
UINT32 disable_lcd_power(void);
UINT32 disable_lcd_backlight(void);
void lcm_config(void);



//------------------------------------------------------------------------------
//
// defines
//
#define LOGO_WIDTH                  320	//480    // Logo bitmap image is RGB24 VGA Portrait bitmap
#define LOGO_HEIGHT                 240	//640

#define BYTES_PER_PIXEL             3
#define DELAY_COUNT                 100 
#define LOGO_GFX_ATTRIBUTES         (DISPC_GFX_ATTR_GFXENABLE | DISPC_GFX_ATTR_GFXFORMAT(DISPC_PIXELFORMAT_RGB24))           // RGB24 packed, enabled

#define BSP_LCD_CONFIG              (DISPC_CONFIG_FUNCGATED | DISPC_CONFIG_LOADMODE(2))

#define BSP_GFX_POS                 (DISPC_GFX_POS_GFXPOSY(g_dwLogoPosY) | DISPC_GFX_POS_GFXPOSX(g_dwLogoPosX))
#define BSP_GFX_SIZE                (DISPC_GFX_SIZE_GFXSIZEY(g_dwLogoHeight) | DISPC_GFX_SIZE_GFXSIZEX(g_dwLogoWidth))
#define BSP_GFX_FIFO_THRESHOLD      (DISPC_GFX_FIFO_THRESHOLD_LOW(192) | DISPC_GFX_FIFO_THRESHOLD_HIGH(252))
#define BSP_GFX_ROW_INC             0x00000001
#define BSP_GFX_PIXEL_INC           0x00000001
#define BSP_GFX_WINDOW_SKIP         0x00000000
//------------------------------------------------------------------------------
//defines SPI1, Ray 13-08-06

//brief  SPI Init structure definition  
typedef struct
{
  UINT32 SPI1_Direction;           /*!< Specifies the SPI unidirectional or bidirectional data mode.
                                         This parameter can be a value of @ref SPI_data_direction */

  UINT32 SPI1_Mode;                /*!< Specifies the SPI operating mode.
                                         This parameter can be a value of @ref SPI_mode */

  UINT32 SPI1_DataSize;            /*!< Specifies the SPI data size.
                                         This parameter can be a value of @ref SPI_data_size */

  UINT32 SPI1_CPOL;                /*!< Specifies the serial clock steady state.
                                         This parameter can be a value of @ref SPI_Clock_Polarity */

  UINT32 SPI1_CPHA;                /*!< Specifies the clock active edge for the bit capture.
                                         This parameter can be a value of @ref SPI_Clock_Phase */

  UINT32 SPI1_CS0;                 /*!< Specifies whether the NSS signal is managed by
                                         hardware (NSS pin) or by software using the SSI bit.
                                         This parameter can be a value of @ref SPI_Slave_Select_management */
 
  UINT32 SPI1_BaudRatePrescaler;   /*!< Specifies the Baud Rate prescaler value which will be
                                         used to configure the transmit and receive SCK clock.
                                         This parameter can be a value of @ref SPI_BaudRate_Prescaler
                                         @note The communication clock is derived from the master
                                               clock. The slave clock does not need to be set. */

  UINT32 SPI1_FirstBit;            /*!< Specifies whether data transfers start from MSB or LSB bit.
                                         This parameter can be a value of @ref SPI_MSB_LSB_transmission */

  UINT32 SPI1_CRCPolynomial;       /*!< Specifies the polynomial used for the CRC calculation. */
}SPI_InitTypeDef;

typedef struct
{
  UINT32 GPIO_Pin;              /*!< Specifies the GPIO pins to be configured.
                                       This parameter can be any value of @ref GPIO_pins_define */

  GPIOMode_TypeDef GPIO_Mode;     /*!< Specifies the operating mode for the selected pins.
                                       This parameter can be a value of @ref GPIOMode_TypeDef */

  GPIOSpeed_TypeDef GPIO_Speed;   /*!< Specifies the speed for the selected pins.
                                       This parameter can be a value of @ref GPIOSpeed_TypeDef */

  GPIOOType_TypeDef GPIO_OType;   /*!< Specifies the operating output type for the selected pins.
                                       This parameter can be a value of @ref GPIOOType_TypeDef */

  GPIOPuPd_TypeDef GPIO_PuPd;     /*!< Specifies the operating Pull-up/Pull down for the selected pins.
                                       This parameter can be a value of @ref GPIOPuPd_TypeDef */
}GPIO_InitTypeDef;
//------------------------------------------------------------------------------



DWORD   g_dwLogoPosX;
DWORD   g_dwLogoPosY;

DWORD   g_dwLogoWidth;
DWORD   g_dwLogoHeight;




static void FlipFrameBuffer(PUCHAR fb, DWORD h, DWORD lineSize,PUCHAR temporaryBuffer)
{
    DWORD y;
    PUCHAR top;
    PUCHAR bottom;

    top = fb;
    bottom = fb + ((h-1)*lineSize);
    
    for (y=0;y<h/2;y++)
    {
        memcpy(temporaryBuffer,top,lineSize);
        memcpy(top,bottom,lineSize);
        memcpy(bottom,temporaryBuffer,lineSize);
        top += lineSize;
        bottom -= lineSize;
    }
}
//------------------------------------------------------------------------------
//
//  Function:  ShowLogo
//
//  This function shows the logo splash screen
//
//BOOL ShowLogo(UINT32 flashAddr, UINT32 offset)
VOID ShowLogo(UINT32 flashAddr, UINT32 offset)
{
    HANDLE  hFlash = NULL;
    DWORD  framebuffer;
    DWORD  framebufferPA;
    PUCHAR  pChar;
    ULONG   x, y;
    WORD    wSignature = 0;
    DWORD   dwOffset = 0;
    DWORD   dwLcdWidth,
            dwLcdHeight;
    DWORD   dwLength;
	
	//  Get the LCD width and height
    LcdPdd_LCD_GetMode( NULL, &dwLcdWidth, &dwLcdHeight, NULL );
	
    dwLength = BYTES_PER_PIXEL * LOGO_WIDTH * LOGO_HEIGHT;

    //  Get the video memory
    LcdPdd_GetMemory( NULL, &framebufferPA );
    framebuffer = (DWORD) OALPAtoUA(framebufferPA);
    pChar = (PUCHAR)framebuffer;
    
   if (flashAddr != -1)
    {
        // Open flash storage
        hFlash = OALFlashStoreOpen(flashAddr);
        if( hFlash != NULL )
        {
            //  The LOGO reserved NAND flash region contains the BMP file
            OALFlashStoreBufferedRead( hFlash, offset, (UCHAR*) &wSignature, sizeof(wSignature), FALSE );

            //  Check for 'BM' signature
            if( wSignature == 0x4D42 )  
            {
                //  Read the offset to the pixel data
                OALFlashStoreBufferedRead( hFlash, offset + 10, (UCHAR*) &dwOffset, sizeof(dwOffset), FALSE );

                //  Read the pixel data with the given offset
                OALFlashStoreBufferedRead( hFlash, offset + dwOffset, pChar, dwLength, FALSE );
            }
           
            //  Close store
            OALFlashStoreClose(hFlash);
        
            //  Compute position and size of logo image 
            g_dwLogoPosX   = (dwLcdWidth - LOGO_WIDTH)/2;
            g_dwLogoPosY   = (dwLcdHeight - LOGO_HEIGHT)/2;
            g_dwLogoWidth  = LOGO_WIDTH;
            g_dwLogoHeight = LOGO_HEIGHT;
            
            //As BMP are stored upside down, we need to flip the frame buffer's content
            FlipFrameBuffer((PUCHAR)framebuffer, LOGO_HEIGHT,LOGO_WIDTH*BYTES_PER_PIXEL, (PUCHAR)framebuffer + dwLength);
        }
    }

    //  If bitmap signature is valid, display the logo, otherwise fill screen with pattern
    // if( wSignature == 0x4D42 )			Ray 13-08-01 
   if( wSignature != 0x4D42 )		
	{
   
		//  Adjust color bars to LCD size
		g_dwLogoPosX   = 0;
        g_dwLogoPosY   = 0;
        g_dwLogoWidth  = dwLcdWidth;
        g_dwLogoHeight = dwLcdHeight;
        
        for (y= 0; y < dwLcdHeight; y++)
        {
            for( x = 0; x < dwLcdWidth; x++ )
            {
				 *pChar++ = 0x00;    //  Blue	
                 *pChar++ = 0x00;    //  Green
                 *pChar++ = 0xFF;    //  Red
				/*if( y < dwLcdHeight/2 )
                {
                    if( x < dwLcdWidth/2 )
                    {
                       *pChar++ = 0x00;    //  Blue	
                        *pChar++ = 0x00;    //  Green
                        *pChar++ = 0xFF;    //  Red
						//*pChar++ = 0x00;    //  Blue	//Ray 13-07-31
                       // *pChar++ = 0x00;    //  Green
                       // *pChar++ = 0x00;    //  Red

                    }
                    else
                    {
                        /**pChar++ = 0x00;    //  Blue	
                        *pChar++ = 0xFF;    //  Green
                        *pChar++ = 0x00;    //  Red*/
						/**pChar++ = 0x00;    //  Blue	//Ray 13-07-31
                        *pChar++ = 0x00;    //  Green
                        *pChar++ = 0xFF;    //  Red
                    }
                }
                else
                {
                    if( x < dwLcdWidth/2 )
                    {
                        /**pChar++ = 0xFF;    //  Blue	
                        *pChar++ = 0x00;    //  Green
                        *pChar++ = 0x00;    //  Red*/
						/**pChar++ = 0x00;    //  Blue	//Ray 13-07-31
                        *pChar++ = 0x00;    //  Green
                        *pChar++ = 0xFF;    //  Red
                    }
                    else
                    {
						/**pChar++ = 0x00;    //  Blue	
                        *pChar++ = 0xFF;    //  Green
                        *pChar++ = 0xFF;    //  Red*/
						/**pChar++ = 0x00;    //  Blue	//Ray 13-07-31
                        *pChar++ = 0x00;    //  Green
                        *pChar++ = 0xFF;    //  Red
				    }
                }*/
            }
        }
    }

    //  Fire up the LCD
    lcd_config(framebufferPA); 
}

//------------------------------------------------------------------------------
//
//  Function:   ShowSDLogo
//
//  This function is called to display the splaschreen bitmap from the SDCard
//
//
BOOL ShowSDLogo()
{
    DWORD	framebuffer = 0;
    DWORD	framebufferPA = 0;
    DWORD	dwLcdWidth = 0;
    DWORD	dwLcdHeight = 0;
	DWORD	dwLength = 0;
	
    // Get the LCD width and height
    LcdPdd_LCD_GetMode( NULL, &dwLcdWidth, &dwLcdHeight, NULL );
	OALMSG(OAL_INFO, (L"ShowSDLogo: dwLcdWidth = %d, dwLcdHeight = %d\r\n",dwLcdWidth,dwLcdHeight));
	
	// Get the frame buffer
	LcdPdd_GetMemory( NULL, &framebufferPA );
	OALMSG(OAL_INFO, (L"ShowSDLogo: framebuffer = 0x%x\r\n",framebuffer));
    framebuffer = (DWORD) OALPAtoUA(framebufferPA);
	OALMSG(OAL_INFO, (L"ShowSDLogo: framebuffer OALPAtoUA = 0x%x\r\n",framebuffer));
	
	// Compute the size
	dwLength = BYTES_PER_PIXEL * LOGO_WIDTH * LOGO_HEIGHT;
	OALMSG(OAL_INFO, (L"ShowSDLogo: BYTES_PER_PIXEL = %d\r\n",BYTES_PER_PIXEL));
	OALMSG(OAL_INFO, (L"ShowSDLogo: LOGO_WIDTH = %d\r\n",LOGO_WIDTH));
	OALMSG(OAL_INFO, (L"ShowSDLogo: LOGO_HEIGHT = %d\r\n",LOGO_HEIGHT));
	OALMSG(OAL_INFO, (L"ShowSDLogo: size = %d\r\n",dwLength));
	
	if (!BLSDCardReadLogo(L"LOGO.bmp", (UCHAR*)framebuffer, dwLength))	//Loadin file name(LOGO.bmp), Ray 13-08-01
	{
		return FALSE;
	}

    //  Compute position and size of logo image 
    g_dwLogoPosX   = (dwLcdWidth - LOGO_WIDTH)/2;
    g_dwLogoPosY   = (dwLcdHeight - LOGO_HEIGHT)/2;
    g_dwLogoWidth  = LOGO_WIDTH;
    g_dwLogoHeight = LOGO_HEIGHT;
	OALMSG(OAL_INFO, (L"ShowSDLogo: g_dwLogoPosX = %d,g_dwLogoPosY = %d\r\n",g_dwLogoPosX,g_dwLogoPosY));
	OALMSG(OAL_INFO, (L"ShowSDLogo: g_dwLogoWidth = %d,g_dwLogoHeight = %d\r\n",g_dwLogoWidth,g_dwLogoHeight));
    //As BMP are stored upside down, we need to flip the frame buffer's content
    FlipFrameBuffer((PUCHAR)framebuffer,LOGO_HEIGHT,LOGO_WIDTH*BYTES_PER_PIXEL,(PUCHAR)framebuffer + dwLength);

    //  Fire up the LCD
    lcd_config(framebufferPA);

	//	Fire up the LCM, Ray 13-08-06.
	lcm_config();
	
	return TRUE;
	//return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  HideLogo
//
//  This function hides the logo splash screen
//
VOID HideLogo(VOID)
{
    lcd_shutdown();
}

//------------------------------------------------------------------------------
//
//  Function:  reset_display_controller
//
//  This function resets the Display Sub System on omap24xx
//
void reset_display_controller( void )
{
    
    UINT32 reg_val;
    UINT16 count;
    UINT32 timeout;
    UINT32 fclk, iclk;
    OMAP_PRCM_DSS_CM_REGS *pPrcmRegs = OALPAtoUA(OMAP_PRCM_DSS_CM_REGS_PA);
    OMAP_DISPC_REGS  *pDisplayRegs = OALPAtoUA(OMAP_DISC1_REGS_PA);
    
	OALMSG(OAL_INFO, (L"reset_display_controller+\r\n"));

    // enable all display clocks
    fclk = INREG32(&pPrcmRegs->CM_FCLKEN_DSS); // functional clock
    iclk = INREG32(&pPrcmRegs->CM_ICLKEN_DSS); // interconnect clock

    OUTREG32(&pPrcmRegs->CM_FCLKEN_DSS, (fclk | CM_CLKEN_DSS1 | CM_CLKEN_DSS2));
    OUTREG32(&pPrcmRegs->CM_ICLKEN_DSS, (iclk | CM_CLKEN_DSS));
  
    // disable the display controller
    //disable_dss();

    // reset the display controller
    OUTREG32(&pDisplayRegs->DISPC_SYSCONFIG, DISPC_SYSCONFIG_SOFTRESET);
    
    // wait until reset completes OR timeout occurs
    timeout=10000;
    while(!((reg_val=INREG32(&pDisplayRegs->DISPC_SYSSTATUS)) & DISPC_SYSSTATUS_RESETDONE) && (timeout > 0))
    {
        // delay
        for(count=0;count<DELAY_COUNT;++count);
        timeout--;
    }

    if(!(reg_val & DISPC_SYSSTATUS_RESETDONE))
    {
        // OALMSG(OAL_INFO, (L"reset_display_controller: DSS reset timeout\r\n"));
    }
    
    reg_val=INREG32(&pDisplayRegs->DISPC_SYSCONFIG);
    reg_val &=~(DISPC_SYSCONFIG_SOFTRESET);
    OUTREG32(&pDisplayRegs->DISPC_SYSCONFIG,reg_val);


    // restore old clock settings
    OUTREG32(&pPrcmRegs->CM_FCLKEN_DSS, fclk);
    OUTREG32(&pPrcmRegs->CM_ICLKEN_DSS, iclk);
    
	OALMSG(OAL_INFO, (L"reset_display_controller-\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  lcd_config
//
//  This function configures the LCD
//
void lcd_config(UINT32 framebuffer)
{    
    reset_display_controller();

    // Enable LCD clocks
    enable_lcd_power();

    // Configure the DSS registers
    configure_dss(framebuffer);
      
    // Display data on LCD
    display_lcd_image() ;
    
	// Turn on backlight last
	enable_lcd_backlight();

}

//------------------------------------------------------------------------------
//
//  Function:  lcd_shutdown
//
//  This function disables the backlight and power of the LCD controller
//
void lcd_shutdown()
{
    disable_lcd_backlight();
    LcdPdd_SetPowerLevel(D4);
    disable_lcd_power();
}   

//------------------------------------------------------------------------------
//
//  Function:  disable_dss
//
//  This function disables the Display Sub System on omap24xx
//
void disable_dss( void )
{
     //OALMSG(OAL_INFO, (L"disable_dss+\r\n"));

     //OALMSG(OAL_INFO, (L"disable_dss-\r\n"));
}
     
//------------------------------------------------------------------------------
//
//  Function:  enable_lcd_power
//
//  This function enables the power for the LCD controller
//
UINT32 enable_lcd_power( void )
{
    OMAP_PRCM_DSS_CM_REGS *pPrcmRegs = OALPAtoUA(OMAP_PRCM_DSS_CM_REGS_PA);
    
	OALMSG(OAL_INFO, (L"enable DSS1_ALWON_FCLK, DSS2_ALWON_FCLK,\r\n"));
	OALMSG(OAL_INFO, (L"enable DSS_L3_ICLK, DSS_L4_ICLK,\r\n"));
    SETREG32(&pPrcmRegs->CM_FCLKEN_DSS, (CM_CLKEN_DSS1 | CM_CLKEN_DSS2));
    SETREG32(&pPrcmRegs->CM_ICLKEN_DSS, (CM_CLKEN_DSS));

	//OALMSG(OAL_INFO, (L"enable_lcd_power-\r\n"));

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  disable_lcd_power
//
//  This function disables the power for the LCD controller
//
UINT32 disable_lcd_power( void )
{
    OMAP_PRCM_DSS_CM_REGS *pPrcmRegs = OALPAtoUA(OMAP_PRCM_DSS_CM_REGS_PA);

     //OALMSG(OAL_INFO, (L"disable_lcd_power+\r\n"));

    CLRREG32(&pPrcmRegs->CM_FCLKEN_DSS, (CM_CLKEN_DSS1 | CM_CLKEN_DSS2));
    CLRREG32(&pPrcmRegs->CM_ICLKEN_DSS, (CM_CLKEN_DSS));
    
     //OALMSG(OAL_INFO, (L"disable_lcd_power-\r\n"));
    return ERROR_SUCCESS;
}
      
//------------------------------------------------------------------------------
//
//  Function:  enable_lcd_backlight
//
//  This function enables the backlight for the LCD controller
//
UINT32 enable_lcd_backlight( void )
{
    void* hTwl;
    
	OALMSG(OAL_INFO, (L"enable_lcd_backlight+\r\n"));

    // Enable LEDA on TPS659XX
    hTwl = TWLOpen();

	TWLWriteByteReg(hTwl, TWL_PMBR1, 0x04); // PWM0 function is enabled
	TWLWriteByteReg(hTwl, TWL_GPBR1, 0x05); // PWM0_ENABLE & PWM0_CLK_ENABLE
	TWLWriteByteReg(hTwl, TWL_PWM0OFF, 0x7F);
	TWLWriteByteReg(hTwl, TWL_PWM0ON, 0x40);
/*#ifdef BSP_EVM2
    TWLWriteByteReg(hTwl, TWL_LEDEN, 0x11);
    // Set PWM registers to same value to trigger 100% duty cycle
    TWLWriteByteReg(hTwl, TWL_PWMAOFF, 0x00);
    TWLWriteByteReg(hTwl, TWL_PWMAON, 0x00);
#else
    // The hardware design is completely backwards.  
    // In order to get 100% brightness, the LEDPWM must 
    // be disabled.
    // Clear LEDAON, LEDAPWM
    TWLWriteByteReg(hTwl, TWL_LEDEN, 0x00);
#endif */   
    TWLClose(hTwl);
    
     //OALMSG(OAL_INFO, (L"enable_lcd_backlight-\r\n"));
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  disable_lcd_backlight
//
//  This function disables the backlight for the LCD controller
//
UINT32 disable_lcd_backlight( void )
{
    HANDLE hTwl;

	OALMSG(OAL_INFO, (L"disable_lcd_backlight+\r\n"));
    // Enable LEDA on TPS659XX
    hTwl = TWLOpen();
	TWLWriteByteReg(hTwl, TWL_PMBR1, 0x00);
	TWLWriteByteReg(hTwl, TWL_GPBR1, 0x00);
	TWLWriteByteReg(hTwl, TWL_PWM0OFF, 0x7F);
/*#ifdef BSP_EVM2
    TWLWriteByteReg(hTwl, TWL_LEDEN, 0x00);
#else
    // The hardware design is completely backwards.  In order
    // to disable the LED control signal, the LEDPWM signal must 
    // be enabled 100%
    // Set LEDAON, LEDAPWM
    TWLWriteByteReg(hTwl, TWL_LEDEN, 0x11);
    // Set PWM registers to same value to trigger 100% duty cycle
    TWLWriteByteReg(hTwl, TWL_PWMAOFF, 0x00);
    TWLWriteByteReg(hTwl, TWL_PWMAON, 0x00);
#endif
*/    
    TWLClose(hTwl);
     //OALMSG(OAL_INFO, (L"disable_lcd_backlight-\r\n"));
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  configure_dss
//
//  This function configures the Display Sub System on omap35xx
//
void configure_dss( UINT32 framebuffer )
{
    OMAP_DSS_REGS   *pDSSRegs		= OALPAtoUA(OMAP_DSS1_REGS_PA);
    OMAP_DISPC_REGS *pDisplayRegs	= OALPAtoUA(OMAP_DISC1_REGS_PA);
    OMAP_RFBI_REGS	*pRfbiRegs		= OALPAtoUA(OMAP_RFBI1_REGS_PA);
    HANDLE hGpio = GPIOOpen();

	OALMSG(OAL_INFO, (L"\r\nconfigure_dss+\r\n"));

    //  Configure the clock source
    OUTREG32( &pDSSRegs->DSS_CONTROL, 
			DSS_CONTROL_DISPC_CLK_SWITCH_DSS1_ALWON | DSS_CONTROL_DSI_CLK_SWITCH_DSS1_ALWON);
    
    //  Configure interconnect parameters
    OUTREG32( &pDSSRegs->DSS_SYSCONFIG, DISPC_SYSCONFIG_AUTOIDLE );
    OUTREG32( &pDisplayRegs->DISPC_SYSCONFIG, DISPC_SYSCONFIG_AUTOIDLE|SYSCONFIG_NOIDLE|SYSCONFIG_NOSTANDBY );

    // Not enabling any interrupts
    OUTREG32( &pDisplayRegs->DISPC_IRQENABLE, 0x00000000 );
    
    //  Configure the LCD
    LcdPdd_LCD_Initialize( pDSSRegs, pDisplayRegs, pRfbiRegs, NULL);
    	
    //  Over-ride default LCD config
    OUTREG32(&pDisplayRegs->DISPC_CONFIG,BSP_LCD_CONFIG);
    
    
    // Configure Graphics Window
    //--------------------------
    
    OUTREG32(&pDisplayRegs->DISPC_GFX_BA0 ,framebuffer );
 
    // configure the position of graphics window
    OUTREG32(&pDisplayRegs->DISPC_GFX_POSITION,BSP_GFX_POS);
    
    // size of graphics window
    OUTREG32(&pDisplayRegs->DISPC_GFX_SIZE,BSP_GFX_SIZE);
    
    // GW Enabled, RGB24 packed, Little Endian
    OUTREG32(&pDisplayRegs->DISPC_GFX_ATTRIBUTES,LOGO_GFX_ATTRIBUTES);
    
    OUTREG32(&pDisplayRegs->DISPC_GFX_FIFO_THRESHOLD,BSP_GFX_FIFO_THRESHOLD);
    OUTREG32(&pDisplayRegs->DISPC_GFX_ROW_INC,BSP_GFX_ROW_INC); 
    OUTREG32(&pDisplayRegs->DISPC_GFX_PIXEL_INC,BSP_GFX_PIXEL_INC); 
    OUTREG32(&pDisplayRegs->DISPC_GFX_WINDOW_SKIP,BSP_GFX_WINDOW_SKIP);

    OALMSG(OAL_INFO, (L"configure_dss-\r\n"));

	GPIOClose(hGpio);
}

//------------------------------------------------------------------------------
//
//  Function:  display_lcd_image
//
//  This function displays the image in the frame buffer on the LCD
//
void display_lcd_image( void )
{
    UINT8  count, timeout = DELAY_COUNT  ;
    UINT16 ctrl;
    OMAP_DISPC_REGS *pDisplayRegs = OALPAtoUA(OMAP_DISC1_REGS_PA);
    
    // Apply display configuration
    SETREG32(&pDisplayRegs->DISPC_CONTROL,DISPC_CONTROL_GOLCD);
    
    // wait for configuration to take effect
    do
    {
        for(count=0;count<DELAY_COUNT;++count);
        ctrl=INREG16(&pDisplayRegs->DISPC_CONTROL);
        timeout-- ;
    }
    while((ctrl & DISPC_CONTROL_GOLCD) && (timeout > 0));
    
    // Power up and start scanning
    LcdPdd_SetPowerLevel(D0);   
    
     //OALMSG(OAL_INFO, (L"display_lcd_image-\r\n"));
}


void LcdStall(DWORD dwMicroseconds)
{
    OALStall(dwMicroseconds);
}

void LcdSleep(DWORD dwMilliseconds)
{
    OALStall(1000 * dwMilliseconds);
}
//------------------------------------------------------------------------------
//LCD_SPI_Init function starting, Ray 13-08-06.
/*BOOL OSPIOpen(void)
{
	
	// enable clock for used IO pins
	InitSpiClock();

	// enable SPI1 peripheral clock
	InitSpiPeriphCloc();
	
	return TRUE;
}*/

/*void OSPIConfigure(void)
{
	GPIO_InitTypeDef	GPIO_InitConfig;
	SPI_InitTypeDef		SPI_InitConfig;
	HANDLE hGpio = GPIOOpen();

	GPIO_InitConfig.GPIO_Pin = 
}*/

//------------------------------------------------------------------------------
//lcm_config function starting, Ray 13-08-06.
void LCD_SPI_Init(void)
{
	//OSPIOpen();
	//OSPIConfigure();
	//OSPIClose();
	SPIOpen();
	SPIConfigure();
	SPIClose();
}


void lcm_config(void)
{
	LCD_SPI_Init();
	SPIWrite();
	R61526_send_command();
	R61526_send_data();
}


//------------------------------------------------------------------------------
//
// end of bsp_logo.c
