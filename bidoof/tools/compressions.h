#include "../base.h"

struct inflate_extra_info {
    u32 winsize;
    char const* level;
    u32 dictid;
    u32 adler32;
    sz last_at;
};

#define _accbits(__n) do {                            \
        unsigned wants = (__n);                       \
        while (has < wants) {                         \
            if (source->len <= at) {                  \
                free(r.ptr);                          \
                exitf("not enough bits to inflate");  \
            }                                         \
            acc|= source->ptr[at++] << has;           \
            has+= 8;                                  \
        }                                             \
    } while (0)
#define _dropbits(__n) do {      \
        unsigned drops = (__n);  \
        has-= drops;             \
        acc>>= drops;            \
    } while (0)
// https://www.ietf.org/rfc/rfc1950.txt
// https://www.ietf.org/rfc/rfc1951.txt
buf inflate(buf cref source, struct inflate_extra_info opref xnfo) _bdf_impl({
    buf r = {0};
    sz at = 0;

    if (source->len < 2) exitf("not enough bytes for inflate header");

    u8 cmf = source->ptr[at++];
    unsigned const cm = cmf & 0x0f, cinfo = (cmf & 0xf0) >> 4;
    if (8 != cm) exitf("compression method is not 8: cm=%d", cm);
    if (7 < cinfo) exitf("windo size bigger than 7: cinfo=%d", cinfo);
    if (xnfo) xnfo->winsize = 1 << (cinfo+8);

    u8 flg = source->ptr[at++];
    unsigned const /*fcheck = flg & 0x1f,*/ fdict = flg & 1<<5, flevel = (flg & 0xc) >> 6;

    if (xnfo) switch (flevel) {
        case 0: xnfo->level = "fastest"; break;
        case 1: xnfo->level = "fast";    break;
        case 2: xnfo->level = "default"; break;
        case 3: xnfo->level = "slowest"; break;
    }

    u32 dictid = 0;
    if (fdict) {
        if (source->len < 6) exitf("not enough bytes for inflate header");
        dictid = peek32be(source, at);
        at+= 2;
        if (xnfo) xnfo->dictid = dictid;
        exitf("NIY: preset dict (given id %u)\n", dictid);
    }

    unsigned has = 0;
    u64 acc = 0;
    unsigned bfinal, btype;
    do {
        _accbits(3);
        bfinal = acc & 1;
        btype = acc>>1 & 3;
        _dropbits(3);

        switch (btype) {
            case 0: // no compression
                _dropbits(has & 7);
                while (has != 0) { has-= 8; at--; }
                acc = 0;
                if (source->len < at+4) {
                    free(r.ptr);
                    exitf("not enough bytes in no compression block for len/nlen");
                }
                u16 len = peek32le(source, at) & 0xffff;
                at+= 4;
                if (source->len < at+len) {
                    free(r.ptr);
                    exitf("not enough bytes in no compression block");
                }
                if (r.cap < r.len+len && !dyarr_resize(&r, r.len+len)) {
                    free(r.ptr);
                    exitf("OOM");
                }
                memcpy(r.ptr, source->ptr+at, len);
                r.len+= len;
                at+= len;
                break;

            case 1: // fixed codes
                // TODO: fixed codes
                free(r.ptr);
                exitf("NIY: fixed codes block");
                break;

            case 2: { // dynamic codes
                _accbits(5+5+4);
                unsigned const hlit = (acc & 31) + 257;
                unsigned const hdist = (acc>>5 & 31) + 1;
                unsigned const hclen = (acc>>(5+4) & 15) + 4;
                _dropbits(5+5+4);

                // read the lens for the encoding of the instructions for the encoding of the codes
                unsigned lens[19] = {0};
                static unsigned clens_map[] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
                for (unsigned k = 0; k < hclen; k++) {
                    _accbits(3);
                    lens[clens_map[k]] = acc & 7;
                    _dropbits(3);
                }

                // build the instruction set for 3.2.7
                unsigned counts[8] = {0};
                for (unsigned k = 0; k < 19; k++) if (lens[k]) counts[lens[k]]++;
                unsigned next_code[8];
                next_code[0] = 0;
                for (unsigned b = 1; b < 8; b++) next_code[b] = (next_code[b-1] + counts[b-1]) << 1;
                unsigned dico[19] = {0};
                for (unsigned k = 0; k < 19; k++) {
                    unsigned code = next_code[lens[k]]++;
                    // reversed because that's how they are stored afterward
                    dico[k] = 0;
                    for (unsigned w = 0; w < lens[k]; w++) {
                        dico[k] = dico[k]<<1 | (code&1);
                        code>>= 1;
                    }
                }

                // FIXME:
                // << The code length repeat codes can cross from HLIT + 257 to the
                //    HDIST + 1 code lengths.  In other words, all code lengths form
                //    a single sequence of HLIT + HDIST + 258 values. >>

                // run the instructions to get the lens for the codes for lit/len alphabet
                // then the dist alphabet, and build their corresponding dico too
                // the dico is not used directly, instead a reverse one is:
                //    rdico[dico[k]] = k+1   where dico[k] would just be acc&mask
                //                           and the mask from 2 to NN bits all 1
                //                           `+1` is so 0 means it's not an entry
                unsigned lit_lens[286] = {0}, dist_lens[32] = {0};
                unsigned lit_dico[286] = {0}, dist_dico[32] = {0};
                unsigned lit_rdico[1<<16] = {0}, dist_rdico[1<<10] = {0}; // XXX: this probably blows the stack up
                for (unsigned* dlens = lit_lens; dlens; dlens = dlens == lit_lens ? dist_lens : NULL) {
                    unsigned const asize = dlens == lit_lens ? 286 : 32;

                    unsigned const count = dlens == lit_lens ? hlit : hdist;
                    for (unsigned l = 0; l < count; ) {
                        _accbits(7);
                        for (unsigned k = 0; k < 19; k++) if (lens[k] && (acc & ((1<<lens[k]) -1)) == dico[k]) {
                            _dropbits(lens[k]);
                            // instruction set of 3.2.7
                            if (k < 16) {
                                dlens[l++] = k;
                            } else if (16 == k) {
                                _accbits(2);
                                unsigned const it = dlens[l-1];
                                for (unsigned c = 0; c <3 + (acc&3); c++) dlens[l++] = it;
                                _dropbits(2);
                            } else if (17 == k) {
                                _accbits(3);
                                for (unsigned c = 0; c <3 + (acc&7); c++) dlens[l++] = 0;
                                _dropbits(3);
                            } else if (18 == k) {
                                _accbits(7);
                                for (unsigned c = 0; c < (acc&127) + 11; c++) dlens[l++] = 0;
                                _dropbits(7);
                            }
                            break;
                        } // for seach
                    } // for count

                    unsigned counts[16] = {0};
                    for (unsigned l = 0; l < asize; l++) if (dlens[l]) counts[dlens[l]]++;
                    unsigned next_code[16];
                    next_code[0] = 0;
                    for (unsigned b = 1; b < 16; b++) next_code[b] = (next_code[b-1] + counts[b-1]) << 1;
                    unsigned* const ddico = dlens == lit_lens ? lit_dico : dist_dico;
                    for (unsigned l = 0; l < asize; l++) {
                        unsigned code = next_code[dlens[l]]++;
                        // reversed because that's how they are stored afterward
                        ddico[l] = 0;
                        for (unsigned w = 0; w < dlens[l]; w++) {
                            ddico[l] = ddico[l]<<1 | (code&1);
                            code>>= 1;
                        }
                    }
                    unsigned* const drdico = dlens == lit_lens ? lit_rdico : dist_rdico;
                    for (unsigned l = 0; l < asize; l++) if (dlens[l]) drdico[ddico[l]] = l+1;
                } // for both alphabets

                for (;3;) {

                    _accbits(15);
                    unsigned b;
                    for (b = 2; b < 16; b++) {
                        unsigned const lpo = lit_rdico[acc & ((1<<b) -1)];
                        if (lpo && b == lit_lens[lpo-1]) {
                            unsigned const l = lpo-1;
                            _dropbits(lit_lens[l]);

                            // literal
                            if (l < 256) {
                                u8* const p = dyarr_push(&r);
                                if (!p) {
                                    free(r.ptr);
                                    exitf("OOM");
                                }
                                *p = l;
                            }

                            // end of block
                            else if (256 == l) goto end_of_block;

                            // length [+ distance]
                            else {
                                // length with its extra
                                static unsigned const extra_len_map[] =       {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2,  2,  2,  3,  3,  3,  3,  4,  4,  4,  4,   5,   5,   5,   5,   0};
                                static unsigned const extra_len_cumul_map[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 7, 10, 13, 16, 23, 30, 37, 44, 59, 74, 89, 104, 135, 166, 197, 227};
                                _accbits(extra_len_map[l-257]);
                                unsigned const len = l-257+3
                                                   + extra_len_cumul_map[l-257]
                                                   + (acc & ((1<<extra_len_map[l-257]) -1));
                                _dropbits(extra_len_map[l-257]);

                                // find the distance code
                                _accbits(9);
                                unsigned b;
                                for (b = 2; b < 10; b++) {
                                    unsigned const lpo = dist_rdico[acc & ((1<<b) -1)];
                                    if (lpo && b == dist_lens[lpo-1]) {
                                        unsigned const l = lpo-1;
                                        _dropbits(dist_lens[l]);

                                        // distance with its extra
                                        static unsigned const extra_dist_map[] =       {0, 0, 0, 0, 1, 1, 2, 2, 3,  3,  4,  4,  5,  5,   6,   6,   7,   7,   8,   8,    9,    9,   10,   10,   11,   11,   12,   12,     13,    13};
                                        static unsigned const extra_dist_cumul_map[] = {0, 0, 0, 0, 0, 1, 2, 5, 8, 15, 22, 37, 52, 83, 114, 177, 240, 367, 494, 749, 1004, 1515, 2026, 3049, 4072, 6119, 8166, 12261, 16356, 24547};
                                        _accbits(extra_dist_map[l]);
                                        unsigned const dist = l+1
                                                            + extra_dist_cumul_map[l]
                                                            + (acc & ((1<<extra_dist_map[l]) -1));
                                        _dropbits(extra_dist_map[l]);

                                        if (r.cap < r.len+len && !dyarr_resize(&r, r.len+len)) {
                                            free(r.ptr);
                                            exitf("OOM");
                                        }
                                        for (unsigned h = 0; h < len; h++) r.ptr[r.len+h] = r.ptr[r.len-dist+h];
                                        r.len+= len;
                                        break;
                                    } // if found dist
                                } // for seach dist
                                if (b == 10) {
                                    free(r.ptr);
                                    exitf("could not find next code in acc=%s", binstr(acc, has));
                                }
                            } // if > 256
                            break;
                        } // if found len
                    } // for search len
                    if (b == 16) {
                        free(r.ptr);
                        exitf("could not find next code in acc=%s", binstr(acc, has));
                    }
                } // until code 256
            end_of_block:;
            } break;

            case 3:
            default:
                free(r.ptr);
                exitf("unknown block type 3");
        }
    } while (!bfinal);

    if (xnfo) {
        _dropbits(has & 7);
        xnfo->adler32 = peek32be(source, at);
        xnfo->last_at = at;
    }

    return r;
})
#undef _dropbits
#undef _accbits
