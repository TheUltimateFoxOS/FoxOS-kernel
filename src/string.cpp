#include <string.h>

char* strcpy(char* dest, const char* src) {
	do {
		*dest++ = *src++;
	} while (*src != 0);
	return 0;
}

int strlen(char* src){
	int i = 0;
	while (*src++)
		i++;
	return i;
}

size_t strnlen(const char *s, size_t maxlen) {
	size_t i;
	for (i = 0; i < maxlen; ++i)
	if (s[i] == '\0')
		break;
	return i;
}

char* strcat(char* dest, const char* src) {
	int i;
	int j;

	for (i = 0; dest[i] != '\0'; i++);
	for (j = 0; src[j] != '\0'; j++) {
		dest[i+j] = src[j];
	}

	dest[i+j] = '\0';

	return dest;
}

int strcmp(char* str1, char* str2) {
	int i = 0;
	int failed = 0;
	while(str1[i] != '\0' && str2[i] != '\0') {
		if(str1[i] != str2[i]){
			failed = 1;
			break;
		}
		i++;
	}
	if((str1[i] == '\0' && str2[i] != '\0') || (str1[i] != '\0' && str2[i] == '\0'))
		failed = 1;
	return failed;
}

const char* strstr(const char* X, const char* Y) {
	if (*Y == '\0') {
		return X;
	}
 
	for (int i = 0; i < strlen((char*) X); i++) {
		if (*(X + i) == *Y) {
			char* ptr = (char*) strstr(X + i + 1, Y + 1);
			return (ptr) ? ptr - 1 : NULL;
		}
	}
	return NULL;
}

char* strchr(const char* s, int c) {
	if(s == NULL) {
		return NULL;
	}
	while(*s) {
		if(*s == (char) c) {
			return (char*) s;
		}
		s++;
	}
	return NULL;
}

char* strrchr(const char* s, int c) {
	const char *found, *p;

	c = (unsigned char) c;
	if (c == '\0') {
		return strchr(s, '\0');
	}

	found = NULL;
	while ((p = strchr (s, c)) != NULL) {
		found = p;
		s = p + 1;
	}

	return (char*) found;
}

int strncmp(const char* s1, const char* s2, size_t n ) {
	while (n && *s1 && (*s1 == *s2)) {
		++s1;
		++s2;
		--n;
	}
	if (n == 0) {
		return 0;
	} else {
		return (*(unsigned char*) s1 - *(unsigned char*) s2);
	}
}

void* memcpy(void* dest, const void* src, size_t n){
	unsigned char* d = (unsigned char*) dest;
	const unsigned char* s = (unsigned char*) src;

	while (n--) {
		*d++ = *s++;
	}

	return dest;
}

int memcmp(const void * _s1, const void* _s2, size_t n) {

	const unsigned char* s1 = (unsigned char*) _s1;
	const unsigned char* s2 = (unsigned char*) _s2;

	while(n--) {
		if(*s1 != *s2) {
			return *s1 - *s2;
		}
		++s1;
		++s2;
	}
	return 0;
}

void* memset(void* s, int c, size_t n) {
	unsigned char* p = (unsigned char*) s;
	while(n--) {
		*p++ = (unsigned char) c;
	}
	return s;
}

char* strncpy(char* _dst, const char* _src, size_t count) {
	size_t len = strlen((char*) _src);
	if(count < len) {
		len = count;
	}
	memcpy(_dst, _src, len);
	for(size_t i = len; i < count; ++i) {
		_dst[i] = 0;
	}
	return _dst;
}