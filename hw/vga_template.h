/*
 * QEMU VGA Emulator templates
 *
 * Copyright (c) 2003 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#if DEPTH == 8
#define BPP 1
#define PIXEL_TYPE uint8_t
#elif DEPTH == 15 || DEPTH == 16
#define BPP 2
#define PIXEL_TYPE uint16_t
#elif DEPTH == 32
#define BPP 4
#define PIXEL_TYPE uint32_t
#else
#error unsupport depth
#endif

#ifdef BGR_FORMAT
#define PIXEL_NAME glue(DEPTH, bgr)
#else
#define PIXEL_NAME DEPTH
#endif /* BGR_FORMAT */

#if DEPTH != 15 && !defined(BGR_FORMAT)

static inline void glue(vga_draw_glyph_line_, DEPTH)(uint8_t *d,
                                                     uint32_t font_data,
                                                     uint32_t xorcol,
                                                     uint32_t bgcol)
{
#if BPP == 1
        ((uint32_t *)d)[0] = (dmask16[(font_data >> 4)] & xorcol) ^ bgcol;
        ((uint32_t *)d)[1] = (dmask16[(font_data >> 0) & 0xf] & xorcol) ^ bgcol;
#elif BPP == 2
        ((uint32_t *)d)[0] = (dmask4[(font_data >> 6)] & xorcol) ^ bgcol;
        ((uint32_t *)d)[1] = (dmask4[(font_data >> 4) & 3] & xorcol) ^ bgcol;
        ((uint32_t *)d)[2] = (dmask4[(font_data >> 2) & 3] & xorcol) ^ bgcol;
        ((uint32_t *)d)[3] = (dmask4[(font_data >> 0) & 3] & xorcol) ^ bgcol;
#else
        ((uint32_t *)d)[0] = (-((font_data >> 7)) & xorcol) ^ bgcol;
        ((uint32_t *)d)[1] = (-((font_data >> 6) & 1) & xorcol) ^ bgcol;
        ((uint32_t *)d)[2] = (-((font_data >> 5) & 1) & xorcol) ^ bgcol;
        ((uint32_t *)d)[3] = (-((font_data >> 4) & 1) & xorcol) ^ bgcol;
        ((uint32_t *)d)[4] = (-((font_data >> 3) & 1) & xorcol) ^ bgcol;
        ((uint32_t *)d)[5] = (-((font_data >> 2) & 1) & xorcol) ^ bgcol;
        ((uint32_t *)d)[6] = (-((font_data >> 1) & 1) & xorcol) ^ bgcol;
        ((uint32_t *)d)[7] = (-((font_data >> 0) & 1) & xorcol) ^ bgcol;
#endif
}

static void glue(vga_draw_glyph8_, DEPTH)(uint8_t *d, int linesize,
                                          const uint8_t *font_ptr, int h,
                                          uint32_t fgcol, uint32_t bgcol)
{
    uint32_t font_data, xorcol;

    xorcol = bgcol ^ fgcol;
    do {
        font_data = font_ptr[0];
        glue(vga_draw_glyph_line_, DEPTH)(d, font_data, xorcol, bgcol);
        font_ptr += 4;
        d += linesize;
    } while (--h);
}

static void glue(vga_draw_glyph16_, DEPTH)(uint8_t *d, int linesize,
                                          const uint8_t *font_ptr, int h,
                                          uint32_t fgcol, uint32_t bgcol)
{
    uint32_t font_data, xorcol;

    xorcol = bgcol ^ fgcol;
    do {
        font_data = font_ptr[0];
        glue(vga_draw_glyph_line_, DEPTH)(d,
                                          expand4to8[font_data >> 4],
                                          xorcol, bgcol);
        glue(vga_draw_glyph_line_, DEPTH)(d + 8 * BPP,
                                          expand4to8[font_data & 0x0f],
                                          xorcol, bgcol);
        font_ptr += 4;
        d += linesize;
    } while (--h);
}

static void glue(vga_draw_glyph9_, DEPTH)(uint8_t *d, int linesize,
                                          const uint8_t *font_ptr, int h,
                                          uint32_t fgcol, uint32_t bgcol, int dup9)
{
    uint32_t font_data, xorcol, v;

    xorcol = bgcol ^ fgcol;
    do {
        font_data = font_ptr[0];
#if BPP == 1
        cpu_to_32wu((uint32_t *)d, (dmask16[(font_data >> 4)] & xorcol) ^ bgcol);
        v = (dmask16[(font_data >> 0) & 0xf] & xorcol) ^ bgcol;
        cpu_to_32wu(((uint32_t *)d)+1, v);
        if (dup9)
            ((uint8_t *)d)[8] = v >> (24 * (1 - BIG));
        else
            ((uint8_t *)d)[8] = bgcol;

#elif BPP == 2
        cpu_to_32wu(((uint32_t *)d)+0, (dmask4[(font_data >> 6)] & xorcol) ^ bgcol);
        cpu_to_32wu(((uint32_t *)d)+1, (dmask4[(font_data >> 4) & 3] & xorcol) ^ bgcol);
        cpu_to_32wu(((uint32_t *)d)+2, (dmask4[(font_data >> 2) & 3] & xorcol) ^ bgcol);
        v = (dmask4[(font_data >> 0) & 3] & xorcol) ^ bgcol;
        cpu_to_32wu(((uint32_t *)d)+3, v);
        if (dup9)
            ((uint16_t *)d)[8] = v >> (16 * (1 - BIG));
        else
            ((uint16_t *)d)[8] = bgcol;
#else
        ((uint32_t *)d)[0] = (-((font_data >> 7)) & xorcol) ^ bgcol;
        ((uint32_t *)d)[1] = (-((font_data >> 6) & 1) & xorcol) ^ bgcol;
        ((uint32_t *)d)[2] = (-((font_data >> 5) & 1) & xorcol) ^ bgcol;
        ((uint32_t *)d)[3] = (-((font_data >> 4) & 1) & xorcol) ^ bgcol;
        ((uint32_t *)d)[4] = (-((font_data >> 3) & 1) & xorcol) ^ bgcol;
        ((uint32_t *)d)[5] = (-((font_data >> 2) & 1) & xorcol) ^ bgcol;
        ((uint32_t *)d)[6] = (-((font_data >> 1) & 1) & xorcol) ^ bgcol;
        v = (-((font_data >> 0) & 1) & xorcol) ^ bgcol;
        ((uint32_t *)d)[7] = v;
        if (dup9)
            ((uint32_t *)d)[8] = v;
        else
            ((uint32_t *)d)[8] = bgcol;
#endif
        font_ptr += 4;
        d += linesize;
    } while (--h);
}

#ifndef VGA_READ_FUNCTION
#define VGA_READ_FUNCTION 1
static inline uint8_t vga_read_byte(VGACommonState *vga, uint32_t addr)
{
    return vga->vram_ptr[addr & vga->vbe_size_mask];
}

static inline uint16_t vga_read_word_le(VGACommonState *vga, uint32_t addr)
{
    uint32_t offset = addr & vga->vbe_size_mask & ~1;
    uint16_t *ptr = (uint16_t *)(vga->vram_ptr + offset);
    return lduw_le_p(ptr);
}

static inline uint16_t vga_read_word_be(VGACommonState *vga, uint32_t addr)
{
    uint32_t offset = addr & vga->vbe_size_mask & ~1;
    uint16_t *ptr = (uint16_t *)(vga->vram_ptr + offset);
    return lduw_be_p(ptr);
}

static inline uint32_t vga_read_dword_le(VGACommonState *vga, uint32_t addr)
{
    uint32_t offset = addr & vga->vbe_size_mask & ~3;
    uint32_t *ptr = (uint32_t *)(vga->vram_ptr + offset);
    return ldl_le_p(ptr);
}
#endif

/*
 * 4 color mode
 */
static void glue(vga_draw_line2_, DEPTH)(VGACommonState *vga, uint8_t *d,
                                         uint32_t addr, int width)
{
    uint32_t plane_mask, *palette, data, v;
    int x;

    palette = vga->last_palette;
    plane_mask = mask16[vga->ar[VGA_ATC_PLANE_ENABLE] & 0xf];
    width >>= 3;
    for(x = 0; x < width; x++) {
        data = vga_read_dword_le(vga, addr);
        data &= plane_mask;
        v = expand2[GET_PLANE(data, 0)];
        v |= expand2[GET_PLANE(data, 2)] << 2;
        ((PIXEL_TYPE *)d)[0] = palette[v >> 12];
        ((PIXEL_TYPE *)d)[1] = palette[(v >> 8) & 0xf];
        ((PIXEL_TYPE *)d)[2] = palette[(v >> 4) & 0xf];
        ((PIXEL_TYPE *)d)[3] = palette[(v >> 0) & 0xf];

        v = expand2[GET_PLANE(data, 1)];
        v |= expand2[GET_PLANE(data, 3)] << 2;
        ((PIXEL_TYPE *)d)[4] = palette[v >> 12];
        ((PIXEL_TYPE *)d)[5] = palette[(v >> 8) & 0xf];
        ((PIXEL_TYPE *)d)[6] = palette[(v >> 4) & 0xf];
        ((PIXEL_TYPE *)d)[7] = palette[(v >> 0) & 0xf];
        d += BPP * 8;
        addr += 4;
    }
}

#if BPP == 1
#define PUT_PIXEL2(d, n, v) ((uint16_t *)d)[(n)] = (v)
#elif BPP == 2
#define PUT_PIXEL2(d, n, v) ((uint32_t *)d)[(n)] = (v)
#else
#define PUT_PIXEL2(d, n, v) \
((uint32_t *)d)[2*(n)] = ((uint32_t *)d)[2*(n)+1] = (v)
#endif

/*
 * 4 color mode, dup2 horizontal
 */
static void glue(vga_draw_line2d2_, DEPTH)(VGACommonState *vga, uint8_t *d,
                                           uint32_t addr, int width)
{
    uint32_t plane_mask, *palette, data, v;
    int x;

    palette = vga->last_palette;
    plane_mask = mask16[vga->ar[VGA_ATC_PLANE_ENABLE] & 0xf];
    width >>= 3;
    for(x = 0; x < width; x++) {
        data = vga_read_dword_le(vga, addr);
        data &= plane_mask;
        v = expand2[GET_PLANE(data, 0)];
        v |= expand2[GET_PLANE(data, 2)] << 2;
        PUT_PIXEL2(d, 0, palette[v >> 12]);
        PUT_PIXEL2(d, 1, palette[(v >> 8) & 0xf]);
        PUT_PIXEL2(d, 2, palette[(v >> 4) & 0xf]);
        PUT_PIXEL2(d, 3, palette[(v >> 0) & 0xf]);

        v = expand2[GET_PLANE(data, 1)];
        v |= expand2[GET_PLANE(data, 3)] << 2;
        PUT_PIXEL2(d, 4, palette[v >> 12]);
        PUT_PIXEL2(d, 5, palette[(v >> 8) & 0xf]);
        PUT_PIXEL2(d, 6, palette[(v >> 4) & 0xf]);
        PUT_PIXEL2(d, 7, palette[(v >> 0) & 0xf]);
        d += BPP * 16;
        addr += 4;
    }
}

/*
 * 16 color mode
 */
static void glue(vga_draw_line4_, DEPTH)(VGACommonState *vga, uint8_t *d,
                                         uint32_t addr, int width)
{
    uint32_t plane_mask, data, v, *palette;
    int x;

    palette = vga->last_palette;
    plane_mask = mask16[vga->ar[VGA_ATC_PLANE_ENABLE] & 0xf];
    width >>= 3;
    for(x = 0; x < width; x++) {
        data = vga_read_dword_le(vga, addr);
        data &= plane_mask;
        v = expand4[GET_PLANE(data, 0)];
        v |= expand4[GET_PLANE(data, 1)] << 1;
        v |= expand4[GET_PLANE(data, 2)] << 2;
        v |= expand4[GET_PLANE(data, 3)] << 3;
        ((PIXEL_TYPE *)d)[0] = palette[v >> 28];
        ((PIXEL_TYPE *)d)[1] = palette[(v >> 24) & 0xf];
        ((PIXEL_TYPE *)d)[2] = palette[(v >> 20) & 0xf];
        ((PIXEL_TYPE *)d)[3] = palette[(v >> 16) & 0xf];
        ((PIXEL_TYPE *)d)[4] = palette[(v >> 12) & 0xf];
        ((PIXEL_TYPE *)d)[5] = palette[(v >> 8) & 0xf];
        ((PIXEL_TYPE *)d)[6] = palette[(v >> 4) & 0xf];
        ((PIXEL_TYPE *)d)[7] = palette[(v >> 0) & 0xf];
        d += BPP * 8;
        addr += 4;
    }
}

/*
 * 16 color mode, dup2 horizontal
 */
static void glue(vga_draw_line4d2_, DEPTH)(VGACommonState *vga, uint8_t *d,
                                           uint32_t addr, int width)
{
    uint32_t plane_mask, data, v, *palette;
    int x;

    palette = vga->last_palette;
    plane_mask = mask16[vga->ar[VGA_ATC_PLANE_ENABLE] & 0xf];
    width >>= 3;
    for(x = 0; x < width; x++) {
        data = vga_read_dword_le(vga, addr);
        data &= plane_mask;
        v = expand4[GET_PLANE(data, 0)];
        v |= expand4[GET_PLANE(data, 1)] << 1;
        v |= expand4[GET_PLANE(data, 2)] << 2;
        v |= expand4[GET_PLANE(data, 3)] << 3;
        PUT_PIXEL2(d, 0, palette[v >> 28]);
        PUT_PIXEL2(d, 1, palette[(v >> 24) & 0xf]);
        PUT_PIXEL2(d, 2, palette[(v >> 20) & 0xf]);
        PUT_PIXEL2(d, 3, palette[(v >> 16) & 0xf]);
        PUT_PIXEL2(d, 4, palette[(v >> 12) & 0xf]);
        PUT_PIXEL2(d, 5, palette[(v >> 8) & 0xf]);
        PUT_PIXEL2(d, 6, palette[(v >> 4) & 0xf]);
        PUT_PIXEL2(d, 7, palette[(v >> 0) & 0xf]);
        d += BPP * 16;
        addr += 4;
    }
}

/*
 * 256 color mode, double pixels
 *
 * XXX: add plane_mask support (never used in standard VGA modes)
 */
static void glue(vga_draw_line8d2_, DEPTH)(VGACommonState *vga, uint8_t *d,
                                           uint32_t addr, int width)
{
    uint32_t *palette;
    int x;

    palette = vga->last_palette;
    width >>= 3;
    for(x = 0; x < width; x++) {
        PUT_PIXEL2(d, 0, palette[vga_read_byte(vga, addr + 0)]);
        PUT_PIXEL2(d, 1, palette[vga_read_byte(vga, addr + 1)]);
        PUT_PIXEL2(d, 2, palette[vga_read_byte(vga, addr + 2)]);
        PUT_PIXEL2(d, 3, palette[vga_read_byte(vga, addr + 3)]);
        d += BPP * 8;
        addr += 4;
    }
}

/*
 * standard 256 color mode
 *
 * XXX: add plane_mask support (never used in standard VGA modes)
 */
static void glue(vga_draw_line8_, DEPTH)(VGACommonState *vga, uint8_t *d,
                                         uint32_t addr, int width)
{
    uint32_t *palette;
    int x;

    palette = vga->last_palette;
    width >>= 3;
    for(x = 0; x < width; x++) {
        ((PIXEL_TYPE *)d)[0] = palette[vga_read_byte(vga, addr + 0)];
        ((PIXEL_TYPE *)d)[1] = palette[vga_read_byte(vga, addr + 1)];
        ((PIXEL_TYPE *)d)[2] = palette[vga_read_byte(vga, addr + 2)];
        ((PIXEL_TYPE *)d)[3] = palette[vga_read_byte(vga, addr + 3)];
        ((PIXEL_TYPE *)d)[4] = palette[vga_read_byte(vga, addr + 4)];
        ((PIXEL_TYPE *)d)[5] = palette[vga_read_byte(vga, addr + 5)];
        ((PIXEL_TYPE *)d)[6] = palette[vga_read_byte(vga, addr + 6)];
        ((PIXEL_TYPE *)d)[7] = palette[vga_read_byte(vga, addr + 7)];
        d += BPP * 8;
        addr += 8;
    }
}

#endif /* DEPTH != 15 */


/* XXX: optimize */

/*
 * 15 bit color
 */
static void glue(vga_draw_line15_, PIXEL_NAME)(VGACommonState *vga, uint8_t *d,
                                               uint32_t addr, int width)
{
#if DEPTH == 15 && defined(HOST_WORDS_BIGENDIAN) == defined(TARGET_WORDS_BIGENDIAN)
    uint32_t offset = addr & vga->vbe_size_mask & ~1;
    memcpy(d, (uint8_t *)(vga->vram_ptr + offset), width * 2);
#else
    int w;
    uint32_t v, r, g, b;

    w = width;
    do {
#if defined(TARGET_WORDS_BIGENDIAN)
        v = vga_read_word_be(vga, addr);
#else
        v = vga_read_word_le(vga, addr);
#endif
        r = (v >> 7) & 0xf8;
        g = (v >> 2) & 0xf8;
        b = (v << 3) & 0xf8;
        ((PIXEL_TYPE *)d)[0] = glue(rgb_to_pixel, PIXEL_NAME)(r, g, b);
        addr += 2;
        d += BPP;
    } while (--w != 0);
#endif
}

/*
 * 16 bit color
 */
static void glue(vga_draw_line16_, PIXEL_NAME)(VGACommonState *vga, uint8_t *d,
                                               uint32_t addr, int width)
{
#if DEPTH == 16 && defined(HOST_WORDS_BIGENDIAN) == defined(TARGET_WORDS_BIGENDIAN)
    uint32_t offset = addr & vga->vbe_size_mask & ~1;
    memcpy(d, (uint8_t *)(vga->vram_ptr + offset), width * 2);
#else
    int w;
    uint32_t v, r, g, b;

    w = width;
    do {
#if defined(TARGET_WORDS_BIGENDIAN)
        v = vga_read_word_be(vga, addr);
#else
        v = vga_read_word_le(vga, addr);
#endif
        r = (v >> 8) & 0xf8;
        g = (v >> 3) & 0xfc;
        b = (v << 3) & 0xf8;
        ((PIXEL_TYPE *)d)[0] = glue(rgb_to_pixel, PIXEL_NAME)(r, g, b);
        addr += 2;
        d += BPP;
    } while (--w != 0);
#endif
}

/*
 * 24 bit color
 */
static void glue(vga_draw_line24_, PIXEL_NAME)(VGACommonState *vga, uint8_t *d,
                                               uint32_t addr, int width)
{
    int w;
    uint32_t r, g, b;

    w = width;
    do {
#if defined(TARGET_WORDS_BIGENDIAN)
        r = vga_read_byte(vga, addr + 0);
        g = vga_read_byte(vga, addr + 1);
        b = vga_read_byte(vga, addr + 2);
#else
        b = vga_read_byte(vga, addr + 0);
        g = vga_read_byte(vga, addr + 1);
        r = vga_read_byte(vga, addr + 2);
#endif
        ((PIXEL_TYPE *)d)[0] = glue(rgb_to_pixel, PIXEL_NAME)(r, g, b);
        addr += 3;
        d += BPP;
    } while (--w != 0);
}

/*
 * 32 bit color
 */
static void glue(vga_draw_line32_, PIXEL_NAME)(VGACommonState *vga, uint8_t *d,
                                               uint32_t addr, int width)
{
#if DEPTH == 32 && defined(HOST_WORDS_BIGENDIAN) == defined(TARGET_WORDS_BIGENDIAN) && !defined(BGR_FORMAT)
    uint32_t offset = addr & vga->vbe_size_mask & ~3;
    memcpy(d, (uint8_t *)(vga->vram_ptr + offset), width * 4);
#else
    int w;
    uint32_t r, g, b;

    w = width;
    do {
#if defined(TARGET_WORDS_BIGENDIAN)
        r = vga_read_byte(vga, addr + 1);
        g = vga_read_byte(vga, addr + 2);
        b = vga_read_byte(vga, addr + 3);
#else
        b = vga_read_byte(vga, addr + 0);
        g = vga_read_byte(vga, addr + 1);
        r = vga_read_byte(vga, addr + 2);
#endif
        ((PIXEL_TYPE *)d)[0] = glue(rgb_to_pixel, PIXEL_NAME)(r, g, b);
        addr += 4;
        d += BPP;
    } while (--w != 0);
#endif
}

#undef PUT_PIXEL2
#undef DEPTH
#undef BPP
#undef PIXEL_TYPE
#undef PIXEL_NAME
#undef BGR_FORMAT
