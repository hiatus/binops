#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#if defined(BINOPS_OR)
#	define MAIN_OPER "or"
#elif defined(BINOPS_AND)
#	define MAIN_OPER "and"
#elif defined(BINOPS_XOR)
#	define MAIN_OPER "xor"
#else
#	error No binary operation specified
#endif

#ifndef EXIT_SUCCESS
#	define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
#	define EXIT_FAILURE 1
#endif

#define MAX_BUFF 32768
#define MAX_MASK  4294967296

const char banner[] =
MAIN_OPER " [options] [file]?\n"
"	Iterate a byte mask against [file]? in a bitwise " MAIN_OPER "\n\n"

"	-h         help\n"
"	-f [file]  use [file] as mask\n"
"	-x [hex]   interpret and use [hex] as mask\n";

static int _hex2raw(const char *hex, uint8_t *data);
static uint8_t *_load_file(const char *path, size_t *size);

int main(int argc, char **argv)
{
	int opt;
	int ret = 0;

	size_t len;
	size_t len_mask;

	uint8_t *mask = NULL;
	uint8_t buff[MAX_BUFF];

	FILE *in = stdin;

	if (argc == 1) {
		fputs(banner, stderr);
		return EXIT_FAILURE;
	}

	while ((opt = getopt(argc, argv, ":hf:x:")) != -1) {
		switch (opt) {
			case 'h':
				ret = EXIT_SUCCESS;
				fputs(banner, stderr);

				goto cleanup;

			case 'f':
				mask = _load_file(optarg, &len_mask);

                                if (! mask || ! len_mask) {
					ret = EXIT_FAILURE;

					if (! len_mask)
						fputs("[err] Empty mask file\n", stderr);

                                        goto cleanup;
                                }

                                break;

			case 'x':
				if (! (len_mask = strlen(optarg) / 2)) {
					ret = EXIT_FAILURE;
					fputs("[err] Zero-length mask\n", stderr);

					goto cleanup;
				}

				if (! (mask = malloc(len_mask + 1))) {
					ret = EXIT_FAILURE;
					perror("malloc");

					goto cleanup;
				}

				if (_hex2raw(optarg, mask)) {
					ret = EXIT_FAILURE;

					fprintf(
						stderr,
						"[err] Invalid hex string: '%s'\n", optarg
					);

                                        goto cleanup;
                                }

				break;

			case ':':
				ret = EXIT_FAILURE;

				fprintf(
					stderr,
					"[err] Option '%c' requires an argument\n", optopt
				);

				goto cleanup;;

			case '?':
				ret = EXIT_FAILURE;
				fprintf(stderr, "[err] Invalid option: '%c'\n", optopt);

				goto cleanup;
		}
	}

	if (! mask || ! len_mask) {
		ret = EXIT_FAILURE;
		fputs("[err] No mask provided\n", stderr);

		goto cleanup;
	}

	if (optind <= argc - 1 && ! (in = fopen(argv[optind], "rb"))) {
		ret = EXIT_FAILURE;
		fprintf(stderr, "[err] Failed to open '%s", argv[optind]);
		perror("'");

		goto cleanup;
	}

	for (size_t off = 0; (len = fread(buff, 1, MAX_BUFF, in)) > 0; off += len) {
		for (size_t i = 0; i < len; ++i)
#if defined(BINOPS_OR)
			buff[i] |= mask[(off + i) % len_mask];
#elif defined(BINOPS_AND)
			buff[i] &= mask[(off + i) % len_mask];
#elif defined(BINOPS_XOR)
			buff[i] ^= mask[(off + i) % len_mask];
#endif

		fwrite(buff, 1, len, stdout);
		off += len;
	}

cleanup:
	if (mask)
		free(mask);

	if (in && in != stdin)
		fclose(in);

	return ret;
}

static int _hex2raw(const char *hex, uint8_t *data)
{
	char byte[3] = {0x00, 0x00, 0x00};

	for (size_t hi = 0, di = 0; hex[hi]; hi += 2, ++di) {
		if (! isxdigit(hex[hi]))
			return 1;

		if (hex[hi + 1]) {
			if (! isxdigit(hex[hi + 1]))
				return 1;

			byte[0] = hex[hi];
			byte[1] = hex[hi + 1];
		}
		else {
			byte[0] = '0';
			byte[1] = hex[hi];
		}

		data[di] = strtoul(byte, NULL, 16);
	}

	return 0;
}

static uint8_t *_load_file(const char *path, size_t *size)
{
	FILE *fp;
	uint8_t *content;

	if (! (fp = fopen(path, "rb"))) {
		perror("[err] Failed to open mask file");
		return NULL;
	}

	fseek(fp, 0, SEEK_END);

	if ((*size = ftell(fp)) > MAX_MASK) {
		fprintf(
			stderr,
			"[err] Mask file too large (over %zu bytes)\n",
			MAX_MASK
		);

		fclose(fp);
		return NULL;
	}

	if (! (content = malloc(*size))) {
		perror("[err] malloc");

		fclose(fp);
		return NULL;
	}

	rewind(fp);

	if (fread(content, 1, *size, fp) != *size) {
		fprintf(
			stderr,
			"[err] Failed to read %zu bytes from mask file\n", *size
		);

		fclose(fp);
		free(content);

		return NULL;
	}

	fclose(fp);

	return content;
}
