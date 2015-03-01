#include <sys/idt.h>
#include <sys/sbunix.h>

void test_idt(void) {
    printf("sizeof(struct idt_t): %lu\n", sizeof(struct idt_t));
}

#define INTERRUPT 0x0e /* Automatically disable interrupts on entry */
#define TRAP_GATE 0x0f /* Allow interrupts to continue on entry */
#define TASK_GATE 0x05 /* The selector will be a TSS Segement Selector in
                       the GDT. Offset values are not used. Set them to 0. */

#define PRESENT_ISR(selector, ist, type, dpl, target) { \
    (target) & 0x0000ffff, \
    selector, \
    ist, \
    0, \
    type, \
    0, \
    dpl, \
    1, \
    ((target) >> 16) & 0x0000ffff, \
    ((target) >> 32) & 0xffffffff, \
    0 \
    }

#define ABSENT_ISR(selector, ist, type, dpl, target) { \
    0, \
    selector, \
    ist, \
    0, \
    type, \
    0, \
    dpl, \
    0, \
    0, \
    0, \
    0 \
    }

static struct idt_t idt[] = {
    /**
    *  selector: 8, offset to kernel code segment
    *  ist:      0, not used
    *  type:     INTERRUPT
    *  dpl:      0, ring 0
    *  target:   pointer to the ISR
    */
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_000),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_001),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_002),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_003),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_004),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_005),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_006),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_007),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_008),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_009),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_010),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_011), /* NOT PRESENT FAULT */
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_012),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_013), /* GENERAL PROTECTION FAULT */
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_014),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_015),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_016),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_017),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_018),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_019),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_020),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_021),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_022),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_023),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_024),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_025),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_026),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_027),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_028),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_029),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_030),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_031),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_032), /* IRQ  0 */
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_033), /* IRQ  1 */
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_034), /* IRQ  2 */
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_035), /* IRQ  3 */
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_036), /* IRQ  4 */
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_037), /* IRQ  5 */
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_038), /* IRQ  6 */
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_039), /* IRQ  7 */
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_040), /* IRQ  8 */
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_041), /* IRQ  9 */
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_042), /* IRQ 10 */
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_043), /* IRQ 11 */
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_044), /* IRQ 12 */
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_045), /* IRQ 13 */
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_046), /* IRQ 14 */
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_047), /* IRQ 15 */
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_048),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_049),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_050),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_051),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_052),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_053),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_054),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_055),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_056),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_057),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_058),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_059),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_060),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_061),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_062),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_063),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_064),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_065),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_066),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_067),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_068),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_069),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_070),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_071),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_072),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_073),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_074),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_075),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_076),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_077),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_078),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_079),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_080),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_081),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_082),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_083),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_084),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_085),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_086),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_087),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_088),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_089),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_090),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_091),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_092),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_093),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_094),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_095),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_096),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_097),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_098),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_099),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_100),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_101),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_102),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_103),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_104),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_105),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_106),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_107),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_108),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_109),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_110),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_111),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_112),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_113),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_114),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_115),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_116),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_117),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_118),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_119),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_120),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_121),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_122),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_123),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_124),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_125),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_126),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_127),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_128),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_129),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_130),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_131),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_132),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_133),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_134),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_135),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_136),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_137),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_138),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_139),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_140),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_141),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_142),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_143),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_144),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_145),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_146),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_147),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_148),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_149),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_150),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_151),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_152),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_153),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_154),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_155),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_156),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_157),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_158),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_159),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_160),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_161),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_162),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_163),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_164),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_165),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_166),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_167),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_168),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_169),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_170),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_171),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_172),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_173),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_174),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_175),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_176),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_177),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_178),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_179),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_180),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_181),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_182),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_183),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_184),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_185),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_186),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_187),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_188),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_189),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_190),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_191),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_192),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_193),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_194),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_195),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_196),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_197),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_198),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_199),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_200),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_201),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_202),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_203),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_204),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_205),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_206),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_207),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_208),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_209),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_210),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_211),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_212),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_213),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_214),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_215),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_216),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_217),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_218),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_219),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_220),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_221),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_222),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_223),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_224),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_225),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_226),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_227),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_228),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_229),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_230),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_231),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_232),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_233),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_234),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_235),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_236),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_237),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_238),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_239),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_240),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_241),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_242),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_243),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_244),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_245),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_246),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_247),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_248),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_249),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_250),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_251),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_252),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_253),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_254),
    ABSENT_ISR  (8, 0, INTERRUPT , 0, &_isr_255),
};


struct idtr_t {

    uint16_t size;
    uint64_t addr;
}__attribute__((packed));

struct idtr_t idtr = {

        (uint16_t)sizeof(idt),
        (uint64_t)idt,
};

#define SET_ISR(vector) \
    { \
            struct idt_t _idt = PRESENT_ISR(8, 0, INTERRUPT , 0, ((uint64_t)&x86_64_isr_vector ## vector)); \
            idt[vector] = _idt; \
    }

#define DUMMY_INTERRUPT(vector) \
    extern void x86_64_isr_vector ## vector(); \
    void x86_64_isr_vector ## vector() \
    { \
            printf("DUMMY INTERRUPT VECTOR! " #vector "\n"); \
    }

DUMMY_INTERRUPT(0)
DUMMY_INTERRUPT(1)
DUMMY_INTERRUPT(2)
DUMMY_INTERRUPT(3)
DUMMY_INTERRUPT(4)
DUMMY_INTERRUPT(5)
DUMMY_INTERRUPT(6)
DUMMY_INTERRUPT(7)
DUMMY_INTERRUPT(8)
DUMMY_INTERRUPT(9)
DUMMY_INTERRUPT(10)
DUMMY_INTERRUPT(11)
DUMMY_INTERRUPT(12)
DUMMY_INTERRUPT(13)
DUMMY_INTERRUPT(14)
DUMMY_INTERRUPT(15)
DUMMY_INTERRUPT(16)
DUMMY_INTERRUPT(17)
DUMMY_INTERRUPT(18)
DUMMY_INTERRUPT(19)
DUMMY_INTERRUPT(20)
DUMMY_INTERRUPT(21)
DUMMY_INTERRUPT(22)
DUMMY_INTERRUPT(23)
DUMMY_INTERRUPT(24)
DUMMY_INTERRUPT(25)
DUMMY_INTERRUPT(26)
DUMMY_INTERRUPT(27)
DUMMY_INTERRUPT(28)
DUMMY_INTERRUPT(29)
DUMMY_INTERRUPT(30)
DUMMY_INTERRUPT(31)

void _x86_64_load_idt() {

//      for(int vector = 0; vector<256; ++vector)
    //      SET_ISR(vector, PRESENT_ISR(8, 0, INTERRUPT , 0, ((uint64_t)&x86_64_isr_vector999)));

    SET_ISR(0);
    SET_ISR(1);
    SET_ISR(2);
    SET_ISR(3);
    SET_ISR(4);
    SET_ISR(5);
    SET_ISR(6);
    SET_ISR(7);
    SET_ISR(8);
    SET_ISR(9);
    SET_ISR(10);
    SET_ISR(11);
    SET_ISR(12);
    SET_ISR(13);
    SET_ISR(14);
    SET_ISR(15);
    SET_ISR(16);
    SET_ISR(17);
    SET_ISR(18);
    SET_ISR(19);
    SET_ISR(20);
    SET_ISR(21);
    SET_ISR(22);
    SET_ISR(23);
    SET_ISR(24);
    SET_ISR(25);
    SET_ISR(26);
    SET_ISR(27);
    SET_ISR(28);
    SET_ISR(29);
    SET_ISR(30);
    SET_ISR(31);

    _x86_64_asm_lidt(&idtr);
}
