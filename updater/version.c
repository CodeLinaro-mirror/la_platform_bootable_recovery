/*==============================================================================
  FILE:         version.c

  OVERVIEW:     Provides methods for skipping installation of components during
                OTA update

  DEPENDENCIES: - None

  Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
  SPDX-License-Identifier: BSD-3-Clause-Clear

==============================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include <unistd.h>
#include "version.h"

static char smem_version_table[4][MAX_STR_SIZE + 1];
static bool smem_search_flag = false;

/**
 * @name    install_checker
 * @brief   Verify if component needs installation
 * @param[in] -
 *          partition: String containing name of partition being updated
 *          data_ptr : Pointer to install image
 *          file_size: Size of install image  
 * @retval  return 0 on success, -1 otherwise. success is no installation required
 */
int install_checker(char *partition, char *data_ptr, ssize_t file_size)
{
  unsigned int i, j;

  char search_string[VERSION_SEARCH_STR_LEN];
  char image_version[MAX_STR_SIZE + 2];
  char smem_version[MAX_STR_SIZE + 1];

  if (!smem_search_flag)
  {
    fprintf(stderr, "Version table not updated\n");
    return -1;
  }

  if (!strcmp(partition, "sbl"))
  {
    strlcpy(search_string, VERSION_SEARCH_STR, VERSION_SEARCH_STR_LEN);
    strlcpy(smem_version, smem_version_table[0], MAX_STR_SIZE+1);
  }
  else if (!strcmp(partition, "tz"))
  {
    strlcpy(search_string, VERSION_SEARCH_STR, VERSION_SEARCH_STR_LEN);
    strlcpy(smem_version, smem_version_table[1], MAX_STR_SIZE+1);
  }
  else if (!strcmp(partition, "rpm"))
  {
    strlcpy(search_string, VERSION_SEARCH_STR, VERSION_SEARCH_STR_LEN);
    strlcpy(smem_version, smem_version_table[2], MAX_STR_SIZE+1);
  }
  else if (!strcmp(partition, "aboot"))
  {
    strlcpy(search_string, ABOOT_SEARCH_STR, ABOOT_SEARCH_STR_LEN);
    strlcpy(smem_version, smem_version_table[3], MAX_STR_SIZE+1);
  }
  else
  {
    fprintf(stdout, "%s: Is always installed\n", partition);
    return -1;
  }

  fprintf(stdout, "%s: Currently Installed Version : %s\n", partition, smem_version);
  // Get Version from binary
  if (blob_search(data_ptr, file_size, search_string, image_version) != 0)
  {
    fprintf(stderr, "Reading version from image failed\n");
    return -1;
  }

  fprintf(stdout, "%s: Version is %s\n", partition, image_version);

  if (strcmp(image_version, smem_version))
  {
    fprintf(stderr, "%s: installed version doesn't match image\n", partition);
    return -1;
  }

  fprintf(stdout, "%s: matches installed version, skipping install\n", partition);
  return 0;
}

/**
 * @name    smem_search
 * @brief   Fetch installed image versions from smem
 * @retval  return 0 on success, -1 otherwise
 */
int smem_search()
{
  int fd;
  char(*image_table)[IMAGE_VERSION_SINGLE_BLOCK_SIZE];
  off_t pa_offset, table_offset;
  char *page_addr;

  fprintf(stdout, "Fetching versions from smem\n");

  fd = open("/dev/mem", O_RDWR | O_SYNC);

  if (fd == -1)
  {
    fprintf(stderr, "Opening /dev/mem failed\n");
    return -1;
  }

  pa_offset = IMAGE_TABLE_START_ADDR & ~(sysconf(_SC_PAGE_SIZE) - 1); /* offset for mmap() must be page aligned */

  table_offset = IMAGE_TABLE_START_ADDR - pa_offset;

  page_addr = (char *)mmap(NULL, IMAGE_VERSION_SIZE + table_offset, PROT_READ, MAP_SHARED, fd, pa_offset);

  if (page_addr == MAP_FAILED)
  {
    fprintf(stderr, "Mapping Failed\n");
    fprintf(stderr, "ERRNO: %s\n", strerror(errno));
    return -1;
  }

  // Address at which smem image table is present
  image_table = page_addr + table_offset;
  unsigned int row_index, col_index;
  unsigned int partition_index_table[4] = {SBL_POS, TZ_POS, RPM_POS, ABOOT_POS};

  /* Extract version number from smem table till reaching MAX_STR_LENGTH
     or seeing a '\n' or '0'.
  */
  for (row_index = 0; row_index < 4; row_index++)
  {
    for (col_index = IMAGE_VERSION_STRING_POS;
         col_index < IMAGE_VERSION_STRING_POS + MAX_STR_SIZE; col_index++)
    {
      if (image_table[partition_index_table[row_index]][col_index] == '\n' || image_table[partition_index_table[row_index]][col_index] == 0)
      {
        break;
      }
      smem_version_table[row_index][col_index - IMAGE_VERSION_STRING_POS] =
          image_table[partition_index_table[row_index]][col_index];
    }
    smem_version_table[row_index][col_index - IMAGE_VERSION_STRING_POS] = '\0';
  }
  smem_search_flag = true;

  int err = munmap(page_addr, IMAGE_VERSION_SIZE + table_offset);

  if (err != 0)
  {
    fprintf(stderr, "Unmapping Failed: %s\n", strerror(errno));
  }

  fprintf(stdout, "Fetching from smem complete\n");
  return 0;
}

/**
 * @name    blob_search
 * @brief   Fetch image versions from image binaries
 * @param[in] -
 *          data_ptr : Pointer to binary being searched
 *          file_size: Size of binary being serached
 *          keyword_str: Keyword preceding version string 
 * @param[out] -
 *          version_str: pointer to store version string
 * @retval  return 0 on success, FAILURE CODE otherwise
 */
static int blob_search(char *data_ptr, ssize_t file_size,
                       char *keyword_str, char *version_str)
{
  if (data_ptr == NULL || file_size == 0)
  {
    fprintf(stderr, "Invalid binary\n");
    return -1;
  }
  fprintf(stdout, "Searching for %s\n", keyword_str);

  unsigned int i = 0;
  long c;
  int keyword_len = strlen(keyword_str);

  // Loop to search if keyword string is present in binary
  while (file_size-- > 0)
  {

    c = *data_ptr++;
    if (STRING_ISCHAR(c)) // Ignore special characters
    {
      if (i == keyword_len)
      {
        fprintf(stdout, "Version found\n");
        break;
      }
      else if (!(keyword_str[i++] == (char)c))
      {
        i = 0;
      }
    }
  }

  // If keyword string is not present return
  if (i != keyword_len)
  {
    fprintf(stderr, "Version Not found\n");
    version_str[i] = '\0';
    return -1;
  }

  // Decrement pointer to read version from correct location
  data_ptr--;
  ++file_size;
  i = 0;

  /* It is possible that version string is longer than MAX_STR_SIZE. Read 1 byte
  more than what is permisible in SMEM to account for this.*/
  while (i < MAX_STR_SIZE + 1 && file_size-- > 0)
  {
    c = *data_ptr++;

    if (STRING_ISCHAR(c))
    {
      version_str[i++] = (char)c;
    }
    else if (!STRING_ISASCII(c))
      break;
  }
  version_str[i] = '\0';

  if (file_size == 0)
  {
    fprintf(stderr, "Version Not found\n");
    return -1;
  }

  return 0;
}
