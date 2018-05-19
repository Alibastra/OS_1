#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#define BUF_SIZE 1024
#define BUF_S 256

void Archiving(char *dir, char *zip)
{
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
	int f;
	int nread;
	int in;
	char buf[BUF_SIZE];
	size_t l;
	off_t l1;

	dp = opendir(dir);
	if (dp == NULL) {
		printf("Error opening directory: %s\n", dir);
		exit(EXIT_FAILURE);
	}

	f = open(zip, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR)
	if (f == -1) {
		printf("Error opening: %s \n", zip);
		exit(EXIT_FAILURE);
	}
	chdir(dir);
	while ((entry = readdir(dp)) != NULL) {
		if (!(lstat(entry->d_name, &statbuf) == 0)) {
			printf("Error\n");
			exit(EXIT_FAILURE);
		}
		if (S_ISDIR(statbuf.st_mode)) {
			if (strcmp(".", entry->d_name) == 0 ||
				strcmp("..", entry->d_name) == 0)
				continue;
			l = strlen(entry->d_name);
			if (write(f, "/", 1) == -1) {
				printf("Error writing 1\n");
				exit(EXIT_FAILURE);
			}
			if (write(f, &l, sizeof(l)) == -1) {
				printf("Error writing 2\n");
				exit(EXIT_FAILURE);
			}
			if (write(f, &(entry->d_name), l) == -1) {
				printf("Error writing 3\n");
				exit(EXIT_FAILURE);
			}
		} else
			if ((strcmp(zip, entry->d_name)) != 0) {
				l = strlen(entry->d_name);
				if (write(f, "*", 1) == -1) {
					printf("Error writing\n");
					exit(EXIT_FAILURE);
				}
				if (write(f, &l, sizeof(l)) == -1) {
					printf("Error writing\n");
					exit(EXIT_FAILURE);
				}
				if (write(f, entry->d_name, l) == -1) {
					printf("Error writing\n");
					exit(EXIT_FAILURE);
				}
				if (write(f, &statbuf.st_size,
					sizeof(statbuf.st_size)) == -1) {
					printf("Error writing\n");
					exit(EXIT_FAILURE);
				}
				in = open(entry->d_name, O_RDONLY);
				if (in < 0) {
					printf("Error open in \n");
					exit(EXIT_FAILURE);
				}
				l1 = statbuf.st_size;
				while (l1 > BUF_SIZE) {
					nread = read(in, buf, BUF_SIZE);
					if (nread == -1) {
						printf("Error reading\n");
						exit(EXIT_FAILURE);
					}
					l1 = l1-BUF_SIZE;
					if (write(f, buf, BUF_SIZE) == -1) {
						printf("Error writing\n");
						exit(EXIT_FAILURE);
					}
				}
				if (read(in, buf, l1) == -1) {
					printf("Error reading\n");
					exit(EXIT_FAILURE);
				}
				if (write(f, buf, l1) == -1) {
					printf("Error writing\n");
					exit(EXIT_FAILURE);
				}
			}
	}
	printf("Completed\n");
	if (closedir(dp) == 0)
		exit(EXIT_FAILURE);
	if (close(f) == 0)
		exit(EXIT_FAILURE);
}

void clearbuf(char *buf, size_t l)
{
	for (int i = 0; i < l; i++)
		buf[i] = '\0';
}

void Decompression(char *zip,  char *dir)
{
	char *buf;
	char buf1[BUF_S] = {0}, b1, buf2[BUF_SIZE];
	int f, out;
	int nread, j = 1;
	size_t l;
	off_t l1;
	int b = 0;


	f = open(zip, O_RDONLY);
	if (f < 0) {
		printf("Error opening file %s\n", zip);
		exit(EXIT_FAILURE);
	}
	if (chdir(dir) == -1) {
		if (mkdir(dir, 0775) == -1) {
			printf("Error reading\n");
			exit(EXIT_FAILURE);
		}
		chdir(dir);
	}
	while (read(f, &b1, 1)) {
		switch (b1) {
		case '/': {
			if (read(f, &l, sizeof(l)) == -1) {
				printf("Error reading l\n");
				exit(EXIT_FAILURE);
			}
			if (read(f, buf1, l) == -1) {
				printf("Error reading l1\n");
				exit(EXIT_FAILURE);
			}
			mkdir(buf1, 0775);
			clearbuf(buf1, l);
			break;
		}
		case '*': {
			if (read(f, &l, sizeof(l)) == -1) {
				printf("Error reading l\n");
				exit(EXIT_FAILURE);
			}
			if (read(f, buf1, l) == -1) {
				printf("Error reading name\n");
				exit(EXIT_FAILURE);
			}
			out = open(buf1, O_WRONLY | O_CREAT, 0755);
			if (out < 0) {
				printf("Error create file\n");
				exit(EXIT_FAILURE);
			}
			clearbuf(buf1, l);
			if (read(f, &l1, sizeof(l1)) == -1) {
				printf("Error reading Nbytes\n");
				exit(EXIT_FAILURE);
			}
			while (l1 > BUF_SIZE) {
				if (read(f, buf2, BUF_SIZE) == -1) {
					printf("Error read file\n");
					exit(EXIT_FAILURE);
				}
				l1 = l1-BUF_SIZE;
				if (write(out, buf2, BUF_SIZE) == -1) {
					write(1, &errno, sizeof(errno));
					printf("Error writing buf\n");
					exit(EXIT_FAILURE);
				}
			}
			if (read(f, buf2, l1) == -1) {
				printf("Error read file\n");
				exit(EXIT_FAILURE);
			}
			if (write(out, buf2, l1) == -1) {
				printf("Error writing\n");
				exit(EXIT_FAILURE);
			}
			break;
		}
		case '\0': {
			printf("No %s file\n", zip);
			exit(EXIT_FAILURE);
		}
		default: {
			printf("%c", b1);
			printf("Error reading b1\n");
			exit(EXIT_FAILURE);
			break;
		}
		}
	}
	printf("Completed\n");
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("Error in the number of arguments\n");
		exit(EXIT_FAILURE);
	}
	char i;
	printf("Press the key:\n 1 - Archiving\n 2 - Decompression\n Any key - Exit\n");
	if (read(0, &i, 1) < 1) {
		printf("Error reading key\n");
		exit(EXIT_FAILURE);
	}

	switch (i) {
	case '1':
	{
		Archiving(argv[1], argv[2]);
		exit(EXIT_SUCCESS);
	}
	case '2':
	{
		Decompression(argv[1], argv[2]);
		exit(EXIT_SUCCESS);
	}
	default:
	{
//		printf("Error while typing the third argument\n'1'-Archiving\n'2'-Decompression\n");
		printf("program break\n");
		exit(EXIT_FAILURE);
	}
	}
}

