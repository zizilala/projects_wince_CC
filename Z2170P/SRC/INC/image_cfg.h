// All rights reserved ADENEO EMBEDDED 2010
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
//  File:  image_cfg.h
//
// This file is also included in the config.bib. It's thereof very important that 
// the preprocessor statements are limited to those understood by romimage.exe
// For example don't use #ifndef  #endif but use #ifdef #else #endif, and keep the #define on 1 line only


//------------------------------------------------------------------------------
//  EVM board 128 MB of SDRAM located physically at 0x80000000
//
#define DEVICE_RAM_PA                   0x80000000
#define DEVICE_RAM_CA                   0x80000000

#if (defined BSP_SDRAM_BANK1_ENABLE)
#define DEVICE_RAM_SIZE                 0x10000000
#else
#define DEVICE_RAM_SIZE                 0x08000000
#endif

//------------------------------------------------------------------------------
//
//  Define: IMAGE_SHARE_ARGS
//
//  Following constants define location and maximal size of arguments shared
//  between loader and kernel. For actual structure see args.h file.
//  
#define IMAGE_SHARE_ARGS_CA             0x80000000
#define IMAGE_SHARE_ARGS_SIZE          0x00001000
//------------------------------------------------------------------------------
//
//  Define: CPU_INFO_ADDR
//
//  Following constants define location and maximal size of arguments shared
//  between loader and kernel. For actual structure see args.h file.
//  
#define CPU_INFO_ADDR_PA                0x80001000
#define CPU_INFO_ADDR_CA                0x80001000
#define CPU_INFO_ADDR_SIZE              0x00001000
//------------------------------------------------------------------------------
//
//  Define: IMAGE_WINCE_CODE
//
//  Following constants define Windows CE OS image layout.
//
#if (defined IMGMULTIXIP)
#define IMAGE_WINCE_CHAIN_CA			0x80002000   // 
#define IMAGE_WINCE_CHAIN_SIZE		0x00001000	// CHAIN is used for IMAGE Extension  

#define IMAGE_WINCE_CODE_CA			0x80003000   // 
#define IMAGE_WINCE_CODE_SIZE			0x01FFE000   // CODE      40 MB //PRG666 was 0x02800000

#define IMAGE_WINCE_EXT_CA			0x82001000  // 
#define IMAGE_WINCE_EXT_SIZE			0x027FF000  // 
#define IMAGE_WINCE_RAM_CA			0x82001000  // PRG666 Same as IMAGE_WINCE_EXT_CA  
#define IMAGE_WINCE_RAM_SIZE			0x027FF000  // PRG666 RAM 32 MB  was 0x1FFE000
#else
#define IMAGE_WINCE_CODE_CA			0x80002000   // 
#define IMAGE_WINCE_CODE_SIZE			0x02800000   // CODE      40 MB 
#define IMAGE_WINCE_RAM_CA			0x82802000  // PRG666 Same as IMAGE_WINCE_EXT_CA  
#define IMAGE_WINCE_RAM_SIZE			0x01FFE000  // PRG666 RAM 32 MB  was 0x1FFE000
#endif

#define IMAGE_CMEM_CA			0x84800000	//CMEM		; this is always needed if DVSDK is enabled
#define IMAGE_CMEM_SIZE			0x01000000	//16 MB

#define IMAGE_DSP_720P_CA		0x85800000	//DSP (2)
#define IMAGE_DSP_720P_SIZE		0x01A00000	//26MB		; extra DSP region if using 720p codecs and SDRAM bank1

#define IMAGE_DSP_CA				0x87200000	//DSP		
#define IMAGE_DSP_SIZE				0x00E00000	// 14MB		; this is always used if DVSDK is enabled


#if (defined BSP_SDRAM_BANK1_ENABLE )
#define IMAGE_WINCE_DISPLAY_CA		0x88000000   // VIDEO    
#define IMAGE_WINCE_DISPLAY_SIZE	0x01A00000   // 26 MB	; for 256MB config, this will be used
#define IMAGE_WINCE_RAM_BANK1_CA	0x89A00000   // 
#define IMAGE_WINCE_RAM_BANK1_SIZE	0x06600000   // RAM (2nd BANK)  102 MB
#else
#define IMAGE_WINCE_DISPLAY_CA		0x85800000   // VIDEO    
#define IMAGE_WINCE_DISPLAY_SIZE	0x01A00000   // 26 MB	; for 128MB config, this will be used and DSP 2 region will be unavailable
#endif

#define IMAGE_WINCE_NOR_OFFSET		0x00060000
#define NAND_ROMOFFSET				0x40000000
#define NOR_ROMOFFSET				0x60000000


//------------------------------------------------------------------------------
//
//  Define: IMAGE_XLDR_xxx
//
//  Following constants define image layout for X-Loader. used to download/program the XLDR
//  XLDR executes from SRAM. (MMU is not used in XLDR, so the address are physical here)
// NOTE : This is for flash XLDR. (XLDR SD is another matter)
#define IMAGE_XLDR_CODE_PA              0x40200000
#define IMAGE_XLDR_CODE_SIZE            0x0000C000

#define IMAGE_XLDR_NOR_OFFSET           (0x00000000)

//------------------------------------------------------------------------------
//
//  Define: IMAGE_EBOOT_xxx
//
//  Following constants define EBOOT image layout. 
//
#define IMAGE_EBOOT_CODE_CA             0x87E00000
#define IMAGE_EBOOT_CODE_SIZE           0x00040000

#define IMAGE_EBOOT_DATA_CA             0x87E80000
#define IMAGE_EBOOT_DATA_SIZE           0x00050000

#define IMAGE_EBOOT_NOR_OFFSET          0x00020000
#define IMAGE_EBOOT_CFG_NOR_OFFSET      0x00008000

//------------------------------------------------------------------------------
//
//  Define: IMAGE_STARTUP_xxx
//
//  Jump address XLDR uses to bring-up the device.
//
#define IMAGE_STARTUP_IMAGE_PA         (IMAGE_EBOOT_CODE_CA - DEVICE_RAM_CA + DEVICE_RAM_PA)
#define IMAGE_STARTUP_IMAGE_CA         (IMAGE_EBOOT_CODE_CA)
#define IMAGE_STARTUP_IMAGE_SIZE       (IMAGE_EBOOT_CODE_SIZE)


//------------------------------------------------------------------------------
//
//  Define: IMAGE_BOOTLOADER_xxx
//
//  Following constants define bootloader information
//

#define IMAGE_XLDR_BOOTSEC_NAND_SIZE		(4 * 128 * 1024)        // Needs to be equal to four NAND flash blocks due to boot ROM requirements
#define IMAGE_EBOOT_BOOTSEC_NAND_SIZE		IMAGE_EBOOT_CODE_SIZE   // Needs to be a multiple of flash block size

#define IMAGE_BOOTLOADER_BITMAP_SIZE		0x00100000                  // Needs to be a multiple of 128k, and minimum 480x640x3 (VGA)  

#define IMAGE_BOOTLOADER_NAND_SIZE			(IMAGE_XLDR_BOOTSEC_NAND_SIZE + IMAGE_EBOOT_BOOTSEC_NAND_SIZE + IMAGE_BOOTLOADER_BITMAP_SIZE)

//------------------------------------------------------------------------------

