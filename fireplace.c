#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <time.h>

static volatile sig_atomic_t run = 1;
static void stop(int s){ (void)s; run = 0; }

static void term_size(int *w, int *h){
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col && ws.ws_row){
        *w = (int)ws.ws_col;
        *h = (int)ws.ws_row;
    } else {
        *w = 80;
        *h = 24;
    }
}

static inline void reset(void){ printf("\x1b[0m"); }
static inline void rgb(int r,int g,int b){ printf("\x1b[38;2;%d;%d;%dm", r,g,b); }

static unsigned int seedv = 1u;
static inline unsigned int rnd(void){
    seedv = seedv * 1103515245u + 12345u;
    return seedv;
}

static inline int clampi(int x, int a, int b){
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

static void draw_cell(unsigned char heat){
    char c;
    if (heat < 20) c = ' ';
    else if (heat < 50) c = '.';
    else if (heat < 90) c = ':';
    else if (heat < 140) c = '*';
    else if (heat < 200) c = 'o';
    else c = '#';

    int r,g,b;
    if (heat < 60) {
        r = heat * 2;
        g = heat / 4;
        b = 0;
    } else if (heat < 140) {
        r = 180 + (heat - 60) * 1;
        g = 30  + (heat - 60) * 1;
        b = 0;
    } else if (heat < 220) {
        r = 255;
        g = 80  + (heat - 140) * 2;
        b = 10  + (heat - 140) / 3;
    } else {
        r = 255;
        g = 240;
        b = 180;
        c = '@';
    }

    r = clampi(r,0,255);
    g = clampi(g,0,255);
    b = clampi(b,0,255);

    rgb(r,g,b);
    putchar(c);
}

static void sleep_ms(int ms){
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

int main(void){
    signal(SIGINT, stop);
    signal(SIGTERM, stop);

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    seedv ^= (unsigned int)ts.tv_nsec ^ (unsigned int)ts.tv_sec;

    setvbuf(stdout, NULL, _IONBF, 0);

    int W,H;
    term_size(&W,&H);

    printf("\x1b[?1049h\x1b[?25l\x1b[2J\x1b[H");

    int fire_h = (H > 6) ? (H - 3) : H;
    int fire_w = W;

    unsigned char *heat = calloc((size_t)fire_w * (size_t)fire_h, 1);
    unsigned char *next = calloc((size_t)fire_w * (size_t)fire_h, 1);
    if(!heat || !next){
        printf("\x1b[?25h\x1b[?1049l");
        free(heat); free(next);
        return 1;
    }

    while(run){
        int w2,h2;
        term_size(&w2,&h2);
        if(w2 != W || h2 != H){
            W = w2; H = h2;
            printf("\x1b[2J\x1b[H");

            fire_h = (H > 6) ? (H - 3) : H;
            fire_w = W;

            free(heat); free(next);
            heat = calloc((size_t)fire_w * (size_t)fire_h, 1);
            next = calloc((size_t)fire_w * (size_t)fire_h, 1);
            if(!heat || !next) break;
        }

        for(int x=0; x<fire_w; x++){
            unsigned char v = 0;
            unsigned int r = rnd() & 1023u;

            int cx = fire_w / 2;
            int dist = (x > cx) ? (x - cx) : (cx - x);
            int center_boost = (dist < fire_w/3) ? (80 - dist*2) : 0;
            if(center_boost < 0) center_boost = 0;

            if(r > 650) v = (unsigned char)(180 + (rnd() % 75));
            else if(r > 450) v = (unsigned char)(110 + (rnd() % 70));
            else v = (unsigned char)(20 + (rnd() % 40));

            int bottom = (fire_h - 1) * fire_w + x;
            heat[bottom] = (unsigned char)clampi((int)v + center_boost, 0, 255);
        }

        for(int y=0; y<fire_h-1; y++){
            for(int x=0; x<fire_w; x++){
                int below = (y+1)*fire_w + x;

                int xl = (x > 0) ? x-1 : x;
                int xr = (x < fire_w-1) ? x+1 : x;

                int a = heat[below];
                int b = heat[(y+1)*fire_w + xl];
                int c = heat[(y+1)*fire_w + xr];
                int d = heat[(y+2 < fire_h ? y+2 : y+1)*fire_w + x];

                int avg = (a + b + c + d) / 4;

                int drift = (int)(rnd() % 3) - 1;
                int nx = x + drift;
                if(nx < 0) nx = 0;
                if(nx > fire_w-1) nx = fire_w-1;

                int out = avg - (int)(rnd() % 12);
                if(out < 0) out = 0;

                next[y*fire_w + nx] = (unsigned char)out;
            }
        }

        for(int x=0; x<fire_w; x++){
            next[(fire_h-1)*fire_w + x] = heat[(fire_h-1)*fire_w + x];
        }

        unsigned char *tmp = heat; heat = next; next = tmp;

        printf("\x1b[H");

        for(int y=0; y<fire_h; y++){
            for(int x=0; x<fire_w; x++){
                draw_cell(heat[y*fire_w + x]);
            }
            reset();
            putchar('\n');
        }

        if(H >= 3){
            for(int x=0; x<W; x++){
                int glow = 40 + (int)(rnd() % 90);
                rgb(120 + glow, 25 + glow/4, 0);
                putchar((x % 7 == 0) ? '=' : '-');
            }
            reset();
            putchar('\n');

            for(int x=0; x<W; x++){
                rgb(120, 70, 30);
                putchar((x % 11 == 0) ? '#' : '_');
            }
            reset();
            putchar('\n');
        }

        sleep_ms(16);
    }

    reset();
    printf("\x1b[?25h\x1b[?1049l\n");
    free(heat);
    free(next);
    return 0;
}
