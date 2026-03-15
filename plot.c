#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "tinyexpr/tinyexpr.h"

#define X_ZERO 400
#define Y_ZERO 300

void draw_at_grid_coordinates(SDL_Surface *psurface, SDL_Rect *obj, uint32_t color){
    SDL_Rect obj_from_origin;
    obj_from_origin.x = X_ZERO + obj->x;
    obj_from_origin.y = Y_ZERO - obj->y;
    obj_from_origin.w = obj->w;
    obj_from_origin.h = obj->h;

    SDL_FillRect(psurface, &obj_from_origin, color);
}

void draw_grid(SDL_Surface *psurface){
    int HEIGHT = 600;
    int WIDTH = 800;
    SDL_Rect x_axis = {-WIDTH/2, 0, WIDTH, 2};
    SDL_Rect y_axis = {0, HEIGHT/2, 2, HEIGHT};

    draw_at_grid_coordinates(psurface, &x_axis, 0x888888);
    draw_at_grid_coordinates(psurface, &y_axis, 0x888888);
} 

struct Segment {
    int a[2]; // posición xy inicio
    int b[2]; // posicion xy final
};

void draw_segment(struct Segment *segment, SDL_Surface *psurface, uint32_t color){
    if((segment->b[0] == segment->a[0]) && (segment->b[1] == segment->a[1])){ // segmento A = B => punto en el plano 
        SDL_Rect pixel = {segment->a[0], segment->a[1], 1, 1};
        draw_at_grid_coordinates(psurface, &pixel, color);
        return;
    }

    float norma = sqrt(pow(segment->b[0] - segment->a[0], 2)
        + pow(segment->b[1] - segment->a[1], 2));
    
    int *p = segment->a; // punto de paso

    int v_dir[2]; 
    v_dir[0] = segment->b[0] - segment->a[0]; // v_x
    v_dir[1] = segment->b[1] - segment->a[1]; // v_y

    for(float i = 0; i < 1; i += 1/norma){ 
        int v_x = v_dir[0] * i + p[0];
        int v_y = v_dir[1] * i + p[1];
        SDL_Rect pixel = {v_x, v_y, 1, 1};

        draw_at_grid_coordinates(psurface, &pixel, color);
    }
}

double segment_scope(struct Segment *segment){
    double b0 = segment->b[0]; 
    if(segment->b[0] == segment->a[0]) // salvando indeterminacion localoca
        b0 += 0.1;

    double m = (segment->b[1] - segment->a[1]) / (double)(b0 - segment->a[0]);
    
    return abs(m); 
}

void draw_function(SDL_Surface *psurface, te_expr *expr, double *x, int scale_x, int scale_y){
    SDL_Rect anterior = {0, 0, 1, 1}; 
    bool discontinuidad = false; 

    for(float i = -400; i < 400; i+=0.01){
        *x = i / scale_x;
        double r = te_eval(expr);
        
        if(isnan(r) || isinf(r)){
            discontinuidad = true;
            continue;
        }

        SDL_Rect point = {i, (1 * scale_y) * r, 1, 1};

        // evalua discontinuidad pasada
        if(discontinuidad){
            anterior = (SDL_Rect){i, (1 * scale_y) * r, 1, 1};
            discontinuidad = false;
        }
        
        if(i == -400){
            anterior = point;
        }
        
        struct Segment segment = {
            .a = {anterior.x, anterior.y},
            .b = {point.x, point.y}
        };

        double m = segment_scope(&segment);
        if(m > 5000){
            discontinuidad = true;
            anterior = point;
            continue;
        }
        
        draw_segment(&segment, psurface, 0x00FF00);
        anterior = point; 
    }
}

int main(int argc, char *argv[]){
    
    // CLI
    if(argc != 4){
        printf("Usage: example2 \"expression\" x_scale y_scale\n");
        return 0;
    }
    int scale_x = atoi(argv[2]);
    int scale_y = atoi(argv[3]);
    if(scale_x == 0  || scale_y == 0){
        printf("Invalid x or y range!\n"); 
        return 0;
    }
    
    double x;
    te_variable vars[] = {{"x", &x}};
    int err;

    te_expr *expr = te_compile(argv[1], vars, 1, &err);

    if(!expr){
        printf("Expression error\n");
        return 0;
    }
    

    // SDL context 
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *pwindow = SDL_CreateWindow(
        "plot",
        0, 0 ,
        800, 600, 0
    );
    
    SDL_Surface *psurface = SDL_GetWindowSurface(pwindow);
    

    // LOOP
    while(1) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT)
                return 0;
            
            if(event.type == SDL_MOUSEWHEEL){
                int y = event.wheel.y; 
                scale_x += y;
                scale_y += y;
            }
        }
        
        

        SDL_FillRect(psurface, NULL, 0x000000);
        draw_grid(psurface);
     
        draw_function(psurface, expr, &x, scale_x, scale_y);
             
        SDL_UpdateWindowSurface(pwindow);
        SDL_Delay(5);
    }

    te_free(expr);
    SDL_DestroyWindow(pwindow);
    SDL_Quit();
    return 0;
}
