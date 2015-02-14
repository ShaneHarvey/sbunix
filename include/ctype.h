#ifndef	_CTYPE_H
#define	_CTYPE_H	1

//__BEGIN_DECLS
// checks for an alphanumeric character
#define isalnum(c)  (isalpha(c) || isdigit(c))
// checks for an alphabetic character
#define isalpha(c)  (isupper(c) || islower(c))
// checks whether c is a 7-bit unsigned char value that fits into the ASCII
// character set
#define isascii(c)  (!((c) & ~0x7f))
// isblank() checks for a blank character; that is, a space or a tab
#define isblank(c)  ((c) == ' ' || (c) == '\t')
// iscntrl() checks for a control character. ASCII codes 0x00 (NUL) and 0x1f (US), plus 0x7f (DEL).
#define iscntrl(c)  (((c) >= 0x0 && (c) <= 0x1f) || (c) == 0x7f)
// isdigit() checks for a digit (0 through 9).
#define isdigit(c)  ((c) >= '0' && (c) <= '9')
// isgraph() checks for any printable character except space.
#define isgraph(c)  (isascii(c) && (c) > ' ')
// islower() checks for a lowercase character.
#define islower(c)  ((c) >= 'a' && (c) <= 'z')
// isprint() checks for any printable character including space.
#define isprint(c)  (isascii(c) && (c) >= ' ')
// ispunct() checks for any printable character which is not a space or an
// alphanumeric character.
#define ispunct(c)  (isgraph(c) && !isalnum(c))
// isspace() checks for white-space characters. In the "C" and "POSIX" locales,
// these are: space, form-feed ('\f'), newline ('\n'), carriage return ('\r'),
// horizontal tab ('\t'), and vertical tab ('\v').
#define isspace(c)  ((c) == ' ' || (c) == '\f' || (c) == '\n' || (c) == '\r' || (c) == '\t' || (c) == '\v')
// isupper() checks for an uppercase letter.
#define isupper(c)  ((c) >= 'A' && (c) <= 'Z')
// isxdigit() checks for hexadecimal digits, that is,
// one of 0 1 2 3 4 5 6 7 8 9 a b c d e f A B C D E F.
#define isxdigit(c) (isdigit(c) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'Z'))


#define tolower(c)  (isupper(c)? (c) - ('a' - 'A') : (c))
#define toupper(c)  (islower(c)? (c) + ('a' - 'A') : (c))
//__END_DECLS

#endif
