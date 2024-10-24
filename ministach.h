#ifndef MINISTACH
#define MINISTACH

#include <string.h> // strlen() is our only dependency

/* Maximum number of keys a template can contain.
 */
#define MINISTACH_MAX_STACHES 128

enum {
    MINISTACH_OK,
    MINISTACH_EMPTY_STACH,       // "{{ }}" <- missing key in stach
    MINISTACH_DUPLICATE_KEY,     // "{{ a b }}" <- multiple potential keys; probably if the input accidently contains something that resembles a stach
    MINISTACH_MALFORMED_CLOSING, // "{{ a } }" || "{{a\00"
    MINISTACH_DATA_FUN_ERROR,    // the user provided dfun() returned NULL
};

typedef struct {
    int status; // non-0 signals an error during template compialation
    int size;   // number of characters the RENDERED template will take up; NULL NOT ACOUNTED FOR
    const char * s;        // source string
    int staches_empty_top; // number of staches; internally used for indexing in a stack like fashion
    struct {
        int beg; // first character to cut off (firt '{')
        int end; // last  character to cut off (second '}')
        const char * value; // value returned by the user provided dfun()
    } staches[MINISTACH_MAX_STACHES];
} ministach_t;

/* The userdata function type which is an argument for ministach_compile()
 * key      - template key name to look up; pointer inside the source template
 * key_len  - the lenght of the key in chars (since `key` will not be null terminated)
 * userdata - the 3th argument to ministach_compile() forwarded
 */
typedef const char * (*ministach_data_fun_t)(const char * key, int key_len, void * userdata);

/* Lexes the template to create a list of insertion positions and insertion values.
 * r        - uninitialized ministach_t, so it wont have to be passed over the stack; this value is returned
 * s        - template string
 * dfun     - user data function
 * userdata - arbitrary data to pass to dfun()
 */
ministach_t * ministach_compile(ministach_t * r, const char * const s, ministach_data_fun_t dfun, void * userdata) {
    *r = (ministach_t) {
        .status            = MINISTACH_OK,
        .size              = 0,
        .s                 = s,
        .staches_empty_top = 0,
    };

    enum {
        initial,
        in_stach,
        in_key,
    } state = initial;

    int brace_counter = 0; // used for recognizing double '{'s
    int is_key_found  = 0; // used for recognizing MINISTACH_EMPTY_STACH and MINISTACH_DUPLICATE_KEY
    int key_size      = 1; // 1 because we only start incrementing after the first character was found
    int key_begin;         // key location; != r.staches[N].beg

    for (int i = 0; s[i] != '\00'; i++) {
        switch (state) {
            case initial: {
                ++r->size;

                if (s[i] == '{') {
                    ++brace_counter;
                    if (brace_counter == 1) {
                        r->staches[r->staches_empty_top].beg = i;
                        break;
                    }
                } else {
                    brace_counter = 0;
                }

                if (brace_counter == 2) {
                    r->size -= 2;
                    state = in_stach;
                }
            } break;
            case in_stach: {
                if (s[i] == ' '
                ||  s[i] == '\t'
                ||  s[i] == '\n') {
                    break;
                }

                if (s[i] == '}') {
                    if (!is_key_found) {
                        r->status = MINISTACH_EMPTY_STACH;
                        goto end;
                    }
                    if (s[i+1] != '}') {
                        r->status = MINISTACH_MALFORMED_CLOSING;
                        goto end;
                    }

                    const char * d = dfun(s + key_begin, key_size, userdata);
                    if (!d) {
                        r->status = MINISTACH_DATA_FUN_ERROR;
                        goto end;
                    }

                    ++i;
                    r->staches[r->staches_empty_top].value = d;
                    r->staches[r->staches_empty_top].end   = i+1;
                    ++r->staches_empty_top;
                    key_size = 1;
                    is_key_found = 0;
                    state = initial;
                    break;
                }

                if (is_key_found) {
                    r->status = MINISTACH_DUPLICATE_KEY;
                    goto end;
                }

                is_key_found = 1;
                key_begin = i;
                state = in_key;
            } break;
            case in_key: {
                if (s[i] == ' '
                ||  s[i] == '\t'
                ||  s[i] == '\n') {
                    state = in_stach;
                    break;
                }

                if (s[i] == '}') {
                    --i;
                    state = in_stach;
                    break;
                }

                ++key_size;
            } break;
        }
    }

    if (state != initial) {
        r->status = MINISTACH_MALFORMED_CLOSING;
        goto end;
    }

    for (int i = 0; i < r->staches_empty_top; i++) {
        r->size += strlen(r->staches[i].value);
    }

  end:
    return r;
}

/* Copy the compiled ministach_t * m
 * into the buffer b
 */
int ministach_render(ministach_t * m, char * b) {
    if (m->staches_empty_top == 0) {
        memcpy(b, m->s, strlen(m->s));
        return 0;
    }

    int ooffset = 0;
    int moffset = 0;
    for (int i = 0; i < m->staches_empty_top; i++) {
        size_t m_si_v_len = strlen(m->staches[i].value);
        memcpy(b + ooffset, m->s + moffset, m->staches[i].beg);
        ooffset += m->staches[i].beg - moffset;
        memcpy(b + ooffset, m->staches[i].value, m_si_v_len);
        ooffset += m_si_v_len;
        moffset = m->staches[i].end;
    }

    size_t m_s_len = strlen(m->s);
    if (moffset != m_s_len) {
        memcpy(b + ooffset, m->s + moffset, m_s_len-moffset+1);
    } else {
        b[ooffset] = '\00';
    }

    return 0;
}

#endif
