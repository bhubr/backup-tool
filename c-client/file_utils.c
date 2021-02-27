#include <stdio.h>
#include <jansson.h>

char *read_file(char *filename)
{
	FILE *fd;
	char *file_content;
	int file_len;

	fd = fopen(filename, "r");

	fseek(fd, 0, SEEK_END);
	file_len = ftell(fd);
	file_content = malloc(file_len + 1);

	fseek(fd, 0, SEEK_SET);
	fread(file_content, 1, file_len, fd);
	fclose(fd);
	file_content[file_len] = '\0';
	return file_content;
}

json_t *read_config_file()
{
	json_t *json;
	json_error_t error;

	json = json_load_file("settings.json", 0, &error);
    if(!json) {
		printf("FATAL: could not read config file: %s\n", error.text);
        exit(1);
    }
	return json;
}
