/* Rink tiles: expand a tile map into the rink bitmap. */
/* DRAFT: 423 vs 459. The prologue homes shape, tiles and y and the tile word matches. Height is stored
 at esp+14h and width at esp+18h; the original stores height at esp+18h and width at esp+14h. The tile
 index is masked in edx; the original masks it in eax, then adds the tile base. The destination address
 and the 8x8 copy are scheduled differently. Do not retry: declaring height before width. */
#include "nhl95.h"

/* BlitTileMap: paint shape (width, height, then words) with 8x8 tiles from tiles. Tile bits 4000h and
 2000h flip vertically and horizontally, xored with flags. When flags is set the tile is mirrored
 through y,x. Colour FFh is skipped. x0 is unused. */
void BlitTileMap(unsigned char *shape, unsigned char *tiles, int x0, short y, short x, short flags)
{
    short height;
    short width;
    int row;
    int col;
    int tile;
    int vflip;
    int hflip;
    unsigned char *src;
    unsigned char *dst;
    int r;
    int c;
    unsigned char px;

    (void)x0;
    height = *(short *)(shape + 2);
    width = *(short *)shape;
    shape += 6;
    for (row = 0; row < height; row++) {
        for (col = 0; col < width; col++) {
            tile = *(short *)(shape + (row * width << 1) + (col << 1));
            vflip = (tile & 0x4000) ^ flags;
            hflip = flags ^ (tile & 0x2000);
            src = tiles + ((tile & 0x3FF) << 6);
            if (flags) {
                dst = (unsigned char *)(*(int *)((char *)rinktilebm + 0x2C) + 0x10
                    + ((((y + row) - (row << 1)) * rinkwtiles) << 6)
                    + ((((x + col) - (col << 1)) << 3)));
            } else {
                dst = (unsigned char *)(*(int *)((char *)rinktilebm + 0x2C) + 0x10
                    + (((y + row) * rinkwtiles) << 6)
                    + ((x + col) << 3));
            }
            if (vflip) dst += ((rinkwtiles << 3) - rinkwtiles) << 3;
            if (hflip) dst += 7;
            r = 0;
            do {
                c = 0;
                do {
                    px = *src;
                    if (px != 0xFF) *dst = px;
                    src++;
                    if (hflip) dst--;
                    else dst++;
                    c++;
                } while (c < 8);
                if (vflip) {
                    dst -= rinkwtiles << 3;
                    if (hflip) dst += 8;
                    else dst -= 8;
                } else {
                    dst += rinkwtiles << 3;
                    if (hflip) dst += 8;
                    else dst -= 8;
                }
                r++;
            } while (r < 8);
        }
    }
}
