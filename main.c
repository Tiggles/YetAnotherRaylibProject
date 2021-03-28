#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>

#define MODE_2D(BLOCK) BeginMode2D(*camera);\
    BLOCK \
EndMode2D();

#define DRAW(BLOCK) BeginDrawing();\
    BLOCK \
EndDrawing();

enum BuildingType {
    House,
    Lumbermill
};

typedef struct Textures {
    Texture2D house;
    Texture2D lumbermill;
} Textures;

typedef struct Resources {
    int gold; 
    int wood;
    int electricity;
} Resources;

typedef struct Building {
    int x;
    int y;
    union buildingData {
        int inhabitants;
        int workers;
    };
    enum BuildingType type;
    int repairState;
} Building;

typedef struct Message {
    char *message;
    float timeRemaining;
} Message;

#define HOUSES_PER_WRAPPER 8
typedef struct BuildingWrapper {
    Rectangle coordinates;
    Building **buildings;
    struct BuildingWrapper *next;
} BuildingWrapper;

Building* initBuilding(int x, int y, enum BuildingType type);
bool isRectangleColliding(Rectangle rect1, Rectangle rect2);
BuildingWrapper* isAnyColliding(BuildingWrapper *housesWrapper,  Vector2 point, int houseWidth, int houseHeight);
void drawBuildings(BuildingWrapper *housesWrapper, Vector2 cameraPosition, Textures *textures);
void addBuilding(BuildingWrapper *housesWrapper, Vector2 position, enum BuildingType type);
void input(Camera2D *camera, float delta, Message *message, BuildingWrapper *housesWrapper, int houseWidth, int houseHeight, enum BuildingType *type);
void update(Message *message, float delta);
void render(Camera2D *camera, Textures *textures, BuildingWrapper *housesWrapper, Message *message, enum BuildingType type);
void freeHouseWrappers(BuildingWrapper *housesWrapper);

int wrappersAmount = 0;
int screenWidth = 800;
int screenHeight = 450;
int debug = 0;

int main() {
    // Initialization
    //--------------------------------------------------------------------------------------

    enum BuildingType buildingType = House;

    InitWindow(screenWidth, screenHeight, "raylib");

    Message message;
    message.message = malloc(sizeof(char) * 256);
    message.timeRemaining = 0;

    Camera2D camera;
    camera.offset.x = camera.offset.y = 0;
    camera.rotation = 0;
    camera.target = (Vector2){0, 0};
    camera.zoom = 1;

    BuildingWrapper buildingsWrapper;
    buildingsWrapper.next = NULL;
    buildingsWrapper.coordinates.x = buildingsWrapper.coordinates.y = 0;
    buildingsWrapper.coordinates.width = screenWidth;
    buildingsWrapper.coordinates.height = screenHeight;
    buildingsWrapper.buildings = malloc(sizeof(Building*) *HOUSES_PER_WRAPPER); // TODO free

    for (int i = 0; i < HOUSES_PER_WRAPPER; i++) buildingsWrapper.buildings[i] = NULL;

    Textures textures;
    textures.house = LoadTexture("Assets/house.png");
    textures.lumbermill = LoadTexture("Assets/lumbermill_outline.png");
    int houseHeight = textures.house.height / 4;
    int houseWidth = textures.house.width / 4;

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    int x = 0;
    int y = 0;

    while (!WindowShouldClose()) {
        float delta = GetFrameTime();
        input(&camera, delta, &message, &buildingsWrapper, houseWidth, houseHeight, &buildingType);
        update(&message, delta);
        render(&camera, &textures, &buildingsWrapper, &message, buildingType);
    }
    freeHouseWrappers(&buildingsWrapper);
    CloseWindow();
    return 0;
}

int freeCount = 0;
void freeHouseWrappers(BuildingWrapper *buildingsWrapper) {
    free(buildingsWrapper->buildings);
    freeCount++;
    BuildingWrapper *wrapper = buildingsWrapper->next;

    while (wrapper != NULL) {
        free(wrapper->buildings);
        BuildingWrapper *next = wrapper->next;
        free(wrapper);
        wrapper = next;
        freeCount++;
    }

    printf("Freed %i wrappers and arrays of houses.\n", freeCount);
}

void update(Message *message, float delta) {
    if (message->timeRemaining > 0) message->timeRemaining -= delta;
}

void render(Camera2D *camera, Textures *textures, BuildingWrapper *housesWrapper, Message *message, enum BuildingType type) {
    DRAW(
        ClearBackground((Color){70, 149, 75});

        MODE_2D(
            drawBuildings(housesWrapper, (Vector2){0, 0}, textures);
        );
        Vector2 mouse = GetMousePosition(); 
        Vector2 pointerPosition = GetScreenToWorld2D(mouse, *camera);
        DrawCircle(mouse.x, mouse.y, 10, RED);

        DrawFPS(10, 10);

        DrawRectangle(screenWidth * 0.75f, 0, screenWidth / 4 , 30, WHITE);
        if (message->timeRemaining > 0) {
            DrawText(message->message, mouse.x, mouse.y, 16, RED);
        }
        char buff[256];
        sprintf(buff, "Wrappers: %i", wrappersAmount);
        DrawText(buff, 2, 2, 20, WHITE);
        sprintf(buff, "%f, %f", pointerPosition.x, pointerPosition.y);
        DrawText(buff, 10, 25, 20, WHITE);
        sprintf(buff, "Type: %i", type);
        DrawText(buff, 10, 40, 20, WHITE);
    );
}

void input(Camera2D *camera, float delta, Message *message, BuildingWrapper *buildingWrapper, int houseWidth, int houseHeight, enum BuildingType *type) {
    SetMouseOffset(camera->offset.x, camera->offset.y);
    Vector2 mouse = GetMousePosition();

    Vector2 pointerPosition = GetScreenToWorld2D(mouse, *camera);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (!isAnyColliding(buildingWrapper, pointerPosition, houseWidth, houseHeight)) {
            addBuilding(buildingWrapper, pointerPosition, *type);
        } else {
            message->message = "Cannot place building. Collides with other house.";
            message->timeRemaining = 5;
        }
    }


    int scrollSpeed = IsKeyDown(KEY_LEFT_SHIFT) ? 250 : 150;

    if (IsKeyDown(KEY_W)) {
        camera->target.y -= delta * scrollSpeed;
    } else if (IsKeyDown(KEY_S)) {
        camera->target.y += delta * scrollSpeed;
    }

    if (IsKeyDown(KEY_A)) {
        camera->target.x -= delta * scrollSpeed;
    } else if (IsKeyDown(KEY_D)) {
        camera->target.x += delta * scrollSpeed;
    }
        
    if (IsKeyPressed(KEY_R)) {
        camera->target.x = camera->target.y = 0;
        camera->zoom = 1;
    }

    if (IsKeyPressed(KEY_F1)) {
        debug = !debug;
    }

    if (IsKeyPressed(KEY_E)) {
        if (*type == House) {
            *type = Lumbermill;
        } else {
            *type = House;
        }
    }

    if (IsKeyPressed(KEY_Q)) {

        if (*type == House) {
            *type = Lumbermill;
        } else {
            *type = House;
        }
    }

    float mouseWheel = GetMouseWheelMove();
    camera->zoom += mouseWheel / 10;
}


inline Building* initBuilding(int x, int y, enum BuildingType type) {
    Building *building = malloc(sizeof(Building));
    building->x = x;
    building->y = y;
    building->type = type;
    building->repairState = 100;
    
    return building;
}

void addBuilding(BuildingWrapper *housesWrapper, Vector2 position, enum BuildingType type) {
    int emptySlot = -1;
    BuildingWrapper *wrapper = housesWrapper;
    while (wrapper != NULL) {
        if (CheckCollisionPointRec(position, wrapper->coordinates)) {
            for (int i = 0; i < HOUSES_PER_WRAPPER; i++) {
                if (wrapper->buildings[i] == NULL) {
                    wrapper->buildings[i] = initBuilding(position.x, position.y, type);
                    return;
                }
            } 
        }

        if (wrapper->next == NULL) {            
            wrapper->next = malloc(sizeof(BuildingWrapper));
            wrapper = wrapper->next;
            wrapper->next = NULL;
            wrapper->coordinates.x = housesWrapper->coordinates.x; // TODO
            wrapper->coordinates.y = housesWrapper->coordinates.y; // TODO
            wrapper->coordinates.width = housesWrapper->coordinates.width; // TODO
            wrapper->coordinates.height = housesWrapper->coordinates.height; // TODO
            wrapper->buildings = malloc(sizeof(Building*) * HOUSES_PER_WRAPPER);
            for (int i = 1; i < HOUSES_PER_WRAPPER; i++) wrapper->buildings[i] = NULL;
            wrapper->buildings[0] = initBuilding(position.x, position.y, type);
            return;
        }

        wrapper = wrapper->next;
    };
}

void drawBuildings(BuildingWrapper *housesWrapper, Vector2 cameraPosition, Textures *textures) {
    BuildingWrapper *wrapper = housesWrapper;
    wrappersAmount = 0;
    while (wrapper != NULL) {
        if (CheckCollisionRecs(
            (Rectangle) {wrapper->coordinates.x, wrapper->coordinates.y, wrapper->coordinates.width, wrapper->coordinates.width},
            (Rectangle) {cameraPosition.x, cameraPosition.y, wrapper->coordinates.width, wrapper->coordinates.width}
        )) {
            for (int i = 0; i < HOUSES_PER_WRAPPER; i++) {
                if (wrapper->buildings[i] != NULL){
                    switch (wrapper->buildings[i]->type)
                    {
                    case House:
                        DrawTextureEx(textures->house, (Vector2){wrapper->buildings[i]->x, wrapper->buildings[i]->y}, 0, 0.25, WHITE);  
                        break;
                    case Lumbermill:
                        DrawTextureEx(textures->lumbermill, (Vector2){wrapper->buildings[i]->x, wrapper->buildings[i]->y}, 0, 0.25, WHITE);  
                    default:
                        break;
                    }
                }
            }
        }
        wrapper = wrapper->next;
        wrappersAmount++;
    }
}

inline BuildingWrapper* isAnyColliding(BuildingWrapper *housesWrapper,  Vector2 point, int houseWidth, int houseHeight) {
    return false;
}
