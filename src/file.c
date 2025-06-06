/* file.c 
 * Github: https:\\github.com\tommojphillips
 */
#include <stdint.h>
#include <stdio.h>

int read_file_into_buffer(const char* filename, void* buff, const uint32_t buff_size, const uint32_t offset, uint32_t* file_size, const uint32_t expected_size) {
	FILE* file = NULL;
	uint32_t size = 0;
	if (filename == NULL)
		return 1;

	fopen_s(&file, filename, "rb");
	if (file == NULL) {
		printf("Error: could not open file: %s\n", filename);
		return 1;
	}

	fseek(file, 0, SEEK_END);
	size = ftell(file);
	if (file_size != NULL) {
		*file_size = size;
	}
	fseek(file, 0, SEEK_SET);

	if (expected_size != 0 && size != expected_size) {
		printf("Error: invalid file size. Expected %u bytes. Got %u bytes\n", expected_size, size);
		fclose(file);
		return 1;
	}

	if (offset + size > buff_size) {
		printf("Error: file is too big for buffer. Offset: %x. File size: %u bytes. Buffer size: %u bytes\n", offset, size, buff_size);
		fclose(file);
		return 1;
	}

	uint32_t bytes_read = (uint32_t)fread((uint8_t*)buff + offset, 1, size, file);
	printf("%X\t-> %s ( %u bytes )\n", offset, filename, bytes_read);
	fclose(file);
	return 0;
}
