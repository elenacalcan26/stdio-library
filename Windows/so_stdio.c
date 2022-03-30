#include <windows.h>

#include "so_stdio.h"

#define MAX_LEN 4096

struct _so_file {
	HANDLE hFile;
	char *buffer; /* buffer din care se scriu/citesc caractere din fisier */
	int buff_pos; /* indica index-ul curent din buffer */
	int currsor; /* indica pozitia curenta din fisier */
	int buff_len; /* lungimea curenta a buffer-ului */
	int feof_flag; /* indica eroarea in urma operatiei cu fisierul */
	int ferror_flag; /* indica finalul fisierului */
	char last_op; /* r = read; w = write */

};

/**
 * @brief Se creeaza structura si se initializeaza campurile ei
 *  cu valori default
 *
 * @return SO_FILE* structura creata, NULL in caz de eroare
 */
SO_FILE *init_so_file(void)
{
	SO_FILE *so_file;

	so_file = (SO_FILE *)malloc(sizeof(SO_FILE));

	if (so_file == NULL)
		return NULL;

	so_file->buffer = (char *)calloc(MAX_LEN, sizeof(char));

	if (so_file->buffer == NULL)
		return NULL;

	so_file->buff_pos = 0;
	so_file->feof_flag = 0;
	so_file->ferror_flag = 0;
	so_file->buff_len = 0;
	so_file->last_op = '\0';
	so_file->currsor = 0;

	return so_file;

}

/**
 * @brief Realizeaza deschiderea unui fisier
 *
 * @param pathname calea catre fisier
 * @param mode modul in care se deschide fisierul
 * @return SO_FILE* structura alocata
 */
SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	SO_FILE *so_file;
	HANDLE hFile;

	/*
	 * se determina modul in care se deschide fisierul
	 */
	if (strcmp(mode, "r") == 0)
		hFile = CreateFile(
				pathname,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ  | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL
				);
	else if (strcmp(mode, "r+") == 0)
		hFile = CreateFile(
				pathname,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL
				);
	else if (strcmp(mode, "w") == 0)
		hFile = CreateFile(
				pathname,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL
				);
	else if (strcmp(mode, "w+") == 0)
		hFile = CreateFile(
				pathname,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL
				);
	else if (strcmp(mode, "a") == 0)
		hFile = CreateFile(
				pathname,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL
				);
	else if (strcmp(mode, "a+") == 0)
		hFile = CreateFile(
				pathname,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL
				);
	else
		hFile = INVALID_HANDLE_VALUE;


	if (hFile == INVALID_HANDLE_VALUE)
		return NULL;

	/* se creeaza structura */
	so_file = init_so_file();

	if (so_file == NULL)
		return NULL;

	so_file->hFile = hFile;

	return so_file;
}

/**
 * @return int intoarce file descriptor-ul fisierului
 */
HANDLE so_fileno(SO_FILE *stream)
{
	return stream->hFile;
}

/**
 * @brief Scrie datele din buffer-ul asociat structurii
 *
 * @return int 0 succes, SO_EOF fail
 */
int so_fflush(SO_FILE *stream)
{
	int r, idx;
	BOOL bRet;
	DWORD bytesWritten;

	/* se verifica ultima operatie asupra fisierului */
	if (stream->last_op == 'w') {

		/* datele sunt scrise din buffer doar cand ultima operatie
		 * a fost una de write
		 */
		idx = 0;
		while (stream->buff_len > 0) {
			bRet = WriteFile(
					stream->hFile,
					stream->buffer + idx,
					stream->buff_len,
					&bytesWritten,
					NULL);

			if (bRet == FALSE) {
				stream->ferror_flag = SO_EOF;
				stream->feof_flag = SO_EOF;
				return SO_EOF;
			}

			stream->buff_len -= bytesWritten;
			idx += bytesWritten;
		}
	}
	memset(stream->buffer, 0, stream->buff_len);
	stream->buff_len = 0;
	stream->buff_pos = 0;
	return 0;
}

/**
 * @brief Muta cursorul in fisier cu un offset dat la o pozitie specificata
 *
 * @return int pozitia curenta din fisier, -1 in caz de eroare
 */
int so_fseek(SO_FILE *stream, long offset, int whence)
{
	int r;
	DWORD dwPos;

	if (stream->last_op == 'w') {
		/* daca inainte s-a realizata o operatie de write
		 * atunci se face flush
		 */

		r = so_fflush(stream);

		if (r < 0) {
			stream->ferror_flag = SO_EOF;
			return SO_EOF;
		}
	}

	if (stream->last_op == 'r') {
		/* daca inainte s-a realizat o operatie de read
		 * atunci se invalideaza buffer-ul
		 */

		stream->buff_len = 0;
		stream->buff_pos = 0;
		memset(stream->buffer, 0, stream->buff_len);
	}

	/* se calculeaza noua pozitiea a cursorului */
	dwPos = SetFilePointer(
		stream->hFile, 
		offset,
		NULL,
		whence);

	if (dwPos == INVALID_SET_FILE_POINTER) {
		stream->ferror_flag = SO_EOF;
		return SO_EOF;
	}

	stream->currsor = dwPos;

	return 0;
}

/**
 * @return long pozitia curenta a cursorului in fisier, SO_EOF in caz de eroare
 */
long so_ftell(SO_FILE *stream)
{
	if (stream->last_op != 'r') {
		int r = so_fseek(stream, 0, SEEK_CUR);

		if (r < 0)
			return SO_EOF;
	}

	return stream->currsor;
}

/**
 * @brief Realizeaza citirea unui numar de elemente de o anumita dimensiune
 * si le stocheaza intr-o adresa de memorie specificata.
 *
 * @return size_t numarul de caractere citite
 */
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	int read_elem = size * nmemb;
	int ch, i;

	/* citirea se realizeaza caracter cu caracter */
	for (i = 0; i < read_elem; i++) {
		ch = so_fgetc(stream);

		/* se verifica aparitia erorilor */
		if (ch < 0) {
			if (stream->feof_flag != 0 || stream->ferror_flag != 0)
				return i / size;
			else
				return 0;
		}

		((char *)ptr)[i] = ch;
	}

	return nmemb;
}

/**
 * @brief Scrie un numar de elemente de o anumita dimensiune, elementele
 * scrise fiind luate de la o adresa de memorie specificata
 *
 * @return size_t numarul de caractere scrise
 */
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	int write_elem = size * nmemb;
	int i, r;
	char ch;

	/* scirerea se realizeaza caracter cu caracter */
	for (i = 0; i < write_elem; i++) {

		ch = ((char *) ptr)[i];
		r = so_fputc(ch, stream);

		/* verificare erori */
		if (r < 0 &&
		(stream->ferror_flag == SO_EOF || stream->feof_flag == SO_EOF))
			return 0;

	}

	return nmemb;
}

/**
 * @brief Se realizeaza citirea unui caracter din fisier
 *
 * @return int caracterul citit, SO_EOF in caz de eroare
 */
int so_fgetc(SO_FILE *stream)
{
	int r;
	unsigned char ch;
	BOOL dwRet;
	DWORD dwBytesRead;

	stream->last_op = 'r';

	if (stream->buff_pos == 0 || stream->buff_pos >= stream->buff_len) {

		/* se populeaza buffer-ul cu date din fisier, atunci cand
		 * buffer-ul este gol sau plin
		 */

		memset(stream->buffer, 0, stream->buff_pos);

		dwRet = ReadFile(
				stream->hFile,
				stream->buffer,
				MAX_LEN,
				&dwBytesRead,
				NULL);

		if (dwRet == FALSE || dwBytesRead <= 0) {
			stream->feof_flag = SO_EOF;
			stream->ferror_flag = SO_EOF;
			return SO_EOF;
		}

		/* se reseteaza pozitia curenta si dimensiunea buffer-ului */
		stream->buff_pos = 0;
		stream->buff_len = dwBytesRead;
	}

	/* se citeste caracterul din buffer si
	 * se incrementeaza cele doua pozitii
	 */

	ch = stream->buffer[stream->buff_pos];
	stream->buff_pos++;
	stream->currsor++;

	return (unsigned int)ch;
}

/**
 * @brief Realizeaza scrierea unui caracter in fisier
 *
 * @return int caracterul citit, SO_EOF in caz de eroare
 */
int so_fputc(int c, SO_FILE *stream)
{
	int r;

	stream->last_op = 'w';

	if (stream->buff_len == MAX_LEN) {
		/* se scriu datele din buffer folosind flush
		 * cand acesta este plin
		 */

		r = so_fflush(stream);

		if (r != 0)
			return SO_EOF;

	}

	/* se adauga caracterul in buffer si
	 * se incrementeaza cele doua pozitii
	 */

	stream->buffer[stream->buff_len] = c;
	stream->buff_len++;
	stream->currsor++;

	return c;
}

/**
 * @brief Intoarce flag-ul ce indica finalul fisierului
 *
 * @return int 0 nu s-a ajuns la finalul fisierului, SO_EOF in caz contrar
 */
int so_feof(SO_FILE *stream)
{
	return stream->feof_flag;

}

/**
 * @brief Intoarce flag-ul care indica daca a aparut o eroare in urma unei
 * operatii cu fisierul
 *
 * @return int SO_EOF eroare, 0 in caz contrar
 */
int so_ferror(SO_FILE *stream)
{
	return stream->ferror_flag;
}


SO_FILE *so_popen(const char *command, const char *type)
{
	return NULL;
}


int so_pclose(SO_FILE *stream)
{
	return 0;
}

/**
 * @brief Inchide fisierul si elibereaza memoria locata structurii
 *
 * @return int 0 succes, SO_EOF eroare
 */
int so_fclose(SO_FILE *stream)
{
	int r;
	BOOL bRet;

	if (stream->last_op == 'w') {
		/* ultima operatie a fost de citire,
		 * atunci se scriu datele din buffer
		 */

		r = so_fflush(stream);
		if (r < 0) {
			free(stream->buffer);
			free(stream);
			return SO_EOF;
		}
	}

	bRet = CloseHandle(stream->hFile);

	if (bRet == FALSE) {
		free(stream->buffer);
		free(stream);
		return SO_EOF;
	}

	free(stream->buffer);
	free(stream);

	return 0;
}
