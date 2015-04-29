#include <sbunix/tarfs.h>
#include <sbunix/vfs/vfs.h>

/**
 * Ascii Octal To 64-bit unsigned Integer
 * @octal
 */
uint64_t aotoi(char *optr) {
    uint64_t val = 0;
    if(optr == NULL)
        return 0;
    while(*optr >= '0' && *optr <= '7') {
        val <<= 3; /* multiply by 8 (the base) */
        val += *optr++ - '0';
    }
    return val;
}

/**
 * Short test of aotoi
 */
void test_aotoi(void) {
    char *octals[] = {
            "01234567",
            "055",
            "01",
            "04672",
            "",
    };
    uint64_t ints[] = {
            342391ULL,
            45ULL,
            1ULL,
            2490ULL,
            0ULL
    };
    int i;

    for(i = 0; i < 5; i++) {
        if (ints[i] != aotoi(octals[i])) {
            kpanic("octal %s != decimal %lu!!\n", octals[i], aotoi(octals[i]));
        }
    }
}

