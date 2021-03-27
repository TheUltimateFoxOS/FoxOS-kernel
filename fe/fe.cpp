/*
** Copyright (c) 2020 rxi
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to
** deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
** IN THE SOFTWARE.
*/

#include <fe/fe.h>

#define unused(x)		 ( (void) (x) )
#define car(x)				( (x)->car.o )
#define cdr(x)				( (x)->cdr.o )
#define tag(x)				( (x)->car.c )
#define isnil(x)			( (x) == &nil )
#define type(x)			 ( tag(x) & 0x1 ? tag(x) >> 2 : FE_TPAIR )
#define settype(x,t)	( tag(x) = (t) << 2 | 1 )
#define number(x)		 ( (x)->cdr.n )
#define prim(x)			 ( (x)->cdr.c )
#define cfunc(x)			( (x)->cdr.f )
#define strbuf(x)		 ( &(x)->car.c + 1 )

#define STRBUFSIZE		( (int) sizeof(fe_Object*) - 1 )
#define GCMARKBIT		 ( 0x2 )
#define GCSTACKSIZE	 ( 256 )

#define exit(code)		renderer::global_font_renderer->printf("EXIT: %d", code); \
						while(1);

enum {
 P_LET, P_SET, P_IF, P_FN, P_MAC, P_WHILE, P_QUOTE, P_AND, P_OR, P_DO, P_CONS,
 P_CAR, P_CDR, P_SETCAR, P_SETCDR, P_LIST, P_NOT, P_IS, P_ATOM, P_PRINT, P_LT,
 P_LTE, P_ADD, P_SUB, P_MUL, P_DIV, P_MAX
};

static const char *primnames[] = {
	"let", "=", "if", "fn", "mac", "while", "quote", "and", "or", "do", "cons",
	"car", "cdr", "setcar", "setcdr", "list", "not", "is", "atom", "print", "<",
	"<=", "+", "-", "*", "/"
};

static const char *typenames[] = {
	"pair", "free", "nil", "number", "symbol", "string",
	"func", "macro", "prim", "cfunc", "ptr"
};

typedef union { fe_Object *o; fe_CFunc f; fe_Number n; char c; } Value;

struct fe_Object { Value car, cdr; };

struct fe_Context {
	fe_Handlers handlers;
	fe_Object *gcstack[GCSTACKSIZE];
	int gcstack_idx;
	fe_Object *objects;
	int object_count;
	fe_Object *calllist;
	fe_Object *freelist;
	fe_Object *symlist;
	fe_Object *t;
	int nextchr;
};

static fe_Object nil = {{ (fe_Object*) (FE_TNIL << 2 | 1) }, { NULL }};


fe_Handlers* fe_handlers(fe_Context *ctx) {
	return &ctx->handlers;
}

// ABSTRACTION CODE START


#define ZEROPAD	1
#define SIGN	2
#define PLUS	4
#define SPACE	8
#define LEFT	16
#define SMALL	32
#define SPECIAL	64

#define isdigit(c) (c >= '0' && c <= '9')

#define __do_div(n, base) ({					\
int __res;										\
__res = ((unsigned long) n) % (unsigned) base;	\
n = ((unsigned long) n) / (unsigned) base;		\
__res; })

int strlen(char *src){
	int i = 0;
	while (*src++)
		i++;
	return i;
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

size_t strnlen(const char *s, size_t maxlen) {
	size_t i;
	for (i = 0; i < maxlen; ++i)
	if (s[i] == '\0')
		break;
	return i;
}

int strcmp(char *str1, char *str2){
	int i = 0;
	int failed = 0;
	while(str1[i] != '\0' && str2[i] != '\0'){
		if(str1[i] != str2[i]){
			failed = 1;
			break;
		}
		i++;
	}
	if( (str1[i] == '\0' && str2[i] != '\0') || (str1[i] != '\0' && str2[i] == '\0') )
		failed = 1;
	return failed;
}


#define uh_oh() not_implemented(__FILE__, __LINE__);

#define isspace(c) (c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r' || c == ' ')

void not_implemented(const char* __file, unsigned int __line) {
	renderer::global_font_renderer->printf("The feature in the file: %s at the line: %d isn't implemented yet!\n@Glowman554 please do that as soon as posible.\n", __file, __line);
	while(1);
}


static int maxExponent = 511;

static double powersOf10[] = {
	10.,
	100.,
	1.0e4,
	1.0e8,
	1.0e16,
	1.0e32,
	1.0e64,
	1.0e128,
	1.0e256
};


double strtod(const char* string, char** endPtr) 
{
	bool sign, expSign = false;
	double fraction, dblExp, *d;
	register const char *p;
	register int c;
	int exp = 0;
	int fracExp = 0;		
	int mantSize;		
	int decPt;			
	const char *pExp;

	p = string;
	while (isspace(*p)) {
		p += 1;
	}
	if (*p == '-') {
		sign = true;
		p += 1;
	} else {
		if (*p == '+') {
			p += 1;
		}
		sign = false;
	}

	decPt = -1;
	for (mantSize = 0; ; mantSize += 1) {
		c = *p;
		if (!isdigit(c)) {
			if ((c != '.') || (decPt >= 0)) {
				break;
			}
			decPt = mantSize;
		}
		p += 1;
	}
	
	pExp  = p;
	p -= mantSize;
	if (decPt < 0) {
		decPt = mantSize;
	} else {
		mantSize -= 1;
	}
	if (mantSize > 18) {
		fracExp = decPt - 18;
		mantSize = 18;
	} else {
		fracExp = decPt - mantSize;
	}
	if (mantSize == 0) {
		fraction = 0.0;
		p = string;
		goto done;
	} else {
		int frac1, frac2;
		frac1 = 0;
		for ( ; mantSize > 9; mantSize -= 1) {
			c = *p;
			p += 1;
			if (c == '.') {
				c = *p;
				p += 1;
			}
			frac1 = 10*frac1 + (c - '0');
		}
		frac2 = 0;
		for (; mantSize > 0; mantSize -= 1) {
			c = *p;
			p += 1;
			if (c == '.') {
				c = *p;
				p += 1;
			}
			frac2 = 10*frac2 + (c - '0');
		}
		fraction = (1.0e9 * frac1) + frac2;
	}

	p = pExp;
	if ((*p == 'E') || (*p == 'e')) {
		p += 1;
		if (*p == '-') {
			expSign = true;
			p += 1;
		} else {
			if (*p == '+') {
				p += 1;
			}
			expSign = false;
		}
		while (isdigit(*p)) {
			exp = exp * 10 + (*p - '0');
			p += 1;
		}
	}
	if (expSign) {
		exp = fracExp - exp;
	} else {
		exp = fracExp + exp;
	}
	
	if (exp < 0) {
		expSign = true;
		exp = -exp;
	} else {
		expSign = false;
	}
	if (exp > maxExponent) {
		exp = maxExponent;
	}
	dblExp = 1.0;
	for (d = powersOf10; exp != 0; exp >>= 1, d += 1) {
		if (exp & 01) {
			dblExp *= *d;
		}
	}
	if (expSign) {
		fraction /= dblExp;
	} else {
		fraction *= dblExp;
	}

done:
	if (endPtr != NULL) {
		*endPtr = (char *) p;
	}

	if (sign) {
		return -fraction;
	}
	return fraction;
}


int skip_atoi(const char **s) {
	int i = 0;

	while (isdigit(**s))
		i = i * 10 + *((*s)++) - '0';
	return i;
}


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

int sprintf(char *buf, const char *fmt, ...) {
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsprintf(buf, fmt, args);
	va_end(args);
	return i;
}

// ABSTRACTION CODE END


void fe_error(fe_Context *ctx, const char *msg) {
	fe_Object *cl = ctx->calllist;
	/* reset context state */
	ctx->calllist = &nil;
	/* do error handler */
	if (ctx->handlers.error) { ctx->handlers.error(ctx, msg, cl); }
	/* error handler returned -- print error and traceback, exit */
	renderer::global_font_renderer->printf("error: %s\n", msg);
	for (; !isnil(cl); cl = cdr(cl)) {
		char buf[64];
		fe_tostring(ctx, car(cl), buf, sizeof(buf));
		renderer::global_font_renderer->printf("=> %s\n", buf);
	}
	exit(-0xf00dbabe);
}


fe_Object* fe_nextarg(fe_Context *ctx, fe_Object **arg) {
	fe_Object *a = *arg;
	if (type(a) != FE_TPAIR) {
		if (isnil(a)) { fe_error(ctx, "too few arguments"); }
		fe_error(ctx, "dotted pair in argument list");
	}
	*arg = cdr(a);
	return car(a);
}


static fe_Object* checktype(fe_Context *ctx, fe_Object *obj, int type) {
	char buf[64];
	if (type(obj) != type) {
		sprintf(buf, "expected %s, got %s", typenames[type], typenames[type(obj)]);
		fe_error(ctx, buf);
	}
	return obj;
}


int fe_type(fe_Context *ctx, fe_Object *obj) {
	unused(ctx);
	return type(obj);
}


int fe_isnil(fe_Context *ctx, fe_Object *obj) {
	unused(ctx);
	return isnil(obj);
}


void fe_pushgc(fe_Context *ctx, fe_Object *obj) {
	if (ctx->gcstack_idx == GCSTACKSIZE) {
		fe_error(ctx, "gc stack overflow");
	}
	ctx->gcstack[ctx->gcstack_idx++] = obj;
}


void fe_restoregc(fe_Context *ctx, int idx) {
	ctx->gcstack_idx = idx;
}


int fe_savegc(fe_Context *ctx) {
	return ctx->gcstack_idx;
}


void fe_mark(fe_Context *ctx, fe_Object *obj) {
	fe_Object *car;
begin:
	if (tag(obj) & GCMARKBIT) { return; }
	car = car(obj); /* store car before modifying it with GCMARKBIT */
	tag(obj) |= GCMARKBIT;

	switch (type(obj)) {
		case FE_TPAIR:
			fe_mark(ctx, car);
			/* fall through */
		case FE_TFUNC: case FE_TMACRO: case FE_TSYMBOL: case FE_TSTRING:
			obj = cdr(obj);
			goto begin;

		case FE_TPTR:
			if (ctx->handlers.mark) { ctx->handlers.mark(ctx, obj); }
			break;
	}
}


static void collectgarbage(fe_Context *ctx) {
	int i;
	/* mark */
	for (i = 0; i < ctx->gcstack_idx; i++) {
		fe_mark(ctx, ctx->gcstack[i]);
	}
	fe_mark(ctx, ctx->symlist);
	/* sweep and unmark */
	for (i = 0; i < ctx->object_count; i++) {
		fe_Object *obj = &ctx->objects[i];
		if (type(obj) == FE_TFREE) { continue; }
		if (~tag(obj) & GCMARKBIT) {
			if (type(obj) == FE_TPTR && ctx->handlers.gc) {
				ctx->handlers.gc(ctx, obj);
			}
			settype(obj, FE_TFREE);
			cdr(obj) = ctx->freelist;
			ctx->freelist = obj;
		} else {
			tag(obj) &= ~GCMARKBIT;
		}
	}
}


static int equal(fe_Object *a, fe_Object *b) {
	if (a == b) { return 1; }
	if (type(a) != type(b)) { return 0; }
	if (type(a) == FE_TNUMBER) { return number(a) == number(b); }
	if (type(a) == FE_TSTRING) {
		for (; !isnil(a); a = cdr(a), b = cdr(b)) {
			if (car(a) != car(b)) { return 0; }
		}
		return a == b;
	}
	return 0;
}


static int streq(fe_Object *obj, const char *str) {
	while (!isnil(obj)) {
		int i;
		for (i = 0; i < STRBUFSIZE; i++) {
			if (strbuf(obj)[i] != *str) { return 0; }
			if (*str) { str++; }
		}
		obj = cdr(obj);
	}
	return *str == '\0';
}


static fe_Object* object(fe_Context *ctx) {
	fe_Object *obj;
	/* do gc if freelist has no more objects */
	if (isnil(ctx->freelist)) {
		collectgarbage(ctx);
		if (isnil(ctx->freelist)) { fe_error(ctx, "out of memory"); }
	}
	/* get object from freelist and push to the gcstack */
	obj = ctx->freelist;
	ctx->freelist = cdr(obj);
	fe_pushgc(ctx, obj);
	return obj;
}


fe_Object* fe_cons(fe_Context *ctx, fe_Object *car, fe_Object *cdr) {
	fe_Object *obj = object(ctx);
	car(obj) = car;
	cdr(obj) = cdr;
	return obj;
}


fe_Object* fe_bool(fe_Context *ctx, int b) {
	return b ? ctx->t : &nil;
}


fe_Object* fe_number(fe_Context *ctx, fe_Number n) {
	fe_Object *obj = object(ctx);
	settype(obj, FE_TNUMBER);
	number(obj) = n;
	return obj;
}


static fe_Object* buildstring(fe_Context *ctx, fe_Object *tail, int chr) {
	if (!tail || strbuf(tail)[STRBUFSIZE - 1] != '\0') {
		fe_Object *obj = fe_cons(ctx, NULL, &nil);
		settype(obj, FE_TSTRING);
		if (tail) {
			cdr(tail) = obj;
			ctx->gcstack_idx--;
		}
		tail = obj;
	}
	strbuf(tail)[strlen(strbuf(tail))] = chr;
	return tail;
}


fe_Object* fe_string(fe_Context *ctx, const char *str) {
	fe_Object *obj = buildstring(ctx, NULL, '\0');
	fe_Object *tail = obj;
	while (*str) {
		tail = buildstring(ctx, tail, *str++);
	}
	return obj;
}


fe_Object* fe_symbol(fe_Context *ctx, const char *name) {
	fe_Object *obj;
	/* try to find in symlist */
	for (obj = ctx->symlist; !isnil(obj); obj = cdr(obj)) {
		if (streq(car(cdr(car(obj))), name)) {
			return car(obj);
		}
	}
	/* create new object, push to symlist and return */
	obj = object(ctx);
	settype(obj, FE_TSYMBOL);
	cdr(obj) = fe_cons(ctx, fe_string(ctx, name), &nil);
	ctx->symlist = fe_cons(ctx, obj, ctx->symlist);
	return obj;
}


fe_Object* fe_cfunc(fe_Context *ctx, fe_CFunc fn) {
	fe_Object *obj = object(ctx);
	settype(obj, FE_TCFUNC);
	cfunc(obj) = fn;
	return obj;
}


fe_Object* fe_ptr(fe_Context *ctx, void *ptr) {
	fe_Object *obj = object(ctx);
	settype(obj, FE_TPTR);
	cdr(obj) = (fe_Object*) ptr;
	return obj;
}


fe_Object* fe_list(fe_Context *ctx, fe_Object **objs, int n) {
	fe_Object *res = &nil;
	while (n--) {
		res = fe_cons(ctx, objs[n], res);
	}
	return res;
}


fe_Object* fe_car(fe_Context *ctx, fe_Object *obj) {
	if (isnil(obj)) { return obj; }
	return car(checktype(ctx, obj, FE_TPAIR));
}


fe_Object* fe_cdr(fe_Context *ctx, fe_Object *obj) {
	if (isnil(obj)) { return obj; }
	return cdr(checktype(ctx, obj, FE_TPAIR));
}


static void writestr(fe_Context *ctx, fe_WriteFn fn, void *udata, const char *s) {
	while (*s) { fn(ctx, udata, *s++); }
}

void fe_write(fe_Context *ctx, fe_Object *obj, fe_WriteFn fn, void *udata, int qt) {
	char buf[32];

	switch (type(obj)) {
		case FE_TNIL:
			writestr(ctx, fn, udata, "nil");
			break;

		case FE_TNUMBER:
			sprintf(buf, "%.7g", number(obj));
			writestr(ctx, fn, udata, buf);
			break;

		case FE_TPAIR:
			fn(ctx, udata, '(');
			for (;;) {
				fe_write(ctx, car(obj), fn, udata, 1);
				obj = cdr(obj);
				if (type(obj) != FE_TPAIR) { break; }
				fn(ctx, udata, ' ');
			}
			if (!isnil(obj)) {
				writestr(ctx, fn, udata, " . ");
				fe_write(ctx, obj, fn, udata, 1);
			}
			fn(ctx, udata, ')');
			break;

		case FE_TSYMBOL:
			fe_write(ctx, car(cdr(obj)), fn, udata, 0);
			break;

		case FE_TSTRING:
			if (qt) { fn(ctx, udata, '"'); }
			while (!isnil(obj)) {
				int i;
				for (i = 0; i < STRBUFSIZE && strbuf(obj)[i]; i++) {
					if (qt && strbuf(obj)[i] == '"') { fn(ctx, udata, '\\'); }
					fn(ctx, udata, strbuf(obj)[i]);
				}
				obj = cdr(obj);
			}
			if (qt) { fn(ctx, udata, '"'); }
			break;

		default:
			sprintf(buf, "[%s %p]", typenames[type(obj)], (void*) obj);
			writestr(ctx, fn, udata, buf);
			break;
	}
}

static void write_screen(fe_Context *ctx, void* udata, char chr) {
	unused(ctx);
	renderer::global_font_renderer->printf("%c", chr);
}


typedef struct { char *p; int n; } CharPtrInt;

static void writebuf(fe_Context *ctx, void *udata, char chr) {
	CharPtrInt *x = (CharPtrInt*) udata;
	unused(ctx);
	if (x->n) { *x->p++ = chr; x->n--; }
}

int fe_tostring(fe_Context *ctx, fe_Object *obj, char *dst, int size) {
	CharPtrInt x;
	x.p = dst;
	x.n = size - 1;
	fe_write(ctx, obj, writebuf, &x, 0);
	*x.p = '\0';
	return size - x.n - 1;
}


fe_Number fe_tonumber(fe_Context *ctx, fe_Object *obj) {
	return number(checktype(ctx, obj, FE_TNUMBER));
}


void* fe_toptr(fe_Context *ctx, fe_Object *obj) {
	return cdr(checktype(ctx, obj, FE_TPTR));
}


static fe_Object* getbound(fe_Object *sym, fe_Object *env) {
	/* try to find in environment */
	for (; !isnil(env); env = cdr(env)) {
		fe_Object *x = car(env);
		if (car(x) == sym) { return x; }
	}
	/* return global */
	return cdr(sym);
}


void fe_set(fe_Context *ctx, fe_Object *sym, fe_Object *v) {
	unused(ctx);
	cdr(getbound(sym, &nil)) = v;
}


static fe_Object rparen;

fe_Object* read_data(fe_Context *ctx, fe_ReadFn fn, void *udata) {
	const char *delimiter = " \n\t\r();";
	fe_Object *v, *res, **tail;
	fe_Number n;
	int chr, gc;
	char buf[64], *p;

	/* get next character */
	chr = ctx->nextchr ? ctx->nextchr : fn(ctx, udata);
	ctx->nextchr = '\0';

	/* skip whitespace */
	while (chr && strchr(" \n\t\r", chr)) {
		chr = fn(ctx, udata);
	}

	switch (chr) {
		case '\0':
			return NULL;

		case ';':
			while (chr && chr != '\n') { chr = fn(ctx, udata); }
			return read_data(ctx, fn, udata);

		case ')':
			return &rparen;

		case '(':
			res = &nil;
			tail = &res;
			gc = fe_savegc(ctx);
			fe_pushgc(ctx, res); /* to cause error on too-deep nesting */
			while ( (v = read_data(ctx, fn, udata)) != &rparen ) {
				if (v == NULL) { fe_error(ctx, "unclosed list"); }
				if (type(v) == FE_TSYMBOL && streq(car(cdr(v)), ".")) {
					/* dotted pair */
					*tail = fe_read(ctx, fn, udata);
				} else {
					/* proper pair */
					*tail = fe_cons(ctx, v, &nil);
					tail = &cdr(*tail);
				}
				fe_restoregc(ctx, gc);
				fe_pushgc(ctx, res);
			}
			return res;

		case '\'':
			v = fe_read(ctx, fn, udata);
			if (!v) { fe_error(ctx, "stray '''"); }
			return fe_cons(ctx, fe_symbol(ctx, "quote"), fe_cons(ctx, v, &nil));

		case '"':
			res = buildstring(ctx, NULL, '\0');
			v = res;
			chr = fn(ctx, udata);
			while (chr != '"') {
				if (chr == '\0') { fe_error(ctx, "unclosed string"); }
				if (chr == '\\') {
					chr = fn(ctx, udata);
					if (strchr("nrt", chr)) { chr = strchr("n\nr\rt\t", chr)[1]; }
				}
				v = buildstring(ctx, v, chr);
				chr = fn(ctx, udata);
			}
			return res;

		default:
			p = buf;
			do {
				if (p == buf + sizeof(buf) - 1) { fe_error(ctx, "symbol too long"); }
				*p++ = chr;
				chr = fn(ctx, udata);
			} while (chr && !strchr(delimiter, chr));
			*p = '\0';
			ctx->nextchr = chr;
			//uh_oh();
			n = strtod(buf, &p);	/* try to read as number */
			if (p != buf && strchr(delimiter, *p)) { return fe_number(ctx, n); }
			if (!strcmp(buf, (char*) "nil")) { return &nil; }
			return fe_symbol(ctx, buf);
	}
}


fe_Object* fe_read(fe_Context *ctx, fe_ReadFn fn, void *udata) {
	fe_Object* obj = read_data(ctx, fn, udata);
	if (obj == &rparen) { fe_error(ctx, "stray ')'"); }
	return obj;
}





static fe_Object* eval(fe_Context *ctx, fe_Object *obj, fe_Object *env, fe_Object **bind);

static fe_Object* evallist(fe_Context *ctx, fe_Object *lst, fe_Object *env) {
	fe_Object *res = &nil;
	fe_Object **tail = &res;
	while (!isnil(lst)) {
		*tail = fe_cons(ctx, eval(ctx, fe_nextarg(ctx, &lst), env, NULL), &nil);
		tail = &cdr(*tail);
	}
	return res;
}


static fe_Object* dolist(fe_Context *ctx, fe_Object *lst, fe_Object *env) {
	fe_Object *res = &nil;
	int save = fe_savegc(ctx);
	while (!isnil(lst)) {
		fe_restoregc(ctx, save);
		fe_pushgc(ctx, lst);
		fe_pushgc(ctx, env);
		res = eval(ctx, fe_nextarg(ctx, &lst), env, &env);
	}
	return res;
}


static fe_Object* argstoenv(fe_Context *ctx, fe_Object *prm, fe_Object *arg, fe_Object *env) {
	while (!isnil(prm)) {
		if (type(prm) != FE_TPAIR) {
			env = fe_cons(ctx, fe_cons(ctx, prm, arg), env);
			break;
		}
		env = fe_cons(ctx, fe_cons(ctx, car(prm), fe_car(ctx, arg)), env);
		prm = cdr(prm);
		arg = fe_cdr(ctx, arg);
	}
	return env;
}


#define evalarg() eval(ctx, fe_nextarg(ctx, &arg), env, (fe_Object**) NULL)

#define arithop(op) {														 \
		fe_Number x = fe_tonumber(ctx, evalarg());		\
		while (!isnil(arg)) {												 \
			x = x op fe_tonumber(ctx, evalarg());			 \
		}																						 \
		res = fe_number(ctx, x);											\
	}

#define numcmpop(op) {														\
		va = checktype(ctx, evalarg(), FE_TNUMBER);	 \
		vb = checktype(ctx, evalarg(), FE_TNUMBER);	 \
		res = fe_bool(ctx, number(va) op number(vb)); \
	}


static fe_Object* eval(fe_Context *ctx, fe_Object *obj, fe_Object *env, fe_Object **newenv) {
	fe_Object *fn, *arg, *res;
	fe_Object cl, *va, *vb;
	int n, gc;

	if (type(obj) == FE_TSYMBOL) { return cdr(getbound(obj, env)); }
	if (type(obj) != FE_TPAIR) { return obj; }

	car(&cl) = obj, cdr(&cl) = ctx->calllist;
	ctx->calllist = &cl;

	gc = fe_savegc(ctx);
	fn = eval(ctx, car(obj), env, NULL);
	arg = cdr(obj);
	res = &nil;

	switch (type(fn)) {
		case FE_TPRIM:
			switch (prim(fn)) {
				case P_LET:
					va = checktype(ctx, fe_nextarg(ctx, &arg), FE_TSYMBOL);
					if (newenv) {
						*newenv = fe_cons(ctx, fe_cons(ctx, va, evalarg()), env);
					}
					break;

				case P_SET:
					va = checktype(ctx, fe_nextarg(ctx, &arg), FE_TSYMBOL);
					cdr(getbound(va, env)) = evalarg();
					break;

				case P_IF:
					while (!isnil(arg)) {
						va = evalarg();
						if (!isnil(va)) {
							res = isnil(arg) ? va : evalarg();
							break;
						}
						if (isnil(arg)) { break; }
						arg = cdr(arg);
					}
					break;

				case P_FN: case P_MAC:
					va = fe_cons(ctx, env, arg);
					fe_nextarg(ctx, &arg);
					res = object(ctx);
					settype(res, prim(fn) == P_FN ? FE_TFUNC : FE_TMACRO);
					cdr(res) = va;
					break;

				case P_WHILE:
					va = fe_nextarg(ctx, &arg);
					n = fe_savegc(ctx);
					while (!isnil(eval(ctx, va, env, NULL))) {
						dolist(ctx, arg, env);
						fe_restoregc(ctx, n);
					}
					break;

				case P_QUOTE:
					res = fe_nextarg(ctx, &arg);
					break;

				case P_AND:
					while (!isnil(arg) && !isnil(res = evalarg()));
					break;

				case P_OR:
					while (!isnil(arg) && isnil(res = evalarg()));
					break;

				case P_DO:
					res = dolist(ctx, arg, env);
					break;

				case P_CONS:
					va = evalarg();
					res = fe_cons(ctx, va, evalarg());
					break;

				case P_CAR:
					res = fe_car(ctx, evalarg());
					break;

				case P_CDR:
					res = fe_cdr(ctx, evalarg());
					break;

				case P_SETCAR:
					va = checktype(ctx, evalarg(), FE_TPAIR);
					car(va) = evalarg();
					break;

				case P_SETCDR:
					va = checktype(ctx, evalarg(), FE_TPAIR);
					cdr(va) = evalarg();
					break;

				case P_LIST:
					res = evallist(ctx, arg, env);
					break;

				case P_NOT:
					res = fe_bool(ctx, isnil(evalarg()));
					break;

				case P_IS:
					va = evalarg();
					res = fe_bool(ctx, equal(va, evalarg()));
					break;

				case P_ATOM:
					res = fe_bool(ctx, fe_type(ctx, evalarg()) != FE_TPAIR);
					break;

				case P_PRINT:
					while (!isnil(arg)) {
						fe_write(ctx, evalarg(), write_screen, 0, 0);
						if (!isnil(arg)) { renderer::global_font_renderer->printf(" "); }
					}
					renderer::global_font_renderer->printf("\n");
					break;

				case P_LT: numcmpop(<); break;
				case P_LTE: numcmpop(<=); break;
				case P_ADD: arithop(+); break;
				case P_SUB: arithop(-); break;
				case P_MUL: arithop(*); break;
				case P_DIV: arithop(/); break;
			}
			break;

		case FE_TCFUNC:
			res = cfunc(fn)(ctx, evallist(ctx, arg, env));
			break;

		case FE_TFUNC:
			arg = evallist(ctx, arg, env);
			va = cdr(fn); /* (env params ...) */
			vb = cdr(va); /* (params ...) */
			res = dolist(ctx, cdr(vb), argstoenv(ctx, car(vb), arg, car(va)));
			break;

		case FE_TMACRO:
			va = cdr(fn); /* (env params ...) */
			vb = cdr(va); /* (params ...) */
			/* replace caller object with code generated by macro and re-eval */
			*obj = *dolist(ctx, cdr(vb), argstoenv(ctx, car(vb), arg, car(va)));
			fe_restoregc(ctx, gc);
			ctx->calllist = cdr(&cl);
			return eval(ctx, obj, env, NULL);

		default:
			fe_error(ctx, "tried to call non-callable value");
	}

	fe_restoregc(ctx, gc);
	fe_pushgc(ctx, res);
	ctx->calllist = cdr(&cl);
	return res;
}


fe_Object* fe_eval(fe_Context *ctx, fe_Object *obj) {
	return eval(ctx, obj, &nil, NULL);
}


fe_Context* fe_open(void *ptr, int size) {
	int i, save;
	fe_Context *ctx;

	/* init context struct */
	ctx = (fe_Context*) ptr;
	memset(ctx, 0, sizeof(fe_Context));
	ptr = (char*) ptr + sizeof(fe_Context);
	size -= sizeof(fe_Context);

	/* init objects memory region */
	ctx->objects = (fe_Object*) ptr;
	ctx->object_count = size / sizeof(fe_Object);

	/* init lists */
	ctx->calllist = &nil;
	ctx->freelist = &nil;
	ctx->symlist = &nil;

	/* populate freelist */
	for (i = 0; i < ctx->object_count; i++) {
		fe_Object *obj = &ctx->objects[i];
		settype(obj, FE_TFREE);
		cdr(obj) = ctx->freelist;
		ctx->freelist = obj;
	}

	/* init objects */
	ctx->t = fe_symbol(ctx, "t");
	fe_set(ctx, ctx->t, ctx->t);

	/* register built in primitives */
	save = fe_savegc(ctx);
	for (i = 0; i < P_MAX; i++) {
		fe_Object *v = object(ctx);
		settype(v, FE_TPRIM);
		prim(v) = i;
		fe_set(ctx, fe_symbol(ctx, primnames[i]), v);
		fe_restoregc(ctx, save);
	}

	return ctx;
}


void fe_close(fe_Context *ctx) {
	/* clear gcstack and symlist; makes all objects unreachable */
	ctx->gcstack_idx = 0;
	ctx->symlist = &nil;
	collectgarbage(ctx);
}