/*
 *  linux/lib/string.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * stupid library routines.. The optimized versions should generally be found
 * as inline code in <asm-xx/string.h>
 *
 * These are buggy as well..
 *
 * * Fri Jun 25 1999, Ingo Oeser <ioe@informatik.tu-chemnitz.de>
 * -  Added strsep() which will replace strtok() soon (because strsep() is
 *    reentrant and should be faster). Use only strsep() in new code, please.
 *
 * * Sat Feb 09 2002, Jason Thomas <jason@topic.com.au>,
 *                    Matthew Hawkins <matt@mh.dropbear.id.au>
 * -  Kissed strtok() goodbye
 *
 * * Feb 2015, Scott Harvey <scott.harvey@stonybrook.edu>
 */

#include <string.h>
#include <ctype.h>


/**
 * strncasecmp - Case insensitive, length-limited string comparison
 * @s1: One string
 * @s2: The other string
 * @len: the maximum number of characters to compare
 */
int strncasecmp(const char *s1, const char *s2, size_t len)
{
	/* Yes, Virginia, it had better be unsigned */
	unsigned char c1, c2;

	if (!len)
		return 0;

	do {
		c1 = *s1++;
		c2 = *s2++;
		if (!c1 || !c2)
			break;
		if (c1 == c2)
			continue;
		c1 = tolower(c1);
		c2 = tolower(c2);
		if (c1 != c2)
			break;
	} while (--len);
	return (int)c1 - (int)c2;
}


int strnicmp(const char *s1, const char *s2, size_t len)
{
	return strncasecmp(s1, s2, len);
}


int strcasecmp(const char *s1, const char *s2)
{
	int c1, c2;

	do {
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
	} while (c1 == c2 && c1 != 0);
	return c1 - c2;
}


/**
 * strcpy - Copy a %NUL terminated string
 * @dest: Where to copy the string to
 * @src: Where to copy the string from
 */
#undef strcpy
char *strcpy(char *dest, const char *src)
{
	char *tmp = dest;

	while ((*dest++ = *src++) != '\0')
		/* nothing */;
	return tmp;
}


/**
 * strncpy - Copy a length-limited, C-string
 * @dest: Where to copy the string to
 * @src: Where to copy the string from
 * @count: The maximum number of bytes to copy
 *
 * The result is not %NUL-terminated if the source exceeds
 * @count bytes.
 *
 * In the case where the length of @src is less than  that  of
 * count, the remainder of @dest will be padded with %NUL.
 *
 */
char *strncpy(char *dest, const char *src, size_t count)
{
	char *tmp = dest;

	while (count) {
		if ((*tmp = *src) != 0)
			src++;
		tmp++;
		count--;
	}
	return dest;
}


/**
 * strlcpy - Copy a C-string into a sized buffer
 * @dest: Where to copy the string to
 * @src: Where to copy the string from
 * @size: size of destination buffer
 *
 * Compatible with *BSD: the result is always a valid
 * NUL-terminated string that fits in the buffer (unless,
 * of course, the buffer size is zero). It does not pad
 * out the result like strncpy() does.
 */
size_t strlcpy(char *dest, const char *src, size_t size)
{
	size_t ret = strlen(src);

	if (size) {
		size_t len = (ret >= size) ? size - 1 : ret;
		memcpy(dest, src, len);
		dest[len] = '\0';
	}
	return ret;
}


/**
 * strcat - Append one %NUL-terminated string to another
 * @dest: The string to be appended to
 * @src: The string to append to it
 */
#undef strcat
char *strcat(char *dest, const char *src)
{
	char *tmp = dest;

	while (*dest)
		dest++;
	while ((*dest++ = *src++) != '\0')
		;
	return tmp;
}


/**
 * strncat - Append a length-limited, C-string to another
 * @dest: The string to be appended to
 * @src: The string to append to it
 * @count: The maximum numbers of bytes to copy
 *
 * Note that in contrast to strncpy(), strncat() ensures the result is
 * terminated.
 */
char *strncat(char *dest, const char *src, size_t count)
{
	char *tmp = dest;

	if (count) {
		while (*dest)
			dest++;
		while ((*dest++ = *src++) != 0) {
			if (--count == 0) {
				*dest = '\0';
				break;
			}
		}
	}
	return tmp;
}


/**
 * strcmp - Compare two strings
 * @cs: One string
 * @ct: Another string
 */
#undef strcmp
int strcmp(const char *cs, const char *ct)
{
	unsigned char c1, c2;

	while (1) {
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
	}
	return 0;
}


/**
 * strncmp - Compare two length-limited strings
 * @cs: One string
 * @ct: Another string
 * @count: The maximum number of bytes to compare
 */
int strncmp(const char *cs, const char *ct, size_t count)
{
	unsigned char c1, c2;

	while (count) {
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
		count--;
	}
	return 0;
}


/**
 * strchr - Find the first occurrence of a character in a string
 * @s: The string to be searched
 * @c: The character to search for
 */
char *strchr(const char *s, int c)
{
	for (; *s != (char)c; ++s)
		if (*s == '\0')
			return NULL;
	return (char *)s;
}


/**
 * strchrnul - Find and return a character in a string, or end of string
 * @s: The string to be searched
 * @c: The character to search for
 *
 * Returns pointer to first occurrence of 'c' in s. If c is not found, then
 * return a pointer to the null byte at the end of s.
 */
char *strchrnul(const char *s, int c)
{
	while (*s && *s != (char)c)
		s++;
	return (char *)s;
}


/**
 * strrchr - Find the last occurrence of a character in a string
 * @s: The string to be searched
 * @c: The character to search for
 */
char *strrchr(const char *s, int c)
{
       const char *p = s + strlen(s);
       do {
           if (*p == (char)c)
               return (char *)p;
       } while (--p >= s);
       return NULL;
}


/**
 * strnchr - Find a character in a length limited string
 * @s: The string to be searched
 * @count: The number of characters to be searched
 * @c: The character to search for
 */
char *strnchr(const char *s, size_t count, int c)
{
	for (; count-- && *s != '\0'; ++s)
		if (*s == (char)c)
			return (char *)s;
	return NULL;
}


/**
 * skip_spaces - Removes leading whitespace from @str.
 * @str: The string to be stripped.
 *
 * Returns a pointer to the first non-whitespace character in @str.
 */
char *skip_spaces(const char *str)
{
	while (isspace(*str))
		++str;
	return (char *)str;
}

/**
 * strim - Removes leading and trailing whitespace from @s.
 * @s: The string to be stripped.
 *
 * Note that the first trailing whitespace is replaced with a %NUL-terminator
 * in the given string @s. Returns a pointer to the first non-whitespace
 * character in @s.
 */
char *strim(char *s)
{
	size_t size;
	char *end;

	size = strlen(s);
	if (!size)
		return s;

	end = s + size - 1;
	while (end >= s && isspace(*end))
		end--;
	*(end + 1) = '\0';

	return skip_spaces(s);
}


/**
 * strlen - Find the length of a string
 * @s: The string to be sized
 */
size_t strlen(const char *s)
{
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}


/**
 * strnlen - Find the length of a length-limited string
 * @s: The string to be sized
 * @count: The maximum number of bytes to search
 */
size_t strnlen(const char *s, size_t count)
{
	const char *sc;

	for (sc = s; count-- && *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}


/**
 * strspn - Calculate the length of the initial substring of @s which only contain letters in @accept
 * @s: The string to be searched
 * @accept: The string to search for
 */
size_t strspn(const char *s, const char *accept)
{
	const char *p;
	const char *a;
	size_t count = 0;

	for (p = s; *p != '\0'; ++p) {
		for (a = accept; *a != '\0'; ++a) {
			if (*p == *a)
				break;
		}
		if (*a == '\0')
			return count;
		++count;
	}
	return count;
}


/**
 * strcspn - Calculate the length of the initial substring of @s which does not contain letters in @reject
 * @s: The string to be searched
 * @reject: The string to avoid
 */
size_t strcspn(const char *s, const char *reject)
{
	const char *p;
	const char *r;
	size_t count = 0;

	for (p = s; *p != '\0'; ++p) {
		for (r = reject; *r != '\0'; ++r) {
			if (*p == *r)
				return count;
		}
		++count;
	}
	return count;
}


/**
 * strpbrk - Find the first occurrence of a set of characters
 * @cs: The string to be searched
 * @ct: The characters to search for
 */
char *strpbrk(const char *cs, const char *ct)
{
	const char *sc1, *sc2;

	for (sc1 = cs; *sc1 != '\0'; ++sc1) {
		for (sc2 = ct; *sc2 != '\0'; ++sc2) {
			if (*sc1 == *sc2)
				return (char *)sc1;
		}
	}
	return NULL;
}


/**
 * strsep - Split a string into tokens
 * @s: The string to be searched
 * @ct: The characters to search for
 *
 * strsep() updates @s to point after the token, ready for the next call.
 *
 * It returns empty tokens, too, behaving exactly like the libc function
 * of that name. In fact, it was stolen from glibc2 and de-fancy-fied.
 * Same semantics, slimmer shape. ;)
 */
char *strsep(char **s, const char *ct)
{
	char *sbegin = *s;
	char *end;

	if (sbegin == NULL)
		return NULL;

	end = strpbrk(sbegin, ct);
	if (end)
		*end++ = '\0';
	*s = end;
	return sbegin;
}


/**
 * memset - Fill a region of memory with the given value
 * @s: Pointer to the start of the area.
 * @c: The byte to fill the area with
 * @count: The size of the area.
 *
 * Do not use memset() to access IO space, use memset_io() instead.
 */
void *memset(void *s, int c, size_t count)
{
	char *xs = s;

	while (count--)
		*xs++ = c;
	return s;
}


/**
 * memcpy - Copy one area of memory to another
 * @dest: Where to copy to
 * @src: Where to copy from
 * @count: The size of the area.
 *
 * You should not use this function to access IO space, use memcpy_toio()
 * or memcpy_fromio() instead.
 */
void *memcpy(void *dest, const void *src, size_t count)
{
	char *tmp = dest;
	const char *s = src;

	while (count--)
		*tmp++ = *s++;
	return dest;
}


/**
 * memmove - Copy one area of memory to another
 * @dest: Where to copy to
 * @src: Where to copy from
 * @count: The size of the area.
 *
 * Unlike memcpy(), memmove() copes with overlapping areas.
 */
void *memmove(void *dest, const void *src, size_t count)
{
	char *tmp;
	const char *s;

	if (dest <= src) {
		tmp = dest;
		s = src;
		while (count--)
			*tmp++ = *s++;
	} else {
		tmp = dest;
		tmp += count;
		s = src;
		s += count;
		while (count--)
			*--tmp = *--s;
	}
	return dest;
}


/**
 * memcmp - Compare two areas of memory
 * @cs: One area of memory
 * @ct: Another area of memory
 * @count: The size of the area.
 */
#undef memcmp
int memcmp(const void *cs, const void *ct, size_t count)
{
	const unsigned char *su1, *su2;
	int res = 0;

	for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}


/**
 * memscan - Find a character in an area of memory.
 * @addr: The memory area
 * @c: The byte to search for
 * @size: The size of the area.
 *
 * returns the address of the first occurrence of @c, or 1 byte past
 * the area if @c is not found
 */
void *memscan(void *addr, int c, size_t size)
{
	unsigned char *p = addr;

	while (size) {
		if (*p == c)
			return (void *)p;
		p++;
		size--;
	}
  	return (void *)p;
}


/**
 * strstr - Find the first substring in a %NUL terminated string
 * @s1: The string to be searched
 * @s2: The string to search for
 */
char *strstr(const char *s1, const char *s2)
{
	size_t l1, l2;

	l2 = strlen(s2);
	if (!l2)
		return (char *)s1;
	l1 = strlen(s1);
	while (l1 >= l2) {
		l1--;
		if (!memcmp(s1, s2, l2))
			return (char *)s1;
		s1++;
	}
	return NULL;
}


/**
 * strnstr - Find the first substring in a length-limited string
 * @s1: The string to be searched
 * @s2: The string to search for
 * @len: the maximum number of characters to search
 */
char *strnstr(const char *s1, const char *s2, size_t len)
{
	size_t l2;

	l2 = strlen(s2);
	if (!l2)
		return (char *)s1;
	while (len >= l2) {
		len--;
		if (!memcmp(s1, s2, l2))
			return (char *)s1;
		s1++;
	}
	return NULL;
}


/**
 * memchr - Find a character in an area of memory.
 * @s: The memory area
 * @c: The byte to search for
 * @n: The size of the area.
 *
 * returns the address of the first occurrence of @c, or %NULL
 * if @c is not found
 */
void *memchr(const void *s, int c, size_t n)
{
	const unsigned char *p = s;
	while (n-- != 0) {
        	if ((unsigned char)c == *p++) {
			return (void *)(p - 1);
		}
	}
	return NULL;
}

/* Inplace string reverse */
char *strrev(char *str) {
	char *start = str, *end = str, temp;

	if(str == NULL)
		return str;
	while(*end)
		end++;
	end--;
	while(start < end) {
		temp = *end;
		*end-- = *start;
		*start++ = temp;
	}
	return str;
}

/**
* integer to ascii
*/
char *itoa(long long val, int base, char *str, size_t len) {
	int neg = 0;
	size_t i;
	if(str == NULL) {
		return NULL;
	}
	if(base < 2 || base > 36 || len < 1) {
		return str;
	}
	if(val == 0) {
		str[0] = '0';
		str[1] = '\0';
		return str;
	}
	/* add negative sign if base-10 */
	if(val < 0 && base == 10) {
		neg = 1;
		val *= -1;
	}
	for(i = 0; i < len && val != 0; i++) {
		char c = (char)(val % base);
		if(c < 10) {
			c += '0';
		} else {
			c += 'a' - 10;
		}
		str[i] = c;
		val /= base;
	}
	if(neg) {
		str[i++] = '-';
	}
	str[(i < len)? i : len] = '\0';

	return strrev(str);
}

/**
* unsigned integer to ascii
*/
char *uitoa(unsigned long long val, int base, char *str, size_t len) {
	size_t i;
	if(str == NULL) {
		return NULL;
	}
	if(base < 2 || base > 36 || len < 1) {
		return str;
	}
	if(val == 0) {
		str[0] = '0';
		str[1] = '\0';
		return str;
	}
	for(i = 0; i < len && val != 0; i++) {
		char c = (char)(val % base);
		if(c < 10) {
			c += '0';
		} else {
			c += 'a' - 10;
		}
		str[i] = c;
		val /= base;
	}
	str[i] = '\0';
	return strrev(str);
}

/**
* strerror - Convert an error number to a descriptive string.
* @err: The error number of the string
*
* returns a pointer to a string that describes the error number, or NULL if invalid.
*/
char *strerror(int err) {
    static char *err_str[] = {
        "Success",
        "Operation not permitted",
        "No such file or directory",
        "No such process",
        "Interrupted system call",
        "I/O error",
        "No such device or address",
        "Argument list too long",
        "Exec format error",
        "Bad file number",
        "No child processes",
        "Try again",
        "Out of memory",
        "Permission denied",
        "Bad address",
        "Block device required",
        "Device or resource busy",
        "File exists",
        "Cross-device link",
        "No such device",
        "Not a directory",
        "Is a directory",
        "Invalid argument",
        "File table overflow",
        "Too many open files",
        "Not a typewriter",
        "Text file busy",
        "File too large",
        "No space left on device",
        "Illegal seek",
        "Read-only file system",
        "Too many links",
        "Broken pipe",
        "Math argument out of domain of func",
        "Math result not representable",
        "Resource deadlock would occur",
        "File name too long",
        "No record locks available",
        "Function not implemented",
        "Directory not empty",
        "Too many symbolic links encountered",
        "Operation would block",
        "No message of desired type",
        "Identifier removed",
        "Channel number out of range",
        "Level 2 not synchronized",
        "Level 3 halted",
        "Level 3 reset",
        "Link number out of range",
        "Protocol driver not attached",
        "No CSI structure available",
        "Level 2 halted",
        "Invalid exchange",
        "Invalid request descriptor",
        "Exchange full",
        "No anode",
        "Invalid request code",
        "Invalid slot",
        "Resource deadlock would occur",
        "Bad font file format",
        "Device not a stream",
        "No data available",
        "Timer expired",
        "Out of streams resources",
        "Machine is not on the network",
        "Package not installed",
        "Object is remote",
        "Link has been severed",
        "Advertise error",
        "Srmount error",
        "Communication error on send",
        "Protocol error",
        "Multihop attempted",
        "RFS specific error",
        "Not a data message",
        "Value too large for defined data type",
        "Name not unique on network",
        "File descriptor in bad state",
        "Remote address changed",
        "Can not access a needed shared library",
        "Accessing a corrupted shared library",
        ".lib section in a.out corrupted",
        "Attempting to link in too many shared libraries",
        "Cannot exec a shared library directly",
        "Illegal byte sequence",
        "Interrupted system call should be restarted",
        "Streams pipe error",
        "Too many users",
        "Socket operation on non-socket",
        "Destination address required",
        "Message too long",
        "Protocol wrong type for socket",
        "Protocol not available",
        "Protocol not supported",
        "Socket type not supported",
        "Operation not supported on transport endpoint",
        "Protocol family not supported",
        "Address family not supported by protocol",
        "Address already in use",
        "Cannot assign requested address",
        "Network is down",
        "Network is unreachable",
        "Network dropped connection because of reset",
        "Software caused connection abort",
        "Connection reset by peer",
        "No buffer space available",
        "Transport endpoint is already connected",
        "Transport endpoint is not connected",
        "Cannot send after transport endpoint shutdown",
        "Too many references: cannot splice",
        "Connection timed out",
        "Connection refused",
        "Host is down",
        "No route to host",
        "Operation already in progress",
        "Operation now in progress",
        "Stale file handle",
        "Structure needs cleaning",
        "Not a XENIX named type file",
        "No XENIX semaphores available",
        "Is a named type file",
        "Remote I/O error",
        "Quota exceeded",
        "No medium found",
        "Wrong medium type",
        "Operation Canceled",
        "Required key not available",
        "Key has expired",
        "Key has been revoked",
        "Key was rejected by service",
        "Owner died",
        "State not recoverable",
        "Operation not possible due to RF-kill",
        "Memory page has hardware error"
    };
    if(err < 0 || err > sizeof(err_str)/ sizeof(char *)) {
        return NULL;
    }
    return err_str[err];
}

const char * const sys_siglist[] = {
		"SIGHUP",
		"SIGINT",
		"SIGQUIT",
		"SIGILL",
		"SIGTRAP",
		"SIGABRT",
		"SIGIOT",
		"SIGBUS",
		"SIGFPE",
		"SIGKILL",
		"SIGUSR1",
		"SIGSEGV",
		"SIGUSR2",
		"SIGPIPE",
		"SIGALRM",
		"SIGTERM",
		"SIGSTKFLT",
		"SIGCHLD",
		"SIGCONT",
		"SIGSTOP",
		"SIGTSTP",
		"SIGTTIN",
		"SIGTTOU",
		"SIGURG",
		"SIGXCPU",
		"SIGXFSZ",
		"SIGVTALRM",
		"SIGPROF",
		"SIGWINCH",
		"SIGIO",
		"SIGPOLL",
		"SIGPWR",
		"SIGSYS",
		"SIGUNUSED"
};

char *strsignal(int sig) {
	if(sig < 0 || sig > sizeof(sys_siglist)/ sizeof(char *)) {
		return NULL;
	}
	return (char*)sys_siglist[sig];
}
