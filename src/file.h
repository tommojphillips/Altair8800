/* file.h
 * Github: https:\\github.com\tommojphillips
 */

#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <stdint.h>

int read_file_into_buffer(const char* filename, void* buff, const uint32_t buff_size, const uint32_t offset, const uint32_t expected_size);

#endif