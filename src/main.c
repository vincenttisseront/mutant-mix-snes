/* Mutant Mix — Histoire -> choix du personnage -> niveau 1 Égouts-Venin */
#include <snes.h>

extern char snesfont, snespal;
extern char level1pic, level1pic_end, level1map, level1map_end, level1pal;
extern char hero0pic, hero0pic_end, hero0pal;
extern char hero1pic, hero1pic_end, hero1pal;
extern char hero2pic, hero2pic_end, hero2pal;
extern char hero3pic, hero3pic_end, hero3pal;
extern char ratpic, ratpic_end, ratpal;
extern const unsigned char level1_col[2048];

#define ST_MENU 0
#define ST_PASS 1
#define ST_SELECT 2
#define ST_BRIEF 3
#define ST_PLAY 4
#define ST_CLEAR 5

#define HIT_W 16
#define HIT_H 36
#define WORLD_W 512
#define GROUND_Y 192
#define HERO_TILES 128
#define HERO_BYTES 4096
#define RAT_TILE 512
#define RAT_BYTES 2048

#define COL_SOLID 1
#define COL_HURT 2
#define COL_EXIT 3

u8 state;
u8 cursor;
u8 hero;
u8 lives;
u8 hurt_t;
u16 px, py;
s16 vy;
u16 cam;
u16 prev_pad;
u16 rat_x[2];
s8 rat_dir[2];
u16 rat_min[2];
u16 rat_max[2];

void text_clear(void)
{
    u16 i;
    for (i = 0; i < 0x800; i++)
        scr_txt_font_map[i] = 0;
    scr_txt_dirty = 1;
}

u8 col_at(u16 x, u16 y)
{
    if (x >= WORLD_W || y >= 256)
        return COL_SOLID;
    return level1_col[(y >> 3) * 64 + (x >> 3)];
}

u8 hit_kind(u16 x, u16 y, u8 kind)
{
    if (col_at(x + 2, y + 4) == kind)
        return 1;
    if (col_at(x + HIT_W - 3, y + 4) == kind)
        return 1;
    if (col_at(x + 2, y + HIT_H - 2) == kind)
        return 1;
    if (col_at(x + HIT_W - 3, y + HIT_H - 2) == kind)
        return 1;
    return 0;
}

u8 blocked(u16 x, u16 y)
{
    return hit_kind(x, y, COL_SOLID);
}

void hide_sprites(void)
{
    oamSetVisible(0, OBJ_HIDE);
    oamSetVisible(4, OBJ_HIDE);
    oamSetVisible(8, OBJ_HIDE);
    oamSetVisible(12, OBJ_HIDE);
    oamSetVisible(16, OBJ_HIDE);
    oamSetVisible(20, OBJ_HIDE);
}

void show_roster(void)
{
    u16 bob;
    u16 i;
    u16 y;
    bob = (snes_vblank_count >> 3) & 1;
    for (i = 0; i < 4; i++)
    {
        y = 96;
        if (i == hero && bob)
            y = 92;
        oamSet(i * 4, 8 + i * 60, y, 3, 0, 0, i * HERO_TILES, i);
        oamSetEx(i * 4, OBJ_LARGE, OBJ_SHOW);
    }
    oamSetVisible(16, OBJ_HIDE);
    oamSetVisible(20, OBJ_HIDE);
}

void draw_menu(void)
{
    text_clear();
    consoleDrawText(8, 4, "MUTANT MIX");
    consoleDrawText(7, 6, "SHADOW SQUAD");
    consoleDrawText(6, 12, "HISTOIRE");
    consoleDrawText(6, 14, "MOT DE PASSE");
    consoleDrawText(2, 22, "HAUT BAS  A VALIDER");
}

void draw_menu_cursor(void)
{
    consoleDrawText(4, 12, cursor == 0 ? ">" : " ");
    consoleDrawText(4, 14, cursor == 1 ? ">" : " ");
}

void draw_pass(void)
{
    text_clear();
    consoleDrawText(6, 8, "CODE MIX");
    consoleDrawText(4, 11, "BIENTOT DISPONIBLE");
    consoleDrawText(6, 20, "B RETOUR");
}

void draw_select(void)
{
    text_clear();
    consoleDrawText(5, 3, "CHOISIS TON OMBRE");
    consoleDrawText(2, 18, "AZU");
    consoleDrawText(9, 18, "KAERU");
    consoleDrawText(16, 18, "QUILLA");
    consoleDrawText(24, 18, "PADDO");
    consoleDrawText(2, 22, "GAUCHE DROITE A  B RETOUR");
}

void draw_select_cursor(void)
{
    consoleDrawText(2, 19, hero == 0 ? "---" : "   ");
    consoleDrawText(9, 19, hero == 1 ? "-----" : "     ");
    consoleDrawText(16, 19, hero == 2 ? "------" : "      ");
    consoleDrawText(24, 19, hero == 3 ? "-----" : "     ");
}

void draw_brief(void)
{
    text_clear();
    consoleDrawText(8, 6, "NIVEAU 1");
    consoleDrawText(5, 9, "EGOUTS-VENIN");
    consoleDrawText(6, 11, "NOX HARBOUR");
    consoleDrawText(3, 14, "BOGOR GARDE LE NOYAU");
    consoleDrawText(6, 18, "A  DESCENDRE");
}

void draw_hud(void)
{
    consoleDrawText(1, 1, "EGOUTS");
    consoleDrawText(22, 1, "VIES ");
    consoleDrawText(27, 1, lives == 3 ? "3" : lives == 2 ? "2" : lives == 1 ? "1" : "0");
}

void draw_clear(void)
{
    text_clear();
    consoleDrawText(4, 8, "NOYAU VENIN EN VUE");
    consoleDrawText(3, 11, "BOGOR T'ATTEND PLUS LOIN");
    consoleDrawText(4, 16, "NIVEAU 1 FRANCHI");
    consoleDrawText(5, 20, "START  MENU");
}

void spawn_player(void)
{
    px = 32;
    py = GROUND_Y - HIT_H;
    vy = 0;
    hurt_t = 90;
    rat_x[0] = 180;
    rat_dir[0] = 1;
    rat_min[0] = 168;
    rat_max[0] = 250;
    rat_x[1] = 360;
    rat_dir[1] = -1;
    rat_min[1] = 328;
    rat_max[1] = 450;
}

void enter_state(u8 next)
{
    state = next;
    if (next == ST_MENU)
    {
        bgSetDisable(1);
        hide_sprites();
        draw_menu();
        draw_menu_cursor();
    }
    else if (next == ST_PASS)
    {
        bgSetDisable(1);
        hide_sprites();
        draw_pass();
    }
    else if (next == ST_SELECT)
    {
        bgSetDisable(1);
        draw_select();
        draw_select_cursor();
    }
    else if (next == ST_BRIEF)
    {
        lives = 3;
        spawn_player();
        cam = 0;
        bgSetScroll(1, 0, 0);
        bgSetEnable(1);
        hide_sprites();
        draw_brief();
    }
    else if (next == ST_PLAY)
    {
        text_clear();
        draw_hud();
        bgSetEnable(1);
    }
    else if (next == ST_CLEAR)
    {
        hide_sprites();
        bgSetDisable(1);
        draw_clear();
    }
}

void load_graphics(void)
{
    u16 n;
    consoleSetTextVramBGAdr(0x6800);
    consoleSetTextVramAdr(0x5000);
    consoleSetTextOffset(0);
    consoleInitText(0, 16 * 2, &snesfont, &snespal);

    bgInitTileSet(1, &level1pic, &level1pal, 0, (&level1pic_end - &level1pic), 16 * 2, BG_16COLORS, 0x6000);
    bgInitMapSet(1, &level1map, (&level1map_end - &level1map), SC_64x32, 0x7000);
    bgSetGfxPtr(0, 0x5000);
    bgSetMapPtr(0, 0x6800, SC_32x32);

    n = HERO_BYTES;
    oamInitGfxSet(&hero0pic, n, &hero0pal, 16 * 2, 0, 0x0000, OBJ_SIZE32_L64);
    dmaCopyVram(&hero1pic, HERO_BYTES, HERO_BYTES);
    dmaCopyVram(&hero2pic, HERO_BYTES * 2, HERO_BYTES);
    dmaCopyVram(&hero3pic, HERO_BYTES * 3, HERO_BYTES);
    dmaCopyVram(&ratpic, HERO_BYTES * 4, RAT_BYTES);
    dmaCopyCGram(&hero1pal, 128 + 16, 16 * 2);
    dmaCopyCGram(&hero2pal, 128 + 32, 16 * 2);
    dmaCopyCGram(&hero3pal, 128 + 48, 16 * 2);
    dmaCopyCGram(&ratpal, 128 + 64, 16 * 2);

    setMode(BG_MODE1, 0);
    bgSetDisable(1);
    bgSetDisable(2);
    setScreenOn();
}

void try_move_x(s16 dx)
{
    u16 nx;
    if (dx < 0 && px < (u16)(-dx))
        nx = 0;
    else
        nx = (u16)((s16)px + dx);
    if (nx > WORLD_W - HIT_W)
        nx = WORLD_W - HIT_W;
    if (!blocked(nx, py))
        px = nx;
}

void try_move_y(void)
{
    s16 ny;
    ny = (s16)py + vy;
    if (ny < 0)
        ny = 0;
    if (vy > 0)
    {
        if (!blocked(px, (u16)ny))
            py = (u16)ny;
        else
        {
            vy = 0;
            py = (py + HIT_H) & 0xFFF8;
            if (py >= HIT_H)
                py = py - HIT_H;
        }
    }
    else if (vy < 0)
    {
        if (!blocked(px, (u16)ny))
            py = (u16)ny;
        else
            vy = 0;
    }
    if (col_at(px + 4, py + HIT_H) == COL_SOLID || col_at(px + HIT_W - 4, py + HIT_H) == COL_SOLID)
    {
        if (vy > 0)
            vy = 0;
    }
    else if (vy < 8)
    {
        vy++;
    }
}

void hurt_player(void)
{
    if (hurt_t)
        return;
    if (lives)
        lives--;
    draw_hud();
    if (lives == 0)
    {
        enter_state(ST_SELECT);
        return;
    }
    spawn_player();
}

void update_rats(void)
{
    u8 i;
    u16 sy;
    s16 sx;
    for (i = 0; i < 2; i++)
    {
        if (rat_dir[i] > 0)
        {
            rat_x[i] += 1;
            if (rat_x[i] >= rat_max[i])
                rat_dir[i] = -1;
        }
        else
        {
            if (rat_x[i] > rat_min[i])
                rat_x[i] -= 1;
            else
                rat_dir[i] = 1;
        }
        sx = (s16)rat_x[i] - (s16)cam;
        sy = GROUND_Y - 32;
        if (sx > -32 && sx < 256)
        {
            oamSet(16 + i * 4, (u16)sx, sy, 3, rat_dir[i] < 0, 0, RAT_TILE, 4);
            oamSetEx(16 + i * 4, OBJ_SMALL, OBJ_SHOW);
        }
        else
            oamSetVisible(16 + i * 4, OBJ_HIDE);

        if (state == ST_PLAY && px + HIT_W > rat_x[i] + 6 && px < rat_x[i] + 26 && py + HIT_H > sy && py < sy + 32)
            hurt_player();
        if (state != ST_PLAY)
            return;
    }
}

void update_play(u16 pad, u16 pressed)
{
    s16 sprx;
    u16 flip;
    if (pad & KEY_LEFT)
    {
        try_move_x(-2);
        flip = 1;
    }
    else if (pad & KEY_RIGHT)
    {
        try_move_x(2);
        flip = 0;
    }
    else
        flip = 0;

    if ((pressed & KEY_B) || (pressed & KEY_A))
    {
        if (col_at(px + 4, py + HIT_H) == COL_SOLID || col_at(px + HIT_W - 4, py + HIT_H) == COL_SOLID)
            vy = -8;
    }
    try_move_y();

    if (hit_kind(px, py, COL_HURT))
        hurt_player();
    if (state != ST_PLAY)
        return;
    if (hit_kind(px, py, COL_EXIT))
    {
        enter_state(ST_CLEAR);
        return;
    }

    if (px > 120)
        cam = px - 120;
    else
        cam = 0;
    if (cam > WORLD_W - 256)
        cam = WORLD_W - 256;
    bgSetScroll(1, cam, 0);

    sprx = (s16)px - 24 - (s16)cam;
    oamSet(0, (u16)sprx, py + HIT_H - 62, 3, flip, 0, hero * HERO_TILES, hero);
    oamSetEx(0, OBJ_LARGE, OBJ_SHOW);
    oamSetVisible(4, OBJ_HIDE);
    oamSetVisible(8, OBJ_HIDE);
    oamSetVisible(12, OBJ_HIDE);
    if (hurt_t)
        hurt_t--;
    update_rats();
}

int main(void)
{
    u16 pad, pressed;
    load_graphics();
    cursor = 0;
    hero = 0;
    prev_pad = 0;
    enter_state(ST_MENU);

    while (1)
    {
        pad = padsCurrent(0);
        pressed = pad & (u16)(~prev_pad);
        prev_pad = pad;

        if (state == ST_MENU)
        {
            if (pressed & KEY_UP)
            {
                cursor = 0;
                draw_menu_cursor();
            }
            else if (pressed & KEY_DOWN)
            {
                cursor = 1;
                draw_menu_cursor();
            }
            else if ((pressed & KEY_A) || (pressed & KEY_START))
            {
                if (cursor == 0)
                    enter_state(ST_SELECT);
                else
                    enter_state(ST_PASS);
            }
        }
        else if (state == ST_PASS)
        {
            if (pressed & KEY_B)
                enter_state(ST_MENU);
        }
        else if (state == ST_SELECT)
        {
            if (pressed & KEY_LEFT)
            {
                if (hero)
                    hero--;
                draw_select_cursor();
            }
            else if (pressed & KEY_RIGHT)
            {
                if (hero < 3)
                    hero++;
                draw_select_cursor();
            }
            else if ((pressed & KEY_A) || (pressed & KEY_START))
                enter_state(ST_BRIEF);
            else if (pressed & KEY_B)
                enter_state(ST_MENU);
            show_roster();
        }
        else if (state == ST_BRIEF)
        {
            if ((pressed & KEY_A) || (pressed & KEY_START))
                enter_state(ST_PLAY);
            else if (pressed & KEY_B)
                enter_state(ST_SELECT);
        }
        else if (state == ST_PLAY)
        {
            update_play(pad, pressed);
        }
        else if (state == ST_CLEAR)
        {
            if ((pressed & KEY_START) || (pressed & KEY_A))
                enter_state(ST_MENU);
        }

        WaitForVBlank();
    }
    return 0;
}
