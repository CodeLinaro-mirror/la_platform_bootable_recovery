/*==============================================================================
  FILE:         version.h

  OVERVIEW:     Declares methods for skipping installation of components during 
                OTA update

  DEPENDENCIES: - None

  Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
  SPDX-License-Identifier: BSD-3-Clause-Clear

==============================================================================*/

#ifndef _UPDATER_VERSION_H_
#define _UPDATER_VERSION_H_

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <unistd.h>


#define STRING_ISCHAR(c) \
    ((c) >= 32 && (c) <= 126)

#define STRING_ISASCII(c) \
    (((c) >= 0 && (c) <= 127) \
    && (c != 0)  \
    )

#define IMAGE_TABLE_START_ADDR            0x87E82470
#define IMAGE_TABLE_END_ADDR              0x87E83470

#define IMAGE_INDEX_LENGTH                2
#define IMAGE_SEP1_LENGTH                 1

#define IMAGE_VERSION_STRING_POS          (IMAGE_INDEX_LENGTH + \
                                            IMAGE_SEP1_LENGTH) // 3

#define IMAGE_VERSION_SINGLE_BLOCK_SIZE   128
#define MAX_STR_SIZE                      72
#define IMAGE_VERSION_SIZE                4096

#define ABOOT_SEARCH_STR                  "09:"
#define ABOOT_SEARCH_STR_LEN              4
#define VERSION_SEARCH_STR                "QC_IMAGE_VERSION_STRING="
#define VERSION_SEARCH_STR_LEN            25

enum IMAGE_TABLE_POS
{
  SBL_POS = 0,
  TZ_POS = 1,
  RPM_POS = 3,
  ABOOT_POS = 9
};

int smem_search();
static int blob_search(char *data_ptr, ssize_t file_size, 
                                        char *keyword_str, char *version_str);
int install_checker(char *partition, char *data_ptr, ssize_t file_size);

#endif /* _UPDATER_VERSION_H_ */
