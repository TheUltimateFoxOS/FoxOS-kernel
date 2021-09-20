#include <string.h>
#include <stdarg.h>

//#strcpy-doc: Copy a source string into a destination string.
char* strcpy(char* dest, const char* src) {
	do {
		*dest++ = *src++;
	} while (*src != 0);
	return 0;
}

//#strlen-doc: Get the length of a string.
int strlen(char* src){
	int i = 0;
	while (*src++)
		i++;
	return i;
}

//#strnlen-doc: Get the length of a string, with a maximum.
size_t strnlen(const char *s, size_t maxlen) {
	size_t i;
	for (i = 0; i < maxlen; ++i)
	if (s[i] == '\0')
		break;
	return i;
}

//#strcat-doc: Copy a source string into a destination string.
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

//#strcmp-doc: Compare str1 to str2. Return 0 if they are the same, and 1 if they are different.
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

//#strstr-doc: Returns a pointer to the first occurrence of X in Y, or null if X is not in Y.
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

//#strchr-doc: Returns a pointer to the first occurrence of the character c in s.
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

//#strrchr-doc: Returns a pointer to the last occurrence of the character c in s.
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

//#strncmp-doc: Compare s1 to s2. Return 0 if they are the same up to the size n of str1, and 1 if they are different.
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

//#memcpy-doc: Copy a chunk of memory at the pointer src and with a size of n into dest.
void* memcpy(void* dest, const void* src, size_t n){
	unsigned char* d = (unsigned char*) dest;
	const unsigned char* s = (unsigned char*) src;

	while (n--) {
		*d++ = *s++;
	}

	return dest;
}

//#memcmp-doc: Compare _s1 with a size of n to _s2.
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

//#strncpy-doc: Copy a number (count) of characters from _src to _dst.
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

#define ZEROPAD	1
#define SIGN	2
#define PLUS	4
#define SPACE	8
#define LEFT	16
#define SMALL	32
#define SPECIAL	64

#define __do_div(n, base) ({					\
int __res;										\
__res = ((unsigned long) n) % (unsigned) base;	\
n = ((unsigned long) n) / (unsigned) base;		\
__res; })

#define isdigit(c) (c >= '0' && c <= '9')

//#skip_atoi-doc: Convert a string s into an integer.
int skip_atoi(const char **s) {
	int i = 0;

	while (isdigit(**s))
		i = i * 10 + *((*s)++) - '0';
	return i;
}

//#number_printf-doc: Copy a number into str.
char *number_printf(char *str, long num, int base, int size, int precision, int type) {
	static const char digits[17] = "0123456789ABCDEF";

	char tmp[66];
	char c, sign, locase;
	int i;

	locase = (type & SMALL);
	if (type & LEFT) {
		type &= ~ZEROPAD;
	}
	if (base < 2 || base > 16) {
		return NULL;
	}
	c = (type & ZEROPAD) ? '0' : ' ';

	sign = 0;

	if (type & SIGN) {
		if (num < 0) {
			sign = '-';
			num = -num;
			size--;
		} else if (type & PLUS) {
			sign = '+';
			size--;
		} else if (type & SPACE) {
			sign = ' ';
			size--;
		}
	}
	if (type & SPECIAL) {
		if (base == 16) {
			size -= 2;
		}
		else if (base == 8) {
			size--;
		}
	}
	i = 0;
	if (num == 0) {
		tmp[i++] = '0';
	}
	else {
		while (num != 0) {
			tmp[i++] = (digits[__do_div(num, base)] | locase);
		}
	}
	if (i > precision) {
		precision = i;
	}
	size -= precision;
	if (!(type & (ZEROPAD + LEFT))) {
		while (size-- > 0) {
			*str++ = ' ';
		}
	}
	if (sign) {
		*str++ = sign;
	}
	if (type & SPECIAL) {
		if (base == 8) {
			*str++ = '0';
		} else if (base == 16) {
			*str++ = '0';
			*str++ = ('X' | locase);
		}
	}
	if (!(type & LEFT)) {
		while (size-- > 0) {
			*str++ = c;
		}
	}
	while (i < precision--) {
		*str++ = '0';
	}
	while (i-- > 0) {
		*str++ = tmp[i];
	}
	while (size-- > 0) {
		*str++ = ' ';
	}
	return str;
}

//#vsprintf-doc: Printf some data into buf.
int vsprintf(char *buf, const char *fmt, va_list args) {
	int len;
	unsigned long num;
	int i, base;
	char *str;
	const char *s;

	int flags;

	int field_width;
	int precision;
	int qualifier;

	for (str = buf; *fmt; ++fmt) {
		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}
		flags = 0;
	repeat:
		++fmt;
		switch (*fmt) {
			case '-':
				flags |= LEFT;
				goto repeat;
			case '+':
				flags |= PLUS;
				goto repeat;
			case ' ':
				flags |= SPACE;
				goto repeat;
			case '#':
				flags |= SPECIAL;
				goto repeat;
			case '0':
				flags |= ZEROPAD;
				goto repeat;
		}

		field_width = -1;
		if (isdigit(*fmt)) {
			field_width = skip_atoi(&fmt);
		} else if (*fmt == '*') {
			++fmt;

			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		precision = -1;
		if (*fmt == '.') {
			++fmt;
			if (isdigit(*fmt)) {
				precision = skip_atoi(&fmt);
			} else if (*fmt == '*') {
				++fmt;
				precision = va_arg(args, int);
			}
			if (precision < 0) {
				precision = 0;
			}
		}

		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
			qualifier = *fmt;
			++fmt;
		}

		base = 10;

		switch (*fmt) {
			case 'c':
				if (!(flags & LEFT)) {
					while (--field_width > 0) {
						*str++ = ' ';
					}
				}
				*str++ = (unsigned char) va_arg(args, int);
				while (--field_width > 0) {
					*str++ = ' ';
				}
				continue;

			case 's':
				s = va_arg(args, char *);
				len = strnlen(s, precision);

				if (!(flags & LEFT)) {
					while (len < field_width--) {
						*str++ = ' ';
					}
				}
				for (i = 0; i < len; ++i) {
					*str++ = *s++;
				}
				while (len < field_width--) {
					*str++ = ' ';
				}
				continue;

			case 'p':
				if (field_width == -1) {
					field_width = 2 * sizeof(void *);
					flags |= ZEROPAD;
				}
				str = number_printf(str, (unsigned long) va_arg(args, void *), 16, field_width, precision, flags);
				continue;

			case 'n':
				if (qualifier == 'l') {
					long *ip = va_arg(args, long *);
					*ip = (str - buf);
				} else {
					int *ip = va_arg(args, int *);
					*ip = (str - buf);
				}
				continue;

			case '%':
				*str++ = '%';
				continue;

			case 'o':
				base = 8;
				break;

			case 'x':
				flags |= SMALL;
			case 'X':
				base = 16;
				break;

			case 'd':
			case 'i':
				flags |= SIGN;
			case 'u':
				break;

			default:
				*str++ = '%';
				if (*fmt) {
					*str++ = *fmt;
				} else {
					--fmt;
				}
				continue;
		}
		if (qualifier == 'l') {
			num = va_arg(args, unsigned long);
		}
		else if (qualifier == 'h') {
			num = (unsigned short)va_arg(args, int);
			if (flags & SIGN) {
				num = (short) num;
			}
		} else if (flags & SIGN) {
			num = va_arg(args, int);
		} else {
			num = va_arg(args, unsigned int);
		}
		str = number_printf(str, num, base, field_width, precision, flags);
	}
	*str = '\0';
	return str - buf;
}

//#sprintf-doc: Printf some data into buf.
int sprintf(char *buf, const char *fmt, ...) {
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsprintf(buf, fmt, args);
	va_end(args);
	return i;
}