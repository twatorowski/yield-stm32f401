/**
 * @file stdio.c
 *
 * @date 2019-09-13
 * @author twatorowski (tomasz.watorowski@gmail.com)
 *
 * @brief Simple yet functional string print scan utilities
 */

#include <math.h>
#include <float.h>
#include <stdint.h>
#include <stdarg.h>

#include "assert.h"
#include "compiler.h"
#include "util/abs.h"
#include "util/elems.h"
#include "util/minmax.h"
#include "util/stdio.h"
#include "util/fp.h"

/* parameter may be none */
#define PARAMETER_NONE                      -1

/* format specifier flags */
#define FLAGS_NONE                          0x00
#define FLAGS_MINUS                         0x01
#define FLAGS_PLUS                          0x02
#define FLAGS_SPACE                         0x04
#define FLAGS_ZERO                          0x08
#define FLAGS_HASH                          0x10

/* width modifier may be simply positive, non-existent or given by
 * a parameter */
#define WIDTH_NONE                          -1
#define WIDTH_PARAMETER                     -2

/* precision modifier may be simply positive, non-existent or given by
 * a parameter */
#define PRECISION_NONE                      -1
#define PRECISION_PARAMETER                 -2

/* all the available length modifiers */
#define LENGTH_NONE                         0
#define LENGTH_CHAR                         1
#define LENGTH_SHORT                        2
#define LENGTH_LONG                         3
#define LENGTH_LONG_LONG                    4
#define LENGTH_LONG_DOUBLE                  5
#define LENGTH_SIZE_T                       6
#define LENGTH_INTMAX_T                     7
#define LENGTH_PTRDIFF_T                    8

/* types */
#define TYPE_NONE                           0
#define TYPE_INT                            1
#define TYPE_DOUBLE                         2
#define TYPE_STR                            3
#define TYPE_CHAR                           4
#define TYPE_PTR                            5
#define TYPE_PRINT_LENGTH                   6
#define TYPE_PERCENT                        7
#define TYPE_END_OF_STRING                  8

/* modifiers applicable to different types */
#define TYPEMOD_NONE                        0x00
#define TYPEMOD_BASE_FROM_PREFIX            0x01
#define TYPEMOD_UNSIGNED                    0x02
#define TYPEMOD_CAPITAL                     0x04
#define TYPEMOD_HEX                         0x08
#define TYPEMOD_OCTAL                       0x10
#define TYPEMOD_EXP                         0x20
#define TYPEMOD_TRUNC                       0x40
#define TYPEMOD_HEXFLOAT                    0x80

/* format specifier block */
struct fs {
    /* flags */
    int flags;
    /* parameter and width */
    int parameter, width;
    /* precision and length */
    int precision, length;
    /* type and it's modifiers */
    int type, type_modifiers;
    /* next character (used for parsing strings) */
    char next_char;
};

/* digit look-up table */
static char digits[] = "0123456789abcdef";

/* is the character a digit? */
int isdigit(int c)
{
    return c >= '0' && c <= '9';
}

/* is a hex digit? */
int isxdigit(int c)
{
    /* convert to lower case digit */
    c = tolower(c);
    /* return */
    return isdigit(c) || (c >= 'a' && c <= 'f');
}

/* checks if character is a whitespace */
int isspace(int c)
{
    /* if character is one of the following then it is a whitespace */
    return (c == '\t' || c == '\n' || c == '\v' ||
            c == '\f' || c == '\r' || c == ' ');
}

/* to lower case */
int tolower(int c)
{
    /* character is uppercase ?*/
    if (c >= 'A' && c <= 'Z')
        c -= ('A' - 'a');
    /* return converted value */
    return c;
}

/* to upper case */
int toupper(int c)
{
    /* character is uppercase ?*/
    if (c >= 'a' && c <= 'z')
        c -= ('a'- 'A');
    /* return converted value */
    return c;
}


/* process the [parameter] part of the format specifier */
static int fs_process_parameter(const char *format, int *parameter)
{
    /* shorthand for the format string pointer */
    const char *c = format;
    /* parameter value */
    int p = PARAMETER_NONE;

    /* 1st character must be a digit 1-9 */
    if (*c >= '1' || *c <= '9') {
        /* reset the parameter */
        p = 0;
        /* scan the number */
        while (isdigit(*c))
            p = p * 10 + *c - '0', c++;
        /* nope, we dont have a parameter number specified as these must end up
         * with '$' */
        if (*c != '$') {
            p = PARAMETER_NONE, c = format;
        /* dollar sign found, move the pointer */
        } else {
            c++;
        }
    }

    /* store value */
    *parameter = p;
    /* return the numbef of chars consumed */
    return c - format;
}

/* process the [flags] part of the format specifier */
static int fs_process_flags(const char *format, int *flags)
{
    /* shorthand for the format string pointer */
    const char *c = format;
    /* flags value */
    int f = FLAGS_NONE, done;

    /* this loop will be broken by the underlying switch */
    for (done = 0; !done; ) {
        /* switch on different parts of format specifier */
        switch (*c) {
        /* Left-align the output of this placeholder (the default is to
            * right-align the output).*/
        case '-' : {
            f |= FLAGS_MINUS, c++;
        } break;
        /* Prepends a plus for positive signed-numeric types. positive = '+'
            * negative = '-'. (the default doesn't prepend anything in front of
            * positive numbers). */
        case '+' : {
            f |= FLAGS_PLUS, c++;
            /* plus and space cannot exist together. sorry. */
            f &= ~FLAGS_SPACE;
        } break;
        /* Prepends a space for positive signed-numeric types.
            * positive = ' ', negative = '-'. This flag is ignored if the '+'
            * flag exists. */
        case ' ' : {
            if ((f & FLAGS_PLUS) == 0)
                f |= FLAGS_SPACE;
            c++;
        } break;
        /* Prepends zeros for numbers when the width option is specified.
         * (the default prepends spaces). */
        case '0' : {
            f |= FLAGS_ZERO, c++;
        } break;
        /* Alternate form. For 'g' and 'G', trailing zeros are not removed.
            * For 'f', 'F', 'e', 'E', 'g', 'G', the output always contains a
            * decimal point. For 'o', 'x', 'X', or '0', '0x', '0X',
            * respectively, is prepended to non-zero numbers. */
        case '#' : {
            f |= FLAGS_HASH, c++;
        } break;
        /* all other characters do not fit here */
        default : {
            done = 1;
        } break;
        }
    }
    /* store the flags */
    *flags = f;
    /* return the number of characters consumed */
    return c - format;
}

/* process the [width] part of the format specifier */
static int fs_process_width(const char *format, int *width)
{
    /* shorthand for the format string pointer */
    const char *c = format;
    /* width value */
    int w = WIDTH_NONE;

    /* got a digit at 1st place? */
    if (isdigit(*c)) {
        /* reset width */
        w = 0;
        /* scan the number */
        while (isdigit(*c))
            w = w * 10 + *c - '0', c++;
        /* store the value */
        *width = w;
    /* when using the asterisk the width may be passed as parameter */
    } else if (*c == '*') {
        *width = WIDTH_PARAMETER, c++;
    /* no width field given */
    } else {
        *width = WIDTH_NONE;
    }

    /* return the numbef of chars consumed */
    return c - format;
}

/* process the [precision] part of the format specifier */
static int fs_process_precision(const char *format, int *precision)
{
    /* shorthand for the format string pointer */
    const char *c = format;
    /* precision value */
    int p = PRECISION_NONE;

    /* got a precision characteristic '.'? */
    if (*c == '.') {
        /* consume the period */
        c++;
        /* got the digit? */
        if (isdigit(*c)) {
            /* reset precision */
            p = 0;
            /* scan the number */
            while (isdigit(*c))
                p = p * 10 + *c - '0', c++;
        /* precision is given as a parameter */
        } else if (*c == '*') {
            p = PRECISION_PARAMETER, c++;
        }
    }

    /* store the precision */
    *precision = p;
    /* return the numbef of chars consumed */
    return c - format;
}

/* process the [length] part of the format specifier */
static int fs_process_length(const char *format, int *length)
{
    /* shorthand for the format string pointer */
    const char *c = format;
    /* length value */
    int l = LENGTH_NONE;

    /* this loop will be broken by the underlying switch */
    for (int done = 0; !done; ) {
        /* switch on the length encoding letter */
        switch (*c) {
        /* got the 'half' modifier? */
        case 'h' : {
            /* 1st 'h' */
            if (l == LENGTH_NONE) {
                l = LENGTH_SHORT, c++;
            /* already parsed one 'h'? */
            } else if (l == LENGTH_SHORT) {
                l = LENGTH_CHAR, c++, done = 1;
            /* unsupported sequence */
            } else {
                done = 1;
            }
        } break;
        /* got the 'long' modifier? */
        case 'l' : {
            /* 1st 'l' */
            if (l == LENGTH_NONE) {
                l = LENGTH_LONG, c++;
            /* already parsed one 'l'? */
            } else if (l == LENGTH_LONG) {
                l = LENGTH_LONG_LONG, c++, done = 1;
            /* unsupported sequence */
            } else {
                done = 1;
            }
        } break;
        /* got the 'long' modifier for the floating point numbers? */
        case 'L' : {
            /* only one 'L' is supported */
            if (l == LENGTH_NONE) {
                l = LENGTH_LONG_DOUBLE, c++, done = 1;
            /* unsupported sequence */
            } else {
                done = 1;
            }
        } break;
        /* got the 'size_t' modifier? */
        case 'z' : {
            /* only one 'z' is supported */
            if (l == LENGTH_NONE) {
                l = LENGTH_SIZE_T, c++, done = 1;
            /* unsupported sequence */
            } else {
                done = 1;
            }
        } break;
        /* got the 'intmax_t' modifier? */
        case 'j' : {
            /* only one 'j' is supported */
            if (l == LENGTH_NONE) {
                l = LENGTH_INTMAX_T, c++, done = 1;
            /* unsupported sequence */
            } else {
                done = 1;
            }
        } break;
        /* got the 'ptrdiff_t' modifier */
        case 't' : {
            /* only one 't' is supported */
            if (l == LENGTH_NONE) {
                l = LENGTH_PTRDIFF_T, c++, done = 1;
            /* unsupported sequence */
            } else {
                done = 1;
            }
        } break;
        /* unknown specifier */
        default : {
            done = 1;
        } break;
        }
    }

    /* store the length */
    *length = l;
    /* return the number of bytes processed */
    return c - format;
}

/* process type specifier */
static int fs_process_type(const char *format, int *type, int *type_modifiers)
{
    /* shorthand for the format string pointer */
    const char *c = format;
    /* initial values */
    int t = TYPE_NONE, tm = TYPEMOD_NONE;

    /* switch on type letter */
    switch (*c) {
    /* all cases of integers */
    case 'd' : case 'i' : case 'u' : case 'x' : case 'X' : case 'o' : {
        /* these are all integers */
        t = TYPE_INT;
        /* get the type modifier */
        switch (*c) {
        case 'i' : tm = TYPEMOD_BASE_FROM_PREFIX; break;
        case 'u' : tm = TYPEMOD_UNSIGNED; break;
        case 'x' : tm = TYPEMOD_UNSIGNED | TYPEMOD_HEX; break;
        case 'X' : tm = TYPEMOD_UNSIGNED | TYPEMOD_HEX | TYPEMOD_CAPITAL; break;
        case 'o' : tm = TYPEMOD_UNSIGNED | TYPEMOD_OCTAL; break;
        }
        /* eat up the character */
        c++;
    } break;
    /* all cases of doubles */
    case 'f' : case 'F': case 'e' : case 'E' : case 'g' : case 'G' :
    case 'a' : case 'A': {
        /* these are all doubles */
        t = TYPE_DOUBLE;
        /* get the modifier */
        switch (*c) {
        case 'F' : tm = TYPEMOD_CAPITAL; break;
        case 'e' : tm = TYPEMOD_EXP; break;
        case 'E' : tm = TYPEMOD_EXP | TYPEMOD_CAPITAL; break;
        case 'g' : tm = TYPEMOD_TRUNC; break;
        case 'G' : tm = TYPEMOD_TRUNC | TYPEMOD_CAPITAL; break;
        case 'A' : tm = TYPEMOD_HEXFLOAT | TYPEMOD_CAPITAL; break;
        case 'a' : tm = TYPEMOD_HEXFLOAT; break;
        }
        /* eat up the character */
        c++;
    } break;
    /* other types do not have modifiers */
    case 's' : t = TYPE_STR, c++; break;
    case 'c' : t = TYPE_CHAR, c++; break;
    case 'p' : t = TYPE_PTR, c++; break;
    case 'n' : t = TYPE_PRINT_LENGTH, c++; break;
    case '%' : t = TYPE_PERCENT, c++; break;
    /* end of string */
    case '\0': t = TYPE_END_OF_STRING; break;
    }

    /* store gathered information */
    *type = t, *type_modifiers = tm;
    /* return number of processed bytes */
    return c - format;
}

/* fetch the next character, useful for the string scanning */
static int fs_process_next_char(const char *format, char *next_char)
{
    /* this is just simply the next charcter in the string */
    *next_char = *format;
    /* this does not consume any characters */
    return 0;
}

/* process the format specifier */
int fs_process(const char *format, struct fs *fs)
{
    /* shorthand */
    const char *c = format;
    /* the syntax for format specifier is %[parameter$][flags][width]
    * [.precision][length]type. let's consume all of these */
    c += fs_process_parameter(c, &fs->parameter);
    c += fs_process_flags(c, &fs->flags);
    c += fs_process_width(c, &fs->width);
    c += fs_process_precision(c, &fs->precision);
    c += fs_process_length(c, &fs->length);
    c += fs_process_type(c, &fs->type, &fs->type_modifiers);
    c += fs_process_next_char(c, &fs->next_char);

    /* return the total number of chars consumed */
    return c - format;
}

/* prints the char using all provided information */
static int print_char(char *dst, size_t size, const void *val, struct fs *fs)
{
    /* shorthand */
    char *d = dst;
    const char *s = val;
    /* number of characters to scan */
    size_t char_num = 1;

    /* precision tells us how many chars we are to consume */
    if (fs->precision != PRECISION_NONE)
        char_num = (size_t)fs->precision;

    /* still got space? */
    while (size && char_num)
        *d++ = *s, size--, char_num--;

    /* return the number of characters written */
    return d - dst;
}

/* prints the string using all provided information */
static int print_str(char *dst, size_t size,  const void *val, struct fs *fs)
{
    /* shorthands */
    char *d = dst;
    const char *s = val;
    size_t len = size;

    /* got the precision specifier? */
    if (fs->precision != PRECISION_NONE)
        len = fs->precision;
    /* limit max length */
    if (len > size)
        len = size;

    /* print */
    for (; *s && len; len--) *d++ = *s++;
    /* return the number of characters printed */
    return d - dst;
}

/* print integer */
static int print_int(char *dst, size_t size, const void *ptr, struct fs *fs)
{
    /* temporary storage for printed numeric value. it's size will be determined
     * by the longest octal representation as it will have the biggest number of
     * digits. if the long long is contained within 64 bits then the octal
     * representation will have ceil(64/3) digits */
    char num[sizeof(long long) * 8 / 3 + 1 + 2 + 1];
    /* we store the prefix in separate buffer since the padding may be printed
     * in form of spaces before the prefix and zeros after the prefix */
    char prefix[3];
    /* handy pointers */
    char *d = dst, *n = num + sizeof(num), *p = prefix;
    /* length of numeric, length of prefix, numerical system base */
    int n_len = 0, p_len = 0, b = 10, negative = 0;

    /* possible types of input */
    union {
        char c; short s; int i; long l; long long ll;
    } PACKED const *val_ptr = ptr;

    /* let's determine the numerical system base */
    if (fs->type_modifiers & TYPEMOD_HEX)
        b = 16;

    /* not dealing with unsigned numbers? */
    if (!(fs->type_modifiers & TYPEMOD_UNSIGNED)) {
        /* get the negative flag */
        switch (fs->length) {
        case LENGTH_LONG_LONG : negative = val_ptr->ll < 0; break;
        case LENGTH_LONG : negative = val_ptr->l < 0; break;
        /* sign extension for short and char */
        case LENGTH_SHORT : negative = val_ptr->s< 0; break;
        case LENGTH_CHAR : negative = (signed char)val_ptr->c < 0; break;
        default : negative = val_ptr->i < 0; break;
        }
    }

    /* couple of versions to avoid long divisions for shorter lengths */
    /* long long version */
    if (fs->length == LENGTH_LONG_LONG) {
        /* extract the value & print */
        unsigned long long v = negative ? -val_ptr->ll : val_ptr->ll;
        do { *--n = digits[v % b]; v /= b; } while (v);
    /* long version */
    } else if (fs->length == LENGTH_LONG) {
        /* extract the value & print */
        unsigned long v = negative ? -val_ptr->l : val_ptr->l;
        do { *--n = digits[v % b]; v /= b; } while (v);
    /* standard version for ints */
    } else {
        /* need both signed and unsi*/
        unsigned int v = val_ptr->i;
        /* sign extend/mask out if needed */
        switch (fs->length) {
        case LENGTH_SHORT : v = negative ? -(short)v : (unsigned short)v; break;
        case LENGTH_CHAR : v = negative ? -(char)v : (unsigned char)v; break;
        default : v = negative ? -v : v; break;
        }
        /* print */
        do { *--n = digits[v % b]; v /= b; } while (v);
    }
    /* set the final length of the number */
    n_len = num + sizeof(num) - n;

    /* add minus sign to the prefix */
    /* time to render the prefix [- +]|[0]|[0x] */
    if (negative) {
        *p++= '-';
    /* the number is signed positive */
    } else if (!(fs->type_modifiers & TYPEMOD_UNSIGNED)) {
        /* print plus sign for positive signed numbers?*/
        if (fs->flags & FLAGS_PLUS) {
            *p++ = '+';
        /* print space for positive numbers? */
        } else if (fs->flags & FLAGS_SPACE) {
            *p++ = ' ';
        }
    /* got a hex prefix request? */
    } else if (fs->flags & FLAGS_HASH && b == 16) {
        *p++ = '0', *p++ = 'x';
    }
    /* set the final length of the prefix */
    p_len = p - prefix;

    /* leading zeros length leading space length */
    int lz_len = 0, ls_len = 0;
    /* in case of integers precision tells us how many digits are used to
     * represent the number (but no less than the integer value would imply) */
    if (fs->precision != PRECISION_NONE)
        lz_len = max(fs->precision - n_len, 0);
    /* width tells us the minimal print length */
    if (fs->width != WIDTH_NONE) {
        /* ..and if zero flag is set then we prepend the numeric value with
         * zeros */
        if ((fs->flags & FLAGS_ZERO) && fs->precision == PRECISION_NONE) {
            lz_len = max(fs->width - (n_len + p_len), 0);
        /* prepend with space */
        } else {
            ls_len = max(fs->width - (n_len + p_len + lz_len), 0);
        }
    }

    /* this is the complete print length */
    size_t print_len = ls_len + p_len + lz_len + n_len;
    /* ...but not everything has to fit! */
    if (print_len > size) {
        /* ...so start discarding from the end */
        n_len -= print_len - size;
        /* ...and back-propagate! */
        if (n_len  < 0) lz_len += n_len, n_len = 0;
        if (lz_len < 0) p_len += lz_len, lz_len = 0;
        if (p_len < 0) ls_len += p_len, lz_len = 0;
    }

    /* print everything */
    for (int i = 0; i < ls_len; i++) *d++ = ' ';
    for (int i = 0; i <  p_len; i++) *d++ = prefix[i];
    for (int i = 0; i < lz_len; i++) *d++ = '0';
    for (int i = 0; i <  n_len; i++) *d++ = *n++;
    /* return the number of printed characters */
    return d - dst;
}

/* print floating point number */
static int print_double(char *dst, size_t size, const void *ptr, struct fs *fs)
{
    /* scratch-pad buffers for the value and optional exponent */
    char nbuf[48] = {0}, *nptr = nbuf;
    /* exponent buffer */
    char ebuf[8] = {0}, *eptr = ebuf;
    /* prefix buffer */
    char pbuf[8] = {0}, *pptr = pbuf;

    /* binary logarithm of 10 or 16, natural logarithm of 10  or 16: these are
     * the two bases that we support */
    static const float lb10 = 3.321928095, ln10 = 2.302585093;
    static const float lb16 = 4.000000000, ln16 = 2.772588722;
    /* powers of base 10 */
    static const float p10[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
        100000000 };
    /* powers of base 16 */
    static const float p16[] = { 0x1, 0x10, 0x100, 0x1000, 0x10000, 0x100000,
        0x1000000 };

    /* in this implementation we only use float */
    float value = fs->length == LENGTH_LONG_DOUBLE ? *(long double *)ptr :
        *(double *)ptr;

    /* negate the sign if the number is negative. its much easier to deal with
     * posititive numbers and then prepend these with minus sign */
    int minus = value < 0; value = fp_fabsf(value);
    /* sign needs to be printed? */
    int sign = (fs->flags & FLAGS_PLUS) || (fs->flags & FLAGS_SPACE) || minus;
    /* use space instead of the plus sign? */
    int sign_space = fs->flags & FLAGS_SPACE;

    /* use the scientific format? */
    int scientific = !!(fs->type_modifiers & TYPEMOD_EXP);
    /* requested precision */
    int precision = fs->precision == PRECISION_NONE ? 4 : fs->precision;
    /* in which base are we about to print? */
    int base = fs->type_modifiers & TYPEMOD_HEXFLOAT ? 16 : 10;

    /* special numbers are: nan, inf */
    int special = 0;

    /* let's support the special numbers */
    if (fp_isinf(value)) {
        /* setup the number */
        special = 1, *nptr++ = 'f', *nptr++= 'n', *nptr++ = 'i';
        /* prefix print will handle the sign */
        goto prefix;
    /* not a number */
    } else if (fp_isnan(value)) {
        /* setup the representation */
        special = 1, *nptr++ = 'n', *nptr++= 'a', *nptr++ = 'n';
        /* no need to generate prefix for the nan */
        goto print;
    }

    /* select appropriate constants for given base */
    float ln_base = base == 10 ? ln10 : ln16;
    float lb_base = base == 10 ? lb10 : lb16;

    /* convert the number to format value = man * 2^exp2 */
    int exp2; float man2 = fp_frexpf(value, &exp2);

    /* convert exponent of two to exponent of base so that we have
     * value = man * (base)^exp_base */
    float exp_base = (float)exp2 / lb_base;
    /* let's split the exponent into integer and fractional part*/
    float _expb_i; float exp_base_f = fp_modff(exp_base, &_expb_i);
    /* store the integer part as an actual integer */
    int exp_base_i = _expb_i;

    /* since the exp_base is likely to be a number with fractional number we
     * need to compute man_base = base ^ exp_base_f in order to represent the
     * number as value = base ^ exp_base_i * base ^ exp_base_f * man. The part
     * "base ^ exp_base_f * man" will be used to produce digits  and the part
     * "base ^ exp_base_i" will be used to tell where the decimal point shall be
     * located. Here we use the trick: base^x = e^(x*ln(base)) */
    float man_base = man2 * fp_expf(exp_base_f * ln_base);
    /* normalize mantissa, if it's greater than unity then divide by the base in
     * order to keep it within [1/base , 1) */
    if (man_base >= 1.0f) {
        man_base /= base; exp_base_i++;
    }

    /* select the base power table */
    const float *p_base = base == 10 ? p10 : p16;
    /* derive the array length */
    int pb_len = base == 10 ? elems(p10) : elems(p16);

    /* compute how many digits the mantissa shall have according to specified
     * representation. */
    int mantissa_digits = (scientific ? 1 : exp_base_i) + precision;
    /* limit the amount of digits to the length of base power table */
    mantissa_digits = min(mantissa_digits, pb_len - 2);
    /* produce mantissa digits in integer form by multiplying by the power table
     * entry. Add one digit to allow the rounding to happen */
    int man_base_i = man_base * p_base[mantissa_digits + 1];
    /* do the rounding */
    man_base_i = (man_base_i + base / 2) / base;
    /* we may have produced another digit during rounding like in case where
     * 99.999 gets rounder to 100.00 when only two decimal places are to be
     * produced. to check let's compare against the base power table entry */
    if (man_base_i >= p_base[mantissa_digits]) {
        /* we can drop the last digit without any loss of precision since it
         * must be 0 if overflow happened */
        man_base_i /= base; exp_base_i += 1;
    }

    /* get the decimal position of least significant digit stored in mantissa */
    int man_spos = exp_base_i - mantissa_digits;
    /* get the starting position of the number that we are about to print */
    int num_spos = scientific ? man_spos : -precision;
    /* get the position of the most significant digit that we are about to print */
    int num_epos = scientific ? exp_base_i : max(1, exp_base_i);
    /* get the decimal point position */
    int dec_pos = scientific ? num_epos - 1 : 0;

    /* sanity check */
    assert(num_epos - num_spos < sizeof(nbuf), "nbuf is too small");

    /* time to render the mantissa */
    for (int pos = num_spos; pos < num_epos; pos++) {
        /* decimal point */
        if (pos == dec_pos && pos != num_spos)
            *nptr++ = '.';
        /* meaningful digits */
        if (pos >= man_spos && man_base_i) {
            *nptr++ = digits[man_base_i % base], man_base_i /= base;
        /* leading and trailing zeros */
        } else {
            *nptr++ = digits[0];
        }
    }

    /* scientific format ends up with the exponent value */
    if (scientific) {
        /* render the exponent */
        for (int pos = 0, exp = abs(exp_base_i - 1) ; pos < 2 || exp; pos++)
            *eptr++ = digits[exp % 10], exp /= 10;
        /* append sign and proper exponent encoding character */
        *eptr++ = exp_base_i < 1 ? '-': '+'; *eptr++ = base == 10 ? 'e' : 'p';
    }

    /* build up the prefix */
    prefix:
    /* print sign if requested */
    if (sign)
        *pptr++ = minus ? '-' : (sign_space ? ' ': '+');
    /* print hex prefix if the number is in hex base and it's not nan nor inf */
    if (base == 16 && !special)
        *pptr++ = '0', *pptr++ = 'x';


    /* start printing to the destination bufer */
    print: do {} while(0);
    /* we are now ready to print the whole number into the destination buffer,
     * create some shorthands */
    char *s, *d = dst, *dend = dst + size;
    /* get the number of characters that constitute the number */
    int number_len = (pptr - pbuf) + (eptr - ebuf) + (nptr - nbuf);
    /* these shall hold the number of leading spaces and zeros if the width of
     * the print was specified */
    int lspace_len = 0, tspace_len = 0, zeros_len = 0;
    /* see if we have the width specified, if so we may need to add preceeding
     * space or leading zeros */
    if (fs->width != WIDTH_NONE) {
        /* get the number of bytes left */
        int width_left = max(fs->width - number_len, 0);
        /* limit the width */
        dend = dst + fs->width;
        /* left justification, done using spaces */
        if (fs->flags & FLAGS_MINUS) {
            tspace_len = width_left;
        /* right justification done with leading zeros */
        } else if ((fs->flags & FLAGS_ZERO) && !special) {
            zeros_len = width_left;
        /* default fallback: right justification done using leading spaces */
        } else {
            lspace_len = width_left;
        }
    }

    /* print leading space, prefix and leading zeros */
    for (        ; d != dend && lspace_len; *d++ = ' ', lspace_len--);
    for (s = pbuf; d != dend && s != pptr ; *d++ = *s++);
    for (        ; d != dend && zeros_len ; *d++ = '0', zeros_len--);
    /* print number & exponent */
    for (s = nptr; d != dend && s != nbuf ; *d++ = *--s);
    for (s = eptr; d != dend && s != ebuf ; *d++ = *--s);
    /* print trailing space */
    for (        ; d != dend && tspace_len; *d++ = ' ', tspace_len--);

    /* return the number of characters produced */
    return d - dst;
}

/* scan for end of string */
static int scan_eos(const char *str, size_t size, void *val, struct fs *fs)
{
    /* source pointer */
    const char *s = str;
    /* number of characters to scan */
    size_t char_num = 1;

    /* precision tells us how many chars we are to consume */
    if (fs->precision != PRECISION_NONE)
        char_num = (size_t)fs->precision;
    /* limit imposed by size */
    if (size < char_num)
        char_num = size;

    /* no output is required */
    while (char_num && isspace(*s))
        s++;

    /* return the number of characters consumed or error */
    return *s == 0 ? s - str : -1;
}

/* scan character(s) */
static int scan_char(const char *str, size_t size, void *val, struct fs *fs)
{
    /* destination pointer */
    char *d = val;
    /* source pointer */
    const char *s = str;
    /* number of characters to scan */
    size_t char_num = 1;

    /* precision tells us how many chars we are to consume */
    if (fs->precision != PRECISION_NONE)
        char_num = (size_t)fs->precision;

    /* eat up the characters */
    if (val) {
        while (size && char_num && s)
            *d++ = *s++, char_num--, size--;
    /* no output is required */
    } else {
        while (size && char_num && s)
            s++, char_num--, size--;
    }

    /* return the number of characters consumed */
    return char_num == 0 ? s - str : -1;
}

/* scan a string till a whitespace occurs */
static int scan_str(const char *str, size_t size, void *val, struct fs *fs)
{
    /* destination pointer */
    char *d = val;
    /* source pointer */
    const char *s = str;
    /* max length of the string */
    size_t max_len, len = 0;
    /* in the quotes mark, escaping backslash */
    int in_quotes = 0, esc_backslash = 0, store = 0;

    /* skip over the initial whitespace */
    for (; size && *s && isspace(*s); s++, size--);
    /* do we have the size limited version? */
    if (fs->precision != PRECISION_NONE) {
        max_len = min((size_t)max(fs->precision - 1, 0), size);
    /* use the size as the limiting factor */
    } else {
        max_len = size;
    }

    /* process as long as there are letters */
    for (; *s; s++, len++) {
        /* clear store flag */
        store = -1;

        /* got an escaped character? */
        if (esc_backslash) {
            /* got the ending character? */
            if (*s == fs->next_char && !in_quotes)
                break;
            store = *s, esc_backslash = 0;
        /* got a backslash? */
        } else if (*s == '\\') {
            esc_backslash = 1;
        /* got the quotation mark? */
        } else if (*s == '"') {
            /* we were already within the quotation marks so that means that
                * this is the ending quotation mark break the processing loop */
            if (in_quotes) {
                in_quotes = 0; s++; break;
            /* quotation mark is the opening character */
            } else if (len == 0) {
                in_quotes = 1;
            /* ending character */
            } else if (*s == fs->next_char) {
                break;
            /* treat as the normal character */
            } else {
                store = *s;
            }
        /* white-space is consumed if we are in quotes */
        } else if (isspace(*s) || *s == fs->next_char) {
            /* ....but we were not in quotes! */
            if (!in_quotes) {
                break;
            } else {
                store = *s;
            }
        /* all other characters go here */
        } else {
            store = *s;
        }

        /* processing indicates that the value needs to be stored? */
        if (store != -1 && val && max_len)
            *d++ = store, max_len -= 1;
    }

    /* zero terminate */
    if (val)
        *d = '\0';

    /* return the number of scanned characters */
    return s - str;
}

/* scan the integer */
static int scan_int(const char *str, size_t size, void *val, struct fs *fs)
{
    /* source pointer */
    const char *s = str;
    /* possible outputs */
    union {
        /* all of these may be produced */
        char c; short s; int i; long l; long long ll;
    } PACKED *d = val;

    /* processing states */
    enum states {
        SIGN, PREFIX, INT, END, ERR
    } state = SIGN;

    /* shorthand for the type modifier */
    int tm = fs->type_modifiers;
    /* integer base */
    int base = 10, sign = 0, digit_num = 0;
    /* working buffer */
    char buf[30];
    /* length of the data in the buffer */
    int buf_len = 0;

    /* skip over the initial whitespace */
    for (; size && *s && isspace(*s); s++, size--);
    /* now let's apply the precision limit */
    if (fs->precision != PRECISION_NONE)
        size = min((size_t)fs->precision, size);

    /* set the base value according to typemodifiers */
    switch (tm & (TYPEMOD_HEX | TYPEMOD_OCTAL | TYPEMOD_BASE_FROM_PREFIX)) {
    case TYPEMOD_HEX : base = 16; break;
    case TYPEMOD_OCTAL: base = 8; break;
    case TYPEMOD_BASE_FROM_PREFIX : base = 0; break;
    default: base = 10; break;
    }

    /* process the float state machine */
    while (state != END && state != ERR) {
        /* convert to lower-case */
        char c = tolower(*s);
        /* get the digit value */
        int v = isdigit(c) ? c - '0' : c - 'a' + 10;
        /* move the pointer by this many positions within the input string */
        int advance = 0, eos = !c || !size;

        /* integer accepting fsm */
        switch (state) {
        /* process sign in front of the number */
        case SIGN: {
            /* whoa! that ended up quickly! */
            if (eos) {
                state = ERR;
            } else {
                /* consume plus or minus sign */
                if (!eos && (c == '+' || c == '-'))
                    advance = 1, sign = c == '+' ? 1 : -1;
                /* go to the next state */
                state = tm & (TYPEMOD_HEX | TYPEMOD_BASE_FROM_PREFIX) ?
                    PREFIX : INT;
            }
        } break;
        /* process prefix (only for hex or base from prefix modes) */
        case PREFIX : {
            /* end of string */
            if (eos) {
                advance = -buf_len, buf_len = 0, state = INT;
                /* no prefix indicated the base, let's resort to 10 */
                if (!base)
                    base = 10;
            } else {
                /* append to buffer */
                buf[buf_len++] = c, advance = 1;
                /* all prefixes are two bytes long */
                if (buf_len == 2) {
                    /* prefix determined base */
                    int prefix_base = 0;
                    /* hex */
                    if (buf[0] == '0' && buf[1] == 'x') {
                        prefix_base = 16;
                    /* binary */
                    } else if (buf[0] == '0' && buf[1] == 'b') {
                        prefix_base = 2;
                    }
                    /* no match, go backwards */
                    if (!prefix_base || (base && prefix_base != base))
                        advance -= buf_len;
                    /* get the base from the prefix or assume it's 10 */
                    if (!base)
                        base = prefix_base ? prefix_base : 10;
                    /* clear buffer, advance state */
                    buf_len = 0, state = INT;
                }
            }
        } break;
        /* process the numeric part */
        case INT : {
            /* end of string */
            if (eos) {
                state = digit_num ? END : ERR;
            /* store digits unsigned comparison test if the v is a digit */
            } else if ((unsigned)base > (unsigned)v) {
                /* update the digit counter */
                digit_num++;
                /* skip leading zeros */
                if (v || buf_len) {
                    /* still got some space? */
                    if ((size_t)buf_len < sizeof(buf))
                        buf[buf_len++] = v;
                }
                /* advance one byte */
                advance = 1;
            } else if (digit_num) {
                state = END;
            /* not a single digit got processed? */
            } else {
                state = ERR;
            }
        } break;
        /* all other fall here */
        default : break;
        }
        /* update the position and size */
        s += advance, size -= advance;
    }

    /* no need to process the number, value will be discarded anyway? */
    if (!val || state == ERR)
        goto end;
    /* final number accumulator */
    unsigned long long ull = 0;
    /* build up the number */
    for (int i = 0; i < buf_len; i++)
        ull = ull * base + buf[i];
    /* apply sign */
    if (sign == -1)
        ull = -ull;
    /* convert to valid type */
    switch (fs->length) {
    case LENGTH_LONG_LONG : d->ll = ull; break;
    case LENGTH_LONG : d->l = ull; break;
    case LENGTH_SHORT : d->s = ull; break;
    case LENGTH_CHAR : d->l = ull; break;
    default : d->i = ull; break;
    }

    /* return the number of the bytes consumed or error */
    end: return state == ERR ? -1 : s - str;
}

/* scan the floating point number */
static int scan_double(const char *str, size_t size, void *val, struct fs *fs)
{
    /* source pointer */
    const char *s = str;
    /* possible outputs */
    union {
        /* all of these may be produced */
        float f; double d; long double ld;
    } PACKED *d = val;

    /* procesing states */
    enum states {
        SIGN, SPECIAL, PREFIX, INT, FRAC, EXP_SIGN, EXP_INT, END, ERR
    } state = SIGN;

    /* shorthand for the type modifier */
    int tm = fs->type_modifiers;
    /* sign, base, exponent part sign and value */
    int sign = 0, base = 10, exp_sign = 0, exp_val = 0, e = 0;
    /* special numbers flags current buffer length */
    int is_nan = 0, is_inf = 0, buf_len = 0, digit_num = 0;
    /* working buffer */
    char buf[64];
    /* final computation result */
    double result = 0;

    /* skip over the initial whitespace */
    for (; size && *s && isspace(*s); s++, size--);
    /* now let's apply the precision limit */
    if (fs->precision != PRECISION_NONE)
        size = min((size_t)fs->precision, size);

    /* process the float state machine */
    while (state != END && state != ERR) {
        /* convert to lower-case */
        char c = tolower(*s);
        /* get the digit value */
        int v = isdigit(c) ? c - '0' : c - 'a' + 10;
        /* move the pointer by this many positions within the input string */
        int advance = 0, eos = !c || !size;

        /* float accepting state machine ;-) */
        switch (state) {
        /* process sign */
        case SIGN : {
            /* consume plus or minus sign */
            if (!eos && (c == '+' || c == '-'))
                advance = 1, sign = c == '+' ? 1 : -1;
            /* go to the next state */
            state = SPECIAL;
        } break;
        /* catch all special numbers like 'inf' or 'nan' */
        case SPECIAL : {
            /* end of string? */
            if (eos) {
                advance = -buf_len, buf_len = 0, state = PREFIX;
            /* still got the data */
            } else {
                /* buffer up! */
                buf[buf_len++] = c, advance = 1;
                /* inf case */
                if (buf_len == 3 && buf[0] == 'i' && buf[1]== 'n' &&
                    buf[2] == 'f') {
                    is_inf = 1, state = END;
                /* nan case */
                } else if (buf_len == 3 && buf[0] == 'n' && buf[1] == 'a' &&
                           buf[2] == 'n') {
                    is_nan = 1, state = END;
                /* this is not a special number */
                } else if (buf_len == 3) {
                    advance -= buf_len, buf_len = 0, state = PREFIX;
                }
            }
        } break;
        /* prefix */
        case PREFIX : {
            /* prefix is allowed only for hexfloat numbers */
            if (!(tm &TYPEMOD_HEXFLOAT)) {
                state = INT;
            /* end of string? */
            } else if (eos) {
                advance = -buf_len, buf_len = 0, state = INT;
            /* prefix is allowed, so check for it */
            } else {
                /* store data */
                buf[buf_len++] = c, advance = 1;
                /* enough data buffered? */
                if (buf_len == 2) {
                    /* got a valid hexfloat prefix? */
                    if (buf[0] == '0' && buf[1] == 'x') {
                        base = 16;
                    /* invalid prefix */
                    } else {
                        advance -= buf_len;
                    }
                    /* clear the buffer and move forward */
                    buf_len = 0, state = INT;
                }
            }
        } break;
        /* parse the integer/fractional part */
        case FRAC :
        case INT : {
            /* end of string */
            if (eos) {
                state = END;
            /* store digits unsigned comparison test if the v is a digit */
            } else if ((unsigned)base > (unsigned)v) {
                /* update the digit counter */
                digit_num++;
                /* skip leading zeros and  */
                if (v || buf_len) {
                    /* still got some space? */
                    if ((size_t)buf_len < sizeof(buf))
                        buf[buf_len++] = v;
                    /* update the exponent */
                    if (state == INT)
                        e++;
                } else if (state == FRAC) {
                    e--;
                }
                /* advance one byte */
                advance = 1;
            /* separator? */
            } else if (state != FRAC && c == '.') {
                state = FRAC, advance = 1;
            /* start of the exponent? */
            } else if (((base == 10 && c == 'e' &&
                        (tm & (TYPEMOD_EXP | TYPEMOD_HEXFLOAT))) ||
                        (base == 16 && c == 'p')) &&
                        (digit_num != 0)) {
                state = EXP_SIGN, advance = 1;
            /* unknown character */
            } else if (digit_num) {
                state = END;
            /* no digits were provided */
            } else {
                state = ERR;
            }
        } break;
        /* parse the exponent sign */
        case EXP_SIGN : {
            /* consume plus or minus sign */
            if (!eos && (c == '+' || c == '-'))
                advance = 1, exp_sign = c == '+' ? 1 : -1;
            /* parse the exponent */
            state = EXP_INT, digit_num = 0;
        } break;
        /* parse the exponent value */
        case EXP_INT : {
            /* suck all the decimal digits! */
            if (!eos && isdigit(c)) {
                exp_val = exp_val * 10 + v, advance = 1, digit_num++;
            /* end of the number */
            } else if (digit_num) {
                state = END;
            /* unable to complete the parsing of the exponent when the format
             * needed it! */
            } else if (tm & TYPEMOD_EXP) {
                state = ERR;
            /* unable to parse the exponent but it was optional for the format
             * so no harm done! */
            } else {
                /* fall back */
                advance = exp_sign ? -2 : -1;
                state = END;
            }
        } break;
        /* all others fall here */
        default: break;
        }
        /* update the position and size */
        s += advance, size -= advance;
    }

    /* no need to process the number, value will be discarded anyway? */
    if (!val || state == ERR)
        goto end;

    /* handle not a numbers */
    if (is_nan) {
        result = NAN;
    /* handle infinities */
    } else if (is_inf) {
        result = sign == -1 ? -INFINITY : +INFINITY;
    /* normal number with at least one digit */
    } else if (buf_len != 0) {
        /* get the final exponent value that is the result of decimal separator
        * placement and explicit exponent in scientific/hexfloat notations */
        e = e + (exp_sign == -1 ? -exp_val : +exp_val);
        /* since we'll be summing from the least significant digits we need to
         * re-adjust the exponent */
        e -= buf_len;

        /* power value for the least significant digit */
        double p = base, base_e = 1;
        /* do we need to invert the base? */
        int invert = e < 0;

        /* compute base ^ e */
        for (unsigned int e_abs = (e < 0 ? -e : e); e_abs; p *= p, e_abs >>= 1)
            if (e_abs & 1)
                base_e *= p;
        /* invert the base power */
        if (invert)
            base_e = 1.0 / base_e;
        /* let's process the digit values */
        for (int i = buf_len - 1; i >= 0; i--)
            result += buf[i] * base_e, base_e *= base;
        /* finally multiply by sign if negative */
        if (sign == -1)
            result = -result;
    }

    /* store the result with valid conversion */
    switch (fs->length) {
    case LENGTH_LONG_DOUBLE: d->ld = result; break;
    case LENGTH_LONG : d->d = result; break;
    default : d->f = result; break;
    }

    /* return the number of chars consumed or a negative number that indicates
     * the error */
    end: return state == ERR ? -1 : s - str;
}

/* safe version of the vsprintf */
int vsnprintf(char * str, size_t size, const char *format, va_list args)
{
    /* shorthand */
    char *s = str; const char *fmt = format;
    /* format specifier placeholder */
    struct fs fs;

    /* actual size is less by one since we need to put the trailing zero in the
     * end of the string, if the size is 0 then something is clearly wrong as we
     * won't be able to even put the trailing zero */
    size_t _size = size > 0 ? size - 1 : 0;

    /* process the format string */
    while (*fmt) {
        /* not a format specifier? */
        if (*fmt != '%') {
            /* copy character if there is space */
            if (_size)
                *s++ = *fmt, _size--;
            /* move the pointer no matter if the character was written */
            fmt++;
            /* no point in further processing as this was a normal character,
             * not a formatting one */
            continue;
        }

        /* consume the initial '%' */
        fmt++;
        /* build up an format specifier block */
        fmt += fs_process(fmt, &fs);

        /* precision and width may be given in parameters and not as the
         * explicit values, so we need to fetch them. Width comes first. */
        if (fs.width == WIDTH_PARAMETER) {
            fs.width = va_arg(args, int);
            /* width may be given as the negative number meaning that the user
             * wants the left justified output */
            if (fs.width < 0)
                fs.width = -fs.width, fs.flags |= FLAGS_MINUS;
        }

        /* precission is simply read from the parameter, no quirks here */
        if (fs.precision == PRECISION_PARAMETER)
            fs.precision = va_arg(args, int);

        /* number of characters printed */
        int chars_printed = 0;
        /* let's do the actual printing */
        switch (fs.type) {
        /* print character */
        case TYPE_CHAR : {
            /* get the argument */
            char val = (char)va_arg(args, int);
            /* process */
            chars_printed = print_char(s, _size, &val, &fs);
        } break;
        /* print string */
        case TYPE_STR : {
            /* get the argument */
            char * val = va_arg(args, char *);
            /* process */
            chars_printed = print_str(s, _size, val, &fs);
        } break;
        /* print integers */
        case TYPE_INT : {
            /* possible parameter types */
            union {
                int i; long l; long long ll;
            } v;
            /* get the argument */
            switch (fs.length) {
            case LENGTH_LONG_LONG : v.ll = va_arg(args, long long); break;
            case LENGTH_LONG : v.l = va_arg(args, long); break;
            default : v.i = va_arg(args, int); break;
            }
            /* process */
            chars_printed = print_int(s, _size, &v, &fs);
        } break;
        /* print floating point numbers */
        case TYPE_DOUBLE : {
            /* possible parameter types */
            union {
                double d; long double ld;
            } v;
            /* get the argument */
            switch (fs.length) {
            case LENGTH_LONG_DOUBLE : v.ld = va_arg(args, long double); break;
            default : v.d = va_arg(args, double); break;
            }
            /* process */
            chars_printed = print_double(s, _size, &v, &fs);
        } break;
        /* print percent sign */
        case TYPE_PERCENT : {
            /* 1 is for a single '%' being printed here */
            chars_printed = 1;
            /* store in output memory */
            if (_size)
                *s = '%';
        } break;
        }

        /* update the pointers */
        _size -= chars_printed, s += chars_printed;
    }

    /* release the main copy of the list */
    va_end(args);
    /* got space for trailing zero? */
    if (size > 0)
        *s = '\0';
    /* return the number of characters printed */
    return s - str;
}

/* safe version of the vscanf function */
int vsnscanf(const char *str, size_t size, const char *format,
    va_list args)
{
    /* shorthand */
    const char *fmt = format;
    /* format specifier placeholder */
    struct fs fs;
    /* number of matches */
    int matches = 0;

    /* process the format string */
    while (*fmt) {
        /* not a format specifier? */
        if (*fmt != '%') {
            /* consume all the whitespaces */
            if (isspace(*fmt)) {
                /* consume all whitespaces */
                while (size && isspace(*str))
                    str++, size--;
                fmt++; continue;
            /* got a match of normal characters */
            } else if (size && *fmt == *str) {
                fmt++, str++, size--; continue;
            /* strings differ */
            } else {
                break;
            }
        }

        /* consume the initial '%' */
        fmt++;
        /* build up an format specifier block */
        fmt += fs_process(fmt, &fs);

        /* precision given in parameter? */
        if (fs.precision == PRECISION_PARAMETER)
            fs.precision = va_arg(args, int);

        /* data storage pointer */
        void *ptr = 0;
        /* these options do not use the parameter */
        if ((fs.type != TYPE_PERCENT) &&
            (fs.type != TYPE_END_OF_STRING) &&
            (fs.width != WIDTH_PARAMETER)) {
            /* get the pointer to destination data storage */
            ptr = va_arg(args, void *);
        }

        /* number of characters consumed */
        int chars_consumed = 0;
        /* switch on type of requested data */
        switch (fs.type) {
        /* char or array of chars */
        case TYPE_CHAR : {
            chars_consumed = scan_char(str, size, ptr, &fs);
        } break;
        /* string */
        case TYPE_STR : {
            chars_consumed = scan_str(str, size, ptr, &fs);
        } break;
        /* integers of different types */
        case TYPE_INT : {
            chars_consumed = scan_int(str, size, ptr, &fs);
        } break;
        /* floating point numbers */
        case TYPE_DOUBLE : {
            chars_consumed = scan_double(str, size, ptr, &fs);
        } break;
        /* percent sign */
        case TYPE_PERCENT : {
            /* match againts the percent sign */
            chars_consumed = *str != '%' ? -1 : 1;
        } break;
        /* end of string */
        case TYPE_END_OF_STRING : {
            /* match againts the end of string */
            chars_consumed = scan_eos(str, size, ptr, &fs);
        } break;
        }
        /* got a match? */
        if (chars_consumed >= 0) {
            /* account for match */
            if (fs.type != TYPE_PERCENT)
                matches++;
            /* update pointers */
            str += chars_consumed, size -= chars_consumed;
        /* mismatch occurred, no point in further processing */
        } else {
            break;
        }
    }
    /* return the number of matches */
    return matches;
}

/* simple snprintf */
int snprintf(char *out, size_t size, const char *fmt, ...)
{
    /* variable arguments list */
    va_list args;
    /* string length */
    int len;

    /* map the list */
    va_start(args, fmt);
    /* process the string */
    len = vsnprintf(out, size, fmt, args);
    /* drop argument list */
    va_end(args);

    /* report length */
    return len;
}

/* simple sprintf */
int sprintf(char *out, const char *fmt, ...)
{
    /* variable arguments list */
    va_list args;
    /* string length */
    int len;

    /* map the list */
    va_start(args, fmt);
    /* process the string */
    len = vsnprintf(out, SIZE_MAX, fmt, args);
    /* drop argument list */
    va_end(args);

    /* report length */
    return len;
}

/* simple snscanf */
int snscanf(const char *str, size_t size, const char *format, ...)
{
    /* variable arguments list */
    va_list args;
    /* number of elements matched */
    int matches;

    /* map the list */
    va_start(args, format);
    /* process the string */
    matches = vsnscanf(str, size, format, args);
    /* drop argument list */
    va_end(args);

    /* report length */
    return matches;
}

/* simple snscanf */
int sscanf(const char *str, const char *format, ...)
{
    /* variable arguments list */
    va_list args;
    /* number of elements matched */
    int matches;

    /* map the list */
    va_start(args, format);
    /* process the string */
    matches = vsnscanf(str, SIZE_MAX, format, args);
    /* drop argument list */
    va_end(args);

    /* report length */
    return matches;
}