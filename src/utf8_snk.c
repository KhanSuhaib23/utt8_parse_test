#include <stdint.h>

typedef enum {
State_Reject = 0,
State_Accept = 6,
State_Cb_Rem_1 = 12, // continuation bytes remaining = 1
State_Cb_Rem_2 = 18,
State_Cb_Rem_3 = 24
} State;

static void* utf8_decode_snk(void *buf, uint32_t *c, uint32_t *e) {
    // mapping top 5 bit values to class of
    // 0xxxx -> 0 to 15 -> 1
    // 10xxx -> 16 to 23 -> 0
    // 110xx -> 24 to 27 -> 2
    // 1110x -> 28 to 29 -> 3
    // 11110 -> 30 -> 4
    // 11111 -> 32 -> 5
    static const uint8_t byte_class[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0,
        2, 2, 2, 2,
        3, 3,
        4,
        5
    };
    static const uint8_t masks[]  = {0x00, 0x7f, 0x1f, 0x0f, 0x07, 0x00 };
    static const uint32_t mins[] = {4194304, 0, 128, 2048, 65536, 0};
/*
**                 10xxx, 0xxxx, 110xxh, 1110x, 11110, 11111
**   reject
**   accept
**   cb rem 1
**   cb rem 2
**   cb rem 3
     */
    static const uint8_t state_transition[] = {
        State_Reject,   State_Accept, State_Cb_Rem_1, State_Cb_Rem_2, State_Cb_Rem_3, State_Reject,
        State_Reject,   State_Accept, State_Cb_Rem_1, State_Cb_Rem_2, State_Cb_Rem_3, State_Reject,
        State_Accept,   State_Reject, State_Reject, State_Reject, State_Reject, State_Reject,
        State_Cb_Rem_1, State_Reject, State_Reject, State_Reject, State_Reject, State_Reject,
        State_Cb_Rem_2, State_Reject, State_Reject, State_Reject, State_Reject, State_Reject
    };

    uint8_t* b = (uint8_t*) buf;
    State state = State_Accept;

    uint8_t cls = byte_class[b[0] >> 3];
    state = state_transition[state + cls];
    *c = b[0] & masks[cls];
    *e = 0;
    ++b;

    while (state > State_Accept) {
        state = state_transition[state + byte_class[b[0] >> 3]];
        *c = (*c << 6) | (b[0] & 0x3f);
        ++b;
    }
    *e |= state == State_Reject;
    *e |= (*c < mins[cls]) << 1;
    *e |= ((*c >> 11) == 0x1b) << 2;
    *e |= (*c > 0x10ffff) << 3;

    return b;
}
