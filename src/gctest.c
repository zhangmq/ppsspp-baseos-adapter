/* gctest - check whether SDL recognizes a joystick as a game controller.
 * Reconstructed 2026-08-15 from session notes (the original one-off was
 * not archived): the on-device check was "SDL_IsGameController(0) == true"
 * with a gamecontrollerdb loaded via SDL_GAMECONTROLLERCONFIG_FILE, proving
 * the crafted entry maps under the given SDL build's index scheme.
 */
#include <SDL2/SDL.h>
#include <stdio.h>

int main(void) {
    if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        return 1;
    }
    int n = SDL_NumJoysticks();
    printf("SDL_NumJoysticks = %d\n", n);
    for (int i = 0; i < n; i++) {
        int is_gc = SDL_IsGameController(i);
        printf("js[%d] name='%s' is_game_controller=%d\n",
               i, SDL_JoystickNameForIndex(i), is_gc);
        if (is_gc) {
            SDL_GameController *c = SDL_GameControllerOpen(i);
            printf("  opened=%s mapping='%s'\n",
                   c ? "yes" : "no", SDL_GameControllerMapping(c));
            if (c) SDL_GameControllerClose(c);
        }
    }
    SDL_Quit();
    return 0;
}
