/* jsdump - dump SDL2 joystick enumeration + live button presses.
 * Compiled against the device SDL2 (mali build), run with the same env the
 * pak session uses (SDL_JOYSTICK_DISABLE_UDEV=1 etc.) so the joystick view
 * matches what PPSSPP sees. Dummy video driver: no display grab.
 */
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>

#define MAXJS 8
#define MAXBTN 128

int main(void) {
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
    if (SDL_Init(SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        return 1;
    }
    int n = SDL_NumJoysticks();
    printf("SDL_NumJoysticks = %d\n", n);
    SDL_Joystick *js[MAXJS];
    int prev[MAXJS][MAXBTN];
    memset(prev, 0, sizeof(prev));
    int nj = 0;
    for (int i = 0; i < n && nj < MAXJS; i++) {
        SDL_Joystick *j = SDL_JoystickOpen(i);
        if (!j) {
            printf("js[%d] open failed: %s\n", i, SDL_GetError());
            continue;
        }
        js[nj] = j;
        int nb = SDL_JoystickNumButtons(j);
        int nh = SDL_JoystickNumHats(j);
        int na = SDL_JoystickNumAxes(j);
        char guid[64];
        SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(j), guid, sizeof(guid));
        printf("js[%d] name='%s' guid=%s buttons=%d hats=%d axes=%d\n",
               i, SDL_JoystickName(j), guid, nb, nh, na);
        if (nb > MAXBTN) nb = MAXBTN;
        nj++;
    }
    if (!nj) {
        printf("no joystick; also scanning /dev/input: ");
        fflush(stdout);
        return 2;
    }
    /* 90s of polling: report button/hat/axis changes as <js> <type> <idx> <val> */
    SDL_JoystickUpdate();
    for (int t = 0; t < 90 * 20; t++) {
        SDL_Delay(50);
        SDL_JoystickUpdate();
        for (int k = 0; k < nj; k++) {
            SDL_Joystick *j = js[k];
            int nb = SDL_JoystickNumButtons(j);
            if (nb > MAXBTN) nb = MAXBTN;
            for (int b = 0; b < nb; b++) {
                Uint8 v = SDL_JoystickGetButton(j, b);
                if (v != prev[k][b]) {
                    prev[k][b] = v;
                    printf("js%d BTN %d %s\n", k, b, v ? "PRESSED" : "released");
                    fflush(stdout);
                }
            }
            int nh = SDL_JoystickNumHats(j);
            for (int h = 0; h < nh; h++) {
                Uint8 v = SDL_JoystickGetHat(j, h);
                if (v != 0)
                    printf("js%d HAT %d value=%d\n", k, h, v);
            }
            int na = SDL_JoystickNumAxes(j);
            for (int a = 0; a < na; a++) {
                Sint16 v = SDL_JoystickGetAxis(j, a);
                if (v < -16384 || v > 16384)
                    printf("js%d AXIS %d value=%d\n", k, a, v);
            }
        }
    }
    return 0;
}
