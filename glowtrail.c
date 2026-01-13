#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <sys/ioctl.h>

static volatile sig_atomic_t run = 1;
static void stop(int s){ (void)s; run = 0; }

static void term_size(int *w, int *h){
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col && ws.ws_row){
        *w = (int)ws.ws_col; *h = (int)ws.ws_row;
    } else { *w = 80; *h = 24; }
}

static void rgb(int r,int g,int b){ printf("\x1b[38;2;%d;%d;%dm", r,g,b); }
static void reset(void){ printf("\x1b[0m"); }

static char shade(float v){
    if (v < 0.06f) return ' ';
    if (v < 0.12f) return '.';
    if (v < 0.22f) return ':';
    if (v < 0.36f) return '*';
    if (v < 0.52f) return 'o';
    if (v < 0.70f) return 'O';
    return '@';
}

int main(void){
    signal(SIGINT, stop);
    signal(SIGTERM, stop);

    setvbuf(stdout, NULL, _IONBF, 0);

    int W,H;
    term_size(&W,&H);

    printf("\x1b[?25l\x1b[2J");

    float *buf = calloc((size_t)W*H, sizeof(float));
    if(!buf) return 1;

    float x = W*0.2f, y = H*0.5f;
    float vx = 0.55f, vy = 0.28f;
    float t = 0.0f;

    while(run){
        int w2,h2;
        term_size(&w2,&h2);
        if(w2 != W || h2 != H){
            free(buf);
            W = w2; H = h2;
            buf = calloc((size_t)W*H, sizeof(float));
            printf("\x1b[2J"); 
        }

        for(int i=0;i<W*H;i++) buf[i] *= 0.92f;

        t += 0.06f;
        vx += 0.015f*sinf(t*0.7f);
        vy += 0.012f*cosf(t*0.9f);

        x += vx; y += vy;

        if(x < 2){ x = 2; vx = fabsf(vx); }
        if(x > W-3){ x = W-3; vx = -fabsf(vx); }
        if(y < 2){ y = 2; vy = fabsf(vy); }
        if(y > H-3){ y = H-3; vy = -fabsf(vy); }

        for(int dy=-2; dy<=2; dy++){
            for(int dx=-3; dx<=3; dx++){
                int px = (int)x + dx;
                int py = (int)y + dy;
                if(px<0||px>=W||py<0||py>=H) continue;

                float d = sqrtf((float)(dx*dx) + (float)(dy*dy*1.8f));
                float add = expf(-d*d*0.35f); 
                int idx = py*W + px;
                buf[idx] = fminf(1.0f, buf[idx] + 0.55f*add);
            }
        }

        printf("\x1b[H");

        for(int j=0;j<H;j++){
            for(int i=0;i<W;i++){
                float v = buf[j*W+i];

                int r = (int)(20 + 80*v);
                int g = (int)(40 + 180*v);
                int b = (int)(70 + 150*v);

                if(v > 0.65f){ r += 40; b += 30; }

                if(r>255) r=255; if(g>255) g=255; if(b>255) b=255;

                rgb(r,g,b);
                putchar(shade(v));
            }
            reset();
            putchar('\n');
        }

        reset();
        printf(" glowtrail — Ctrl+C to exit\n");

        usleep(16000);
    }

    reset();
    printf("\x1b[?25h\n");
    free(buf);
    return 0;
}
