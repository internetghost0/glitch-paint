#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>





#define RED 0xFF0000FF
#define GREEN 0x00FF00FF
#define BLUE 0X0000FFFF
#define BLACK 0x000000FF
#define WHITE 0xFFFFFFFF

#define FPS 10
#define MAX_LINES (1024*10)

struct {
    int x = 0;
    int y = 0;
} Coord;

using std::cerr;
using std::endl;

uint32_t WIDTH = 800;
uint32_t HEIGHT = 600;

#define HEX_COLOR(hex)                                                         \
    ((hex) >> (3 * 8)) & 0xFF, ((hex) >> (2 * 8)) & 0xFF,                      \
        ((hex) >> (1 * 8)) & 0xFF, ((hex) >> (0 * 8)) & 0xFF


void sdl_check(int code) {
    if (code != 0) {
        std::cerr << "SDL_Error: " << SDL_GetError() << std::endl;
        exit(code);
    }
}
template <typename T> T *sdl_check(T *ptr) {
    if (ptr == nullptr) {
        std::cerr << "SDL_Error: " << SDL_GetError() << std::endl;
        exit(-1);
    } else
        return ptr;
}
void dump_lines_to_file(const char* fn, SDL_Point* points, int count);
int load_lines_from_file(const char* fn, SDL_Point* points, int max=-1);

int main(int argc, char** argv) {
    if (argc == 3) {
        WIDTH =  std::stoi(argv[1]);
        HEIGHT = std::stoi(argv[2]);
    } else if (argc > 1) {
        std::cout << argv[0] << " width height\n";
    }


    sdl_check(SDL_Init(SDL_INIT_EVERYTHING));

    SDL_Window *window = sdl_check(SDL_CreateWindow(
        "dead inside", 0, 0, WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE));
    SDL_Renderer *renderer = sdl_check(SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));

    sdl_check(SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT));

    bool running = true;
    uint32_t background_color = BLACK;
    uint32_t foreground_color = GREEN;
    bool is_background = 0;
    
    SDL_Point points[MAX_LINES] = {0};
    int count = 0;
    count = load_lines_from_file("./maps/save0.bin", points, MAX_LINES);
    // SDL_Delay(1000);
    bool glitch_mode = true;
    while (running) {
        SDL_SetRenderDrawColor(renderer, HEX_COLOR(background_color));
        sdl_check(SDL_RenderClear(renderer));
        if (glitch_mode) {
            if (foreground_color == RED)
                foreground_color = GREEN;
            else if (foreground_color == GREEN)
                foreground_color = BLUE;
            else if (foreground_color == BLUE)
                foreground_color = RED;
        }

        sdl_check(
            SDL_SetRenderDrawColor(renderer, HEX_COLOR(foreground_color)));

        for (int i = 0; i < count - 1; i++) {
            if (points[i].x * points[i].y == 0 ||
                points[i + 1].x * points[i + 1].y == 0) {
                continue;
            }
            sdl_check(SDL_RenderDrawLine(renderer, points[i].x, points[i].y,
                                         points[i + 1].x, points[i + 1].y));
        }
        SDL_RenderPresent(renderer);

        SDL_Delay((1.0f/FPS)*1000.0f);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    running = false;
                    break;
                case SDLK_F12: {// screenshot
                    const char* fn = "screenshot0.bmp";
                    std::cout << "Saving screenshot `" << fn << "`... ";
                    std::fflush(stdout);
                    int w,h;
                    sdl_check(SDL_GetRendererOutputSize(renderer, &w, &h));
                    SDL_Surface *sshot = SDL_CreateRGBSurface(
                        0, w, h, 32, 0x00ff0000, 0x0000ff00,
                        0x000000ff, 0xff000000);
                    SDL_RenderReadPixels(renderer, NULL,
                                         SDL_PIXELFORMAT_ARGB8888,
                                         sshot->pixels, sshot->pitch);
                    SDL_SaveBMP(sshot, fn);
                    SDL_FreeSurface(sshot);
                    SDL_Delay(500);
                    std::cout << "finished." << std::endl;
                }break;
                case SDLK_F5: {
                    dump_lines_to_file("./maps/save0.bin", points, count);
                }break;
                case SDLK_F9: {
                    count = load_lines_from_file("./maps/save0.bin", points, MAX_LINES);
                    SDL_Delay(1000);
                }break;
                case SDLK_q:
                    if (is_background)
                        background_color = RED;
                    else
                        foreground_color = RED;
                    break;
                case SDLK_w:
                    if (is_background)
                        background_color = GREEN;
                    else
                        foreground_color = GREEN;
                    break;
                case SDLK_e:
                    if (is_background)
                        background_color = BLUE;
                    else
                        foreground_color = BLUE;
                    break;
                case SDLK_r:
                    if (is_background)
                        background_color = WHITE;
                    else
                        foreground_color = WHITE;
                    break;
                case SDLK_t:
                    if (is_background)
                        background_color = BLACK;
                    else
                        foreground_color = BLACK;
                    break;
                case SDLK_g:
                    glitch_mode = 1 - glitch_mode;
                    foreground_color = GREEN;
                    break;
                case SDLK_SPACE:
                    if (count < MAX_LINES && count > 0) {
                        if (points[count-1].x * points[count-1].y != 0) {
                            points[count++] = {0};
                        }
                            
                    }
                    break;
                case SDLK_c:
                    bzero(points, sizeof(points));
                    count = 0;
                    break;
                case SDLK_v:
                    is_background = 1 - is_background;
                    break;
                case SDLK_z:
                case SDLK_u: // undo
                    if (count > 0)
                        count--;
                    break;
                case SDLK_x: // reverse undo
                case SDLK_b:
                    if (count < MAX_LINES)
                        count++;
                    break;
                }

                break;
            case SDL_MOUSEBUTTONDOWN:
                if (count >= MAX_LINES) {
                    bzero(points, sizeof(points));
                    count = 0;
                }
                SDL_Point p{event.button.x, event.button.y};
                points[count++] = p;
                break;
            }
        }
    }

    // SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}


void dump_lines_to_file(const char* fn, SDL_Point* points, int count) {
    try {
        std::cout << "Saving to binary... ";
        std::fflush(stdout);
        std::ofstream file(fn, std::ios::binary | std::ios::out);
        file.write((char*)&count, sizeof(int));
        file.write((char*)points, sizeof(SDL_Point)*count);
        file.close();
        std::cout << "Successfull\n";
    } catch (const std::exception& e){
        std::cerr << "Failed to dump into file...\n";
        std::cerr << e.what();
    }

}
int load_lines_from_file(const char* fn, SDL_Point* points, int max) {
    try {
        int count = 0;
        std::cout << "Loading from binary... ";
        std::fflush(stdout);
        std::ifstream file(fn, std::ios::binary | std::ios::in);
        file.read((char*)&count, sizeof(int));
        if (max >= 0 && count > max) {
            count = max;
        }
        file.read((char*)points, sizeof(SDL_Point)*count);
        file.close();
        std::cout << "Successfull\n";
        return count;
    } catch (...){
        std::cerr << "Failed to read from file...\n";
        
    }
    return 0;
}
