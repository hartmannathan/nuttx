/****************************************************************************
 * boards/arm/gd32f4/gd32f450zk-aiotbox/src/gd32f4xx_bringup.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdbool.h>
#include <stdio.h>
#include <debug.h>
#include <errno.h>

#include <nuttx/board.h>
#include <nuttx/clock.h>
#include <nuttx/kmalloc.h>
#include <nuttx/mtd/mtd.h>
#include <nuttx/fs/fs.h>
#include <nuttx/fs/nxffs.h>
#include <nuttx/spi/spi_transfer.h>
#include <nuttx/rc/dummy.h>

#include "gd32f4xx.h"

#ifdef CONFIG_USERLED
#  include <nuttx/leds/userled.h>
#endif

#ifdef CONFIG_INPUT_BUTTONS
#  include <nuttx/input/buttons.h>
#endif

#ifdef CONFIG_GD32F4_ROMFS
#include "gd32f4xx_romfs.h"
#endif

#include "gd32f450z_aiotbox.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: gd32_bringup
 *
 * Description:
 *   Perform architecture-specific initialization
 *
 *   CONFIG_BOARD_LATE_INITIALIZE=y :
 *     Called from board_late_initialize().
 *
 *   CONFIG_BOARD_LATE_INITIALIZE=n && CONFIG_BOARDCTL=y :
 *     Called from the NSH library via boardctl()
 *
 ****************************************************************************/

int gd32_bringup(void)
{
#ifdef CONFIG_RAMMTD
  uint8_t *ramstart;
#endif

  int ret = OK;

#ifdef CONFIG_FS_PROCFS
  /* Mount the procfs file system */

  ret = nx_mount(NULL, GD32_PROCFS_MOUNTPOINT, "procfs", 0, NULL);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: Failed to mount procfs at %s: %d\n",
             GD32_PROCFS_MOUNTPOINT, ret);
    }
#endif

#ifdef CONFIG_RAMMTD
  /* Create a RAM MTD device if configured */

  ramstart = kmm_malloc(64 * 1024);
  if (ramstart == NULL)
    {
      syslog(LOG_ERR, "ERROR: Allocation for RAM MTD failed\n");
    }
  else
    {
      /* Initialized the RAM MTD */

      struct mtd_dev_s *mtd = rammtd_initialize(ramstart, 64 * 1024);
      if (mtd == NULL)
        {
          syslog(LOG_ERR, "ERROR: rammtd_initialize failed\n");
          kmm_free(ramstart);
        }
      else
        {
          /* Erase the RAM MTD */

          ret = mtd->ioctl(mtd, MTDIOC_BULKERASE, 0);
          if (ret < 0)
            {
              syslog(LOG_ERR, "ERROR: IOCTL MTDIOC_BULKERASE failed\n");
            }

#if defined(CONFIG_MTD_SMART) && defined(CONFIG_FS_SMARTFS)
          /* Initialize a SMART Flash block device and bind it to the MTD
           * device.
           */

          smart_initialize(0, mtd, NULL);

#elif defined(CONFIG_FS_SPIFFS)
          /* Register the MTD driver so that it can be accessed from the
           * VFS.
           */

          ret = register_mtddriver("/dev/rammtd", mtd, 0755, NULL);
          if (ret < 0)
            {
              syslog(LOG_ERR, "ERROR: Failed to register MTD driver: %d\n",
                     ret);
            }

          /* Mount the SPIFFS file system */

          ret = nx_mount("/dev/rammtd", "/mnt/spiffs", "spiffs", 0, NULL);
          if (ret < 0)
            {
              syslog(LOG_ERR,
                     "ERROR: Failed to mount SPIFFS at /mnt/spiffs: %d\n",
                     ret);
            }

#elif defined(CONFIG_FS_LITTLEFS)
          /* Register the MTD driver so that it can be accessed from the
           * VFS.
           */

          ret = register_mtddriver("/dev/rammtd", mtd, 0755, NULL);
          if (ret < 0)
            {
              syslog(LOG_ERR, "ERROR: Failed to register MTD driver: %d\n",
                     ret);
            }

          /* Mount the LittleFS file system */

          ret = nx_mount("/dev/rammtd", "/mnt/lfs", "littlefs", 0,
                         "forceformat");
          if (ret < 0)
            {
              syslog(LOG_ERR,
                     "ERROR: Failed to mount LittleFS at /mnt/lfs: %d\n",
                     ret);
            }

#elif defined(CONFIG_FS_NXFFS)
          /* Initialize to provide NXFFS on the MTD interface */

          ret = nxffs_initialize(mtd);
          if (ret < 0)
            {
              syslog(LOG_ERR, "ERROR: NXFFS initialization failed: %d\n",
                     ret);
            }
#endif
        }
    }
#endif

#if defined(CONFIG_INPUT_BUTTONS_LOWER)
  ret = btn_lower_initialize("/dev/buttons");
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: btn_lower_initialize() failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_GD32F4_ROMFS
      /* Mount the romfs partition */

      ret = gd32_romfs_initialize();
      if (ret < 0)
        {
          syslog(LOG_ERR, "ERROR: Failed to mount romfs at %s: %d\n",
                CONFIG_GD32F4_ROMFS_MOUNTPOINT, ret);
        }
#endif

#ifndef CONFIG_DISABLE_MOUNTPOINT

#  ifdef CONFIG_GD32F4_PROGMEM

      /* Create an instance of the GD32F4 FLASH program memory
       * device driver
       */

      struct mtd_dev_s *mtd = progmem_initialize();
      if (!mtd)
        {
          syslog(LOG_ERR, "ERROR: progmem_initialize failed\n");
        }

#    if defined(CONFIG_FS_NXFFS)
      /* Initialize to provide NXFFS on the MTD interface */

      ret = nxffs_initialize(mtd);
      if (ret < 0)
        {
          syslog(LOG_ERR, "ERROR: NXFFS initialization failed: %d\n",
                  ret);
        }

      /* Mount the file system */

      ret = nx_mount(NULL, CONFIG_GD32F4_NXFFS_MOUNTPT, "nxffs", 0, NULL);
      if (ret < 0)
        {
          syslog(LOG_ERR, "ERROR: Failed to mount the NXFFS volume: %d\n",
                  ret);
        }
#      elif defined(CONFIG_FS_LITTLEFS)
      /* Initialize to provide LittleFS on the MTD interface */

      ret = register_mtddriver("/dev/fmc", mtd, 0755, NULL);
      if (ret < 0)
        {
          syslog(LOG_ERR, "ERROR: Failed to register MTD: %d\n", ret);
          return ret;
        }

      /* Mount the file system at /mnt/fmc */

      ret = nx_mount("/dev/fmc", "/mnt/fmc", "littlefs", 0, NULL);
      if (ret < 0)
        {
          ret = nx_mount("/dev/fmc", "/mnt/fmc", "littlefs", 0,
                         "forceformat");
          if (ret < 0)
            {
              ferr("ERROR: Failed to mount the FS volume: %d\n", ret);
              return ret;
            }
        }

      syslog(LOG_INFO, "INFO: LittleFS volume /mnt/fmc mount " \
            "on chip flash success: %d\n", ret);
#    endif
#  endif

#  ifdef HAVE_GD25

      ret = gd32_gd25_automount(0);
      if (ret < 0)
        {
          syslog(LOG_ERR, "ERROR: Failed to mount the NXFFS \
                 volume on spi flash: %d\n", ret);
        }

#  endif

#  ifdef HAVE_AT24

     ret = gd32_at24_wr_test(AT24_MINOR);
      if (ret < 0)
        {
          syslog(LOG_ERR, "ERROR: I2C EEPROM write and read test fail: \
                 %d\n", ret);
        }

#  endif

#endif /* CONFIG_FS_NXFFS */

#ifdef CONFIG_DEV_GPIO
      /* Register the GPIO driver */

      ret = gd32_gpio_initialize();
      if (ret < 0)
        {
          syslog(LOG_ERR, "Failed to initialize GPIO Driver: %d\n", ret);
          return ret;
        }
#endif

#ifdef CONFIG_I2C
  gd32_i2c_initialize();
#endif

#ifdef CONFIG_INPUT_BUTTONS
#ifdef CONFIG_INPUT_BUTTONS_LOWER
  /* Register the BUTTON driver */

      ret = btn_lower_initialize("/dev/buttons");
      if (ret != OK)
        {
          syslog(LOG_ERR, "ERROR: btn_lower_initialize() failed: %d\n", ret);
          return ret;
        }
#else
  /* Enable BUTTON support for some other purpose */

  board_button_initialize();
#endif /* CONFIG_INPUT_BUTTONS_LOWER */
#endif /* CONFIG_INPUT_BUTTONS */

#if !defined(CONFIG_ARCH_LEDS) && defined(CONFIG_USERLED_LOWER)
      /* Register the LED driver */

      ret = userled_lower_initialize(LED_DRIVER_PATH);
      if (ret < 0)
        {
          syslog(LOG_ERR, "ERROR: userled_lower_initialize() failed: %d\n",
                 ret);
        }
#endif

  /* Configure SDIO chip selects */

#ifdef CONFIG_ARCH_HAVE_SDIO
      ret = gd32_sdio_initialize();
      if (ret != OK)
        {
          syslog(LOG_ERR, "ERROR: gd32_sdio_initialize() failed: %d\n", ret);
          return ret;
        }

  /* Mount the file system at /mnt/sd */

      ret = nx_mount("/dev/mmcsd0", "/mnt/sd", "vfat", 0, NULL);
      if (ret < 0)
        {
          ret = nx_mount("/dev/mmcsd0", "/mnt/sd", "vfat", 0,
            "forceformat");
          if (ret < 0)
            {
              ferr("ERROR: Failed to mount the SD card: %d\n", ret);
              return ret;
            }
        }

    syslog(LOG_INFO, "INFO: FAT volume /mnt/sd mount " \
        "sd card success: %d\n", ret);
#endif
  return ret;
}
