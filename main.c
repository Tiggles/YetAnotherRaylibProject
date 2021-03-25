#include "raylib.h"
#include <stdlib.h>

#define MODE_2D(BLOCK) BeginMode2D(camera);\
    BLOCK \
EndMode2D();

#define DRAW(BLOCK) BeginDrawing();\
    BLOCK \
EndDrawing();

typedef struct Resources {
    int gold; 
    int wood;
    int electricity;
} Resources;

typedef struct House {
    int x;
    int y;
    int inhabitants;
    int repairState;
} House;

typedef struct Message {
    char *message;
    float timeRemaining;
} Message;

#define HOUSES_PER_WRAPPER 8
typedef struct HouseWrapper {
    Rectangle coordinates;
    House **houses;
    struct HouseWrapper *next;
} HouseWrapper;

struct House* initHouse(int x, int y);
bool isRectangleColliding(Rectangle rect1, Rectangle rect2);
HouseWrapper* isAnyColliding(HouseWrapper *housesWrapper,  Vector2 point, int houseWidth, int houseHeight);
void drawHouses(HouseWrapper *housesWrapper, Vector2 cameraPosition, Texture2D *house);
void addHouse(HouseWrapper *housesWrapper, Vector2 position);
void input(Camera2D *camera, float delta, Message *message, HouseWrapper *housesWrapper, int houseWidth, int houseHeight);
void update(Message *message, float delta);
void render(void);

int wrappersAmount = 0;
int screenWidth = 800;
int screenHeight = 450;

int main() {
    // Initialization
    //--------------------------------------------------------------------------------------


    InitWindow(screenWidth, screenHeight, "raylib");

    Message message;
    message.message = malloc(sizeof(char) * 256);
    message.timeRemaining = 0;

    Camera2D camera;
    camera.offset.x = camera.offset.y = 0;
    camera.rotation = 0;
    camera.target = (Vector2){0, 0};
    camera.zoom = 1;

    HouseWrapper housesWrapper;
    housesWrapper.next = NULL;
    housesWrapper.coordinates.x = housesWrapper.coordinates.y = 0;
    housesWrapper.coordinates.width = screenWidth;
    housesWrapper.coordinates.height = screenHeight;
    housesWrapper.houses = malloc(sizeof(House*) *HOUSES_PER_WRAPPER); // TODO free

    for (int i = 0; i < HOUSES_PER_WRAPPER; i++) housesWrapper.houses[i] = NULL;

    Texture2D house = LoadTexture("Assets/house.png");
    int houseHeight = house.height / 4;
    int houseWidth = house.width / 4;

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    int x = 0;
    int y = 0;

    while (!WindowShouldClose()) {
        float delta = GetFrameTime();
        input(&camera, delta, &message, &housesWrapper, houseWidth, houseHeight);
        update(&message, delta);
        DRAW(

            ClearBackground((Color){70, 149, 75});
            render();

            MODE_2D(
                drawHouses(&housesWrapper, camera.offset, &house);
            );
            Vector2 mouse = GetMousePosition(); 
            Vector2 pointerPosition = GetScreenToWorld2D(mouse, camera);
            DrawCircle(pointerPosition.x, pointerPosition.y, 10, RED);

            DrawFPS(10, 10);

            DrawRectangle(screenWidth * 0.75f, 0, screenWidth / 4 , 30, WHITE);
            if (message.timeRemaining > 0) {
                DrawText(message.message, pointerPosition.x, pointerPosition.y, 16, RED);
            }
            char buff[10];
            DrawText("Wrappers: ", 2, 2, 20, WHITE);
            DrawText(itoa(wrappersAmount, buff, 10), 108, 2, 20, WHITE);
            DrawText(itoa(mouse.x, buff, 10), 10, 25, 20, WHITE);
            DrawText(itoa(mouse.y, buff, 10), 10, 40, 20, WHITE);
        );
    }
    CloseWindow(); // Close window and OpenGL context
    return 0;
}

void update(Message *message, float delta) {
    if (message->timeRemaining > 0) message->timeRemaining -= delta;
}

void render() {

}

void input(Camera2D *camera, float delta, Message *message, HouseWrapper *housesWrapper, int houseWidth, int houseHeight) {
    Vector2 mouse = GetMousePosition();
    SetMouseOffset(camera->offset.x, camera->offset.y);

    Vector2 pointerPosition = GetScreenToWorld2D(mouse, *camera);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (!isAnyColliding(housesWrapper, pointerPosition, houseWidth, houseHeight)) {
            addHouse(housesWrapper, pointerPosition);
        } else {
            message->message = "Cannot place building. Collides with other house.";
            message->timeRemaining = 5;
        }
    }


    int scrollSpeed = IsKeyDown(KEY_LEFT_SHIFT) ? 250 : 150;

    if (IsKeyDown(KEY_W)) {
        camera->offset.y += delta * scrollSpeed;
    } else if (IsKeyDown(KEY_S)) {
        camera->offset.y -= delta * scrollSpeed;
    }

    if (IsKeyDown(KEY_A)) {
        camera->offset.x += delta * scrollSpeed;
    } else if (IsKeyDown(KEY_D)) {
        camera->offset.x -= delta * scrollSpeed;
    }
        
    if (IsKeyPressed(KEY_R)) {
        camera->offset.x = camera->offset.y = 0;
        camera->zoom = 1;
    }

    float mouseWheel = GetMouseWheelMove();
    camera->zoom += mouseWheel / 10;
}


inline House* initHouse(int x, int y) {
    House *house = malloc(sizeof(House));
    house->x = x;
    house->y = y;
    house->inhabitants = 0;
    house->repairState = 100;
    return house;
}

void addHouse(HouseWrapper *housesWrapper, Vector2 position) {
    int emptySlot = -1;
    HouseWrapper *wrapper = housesWrapper;
    while (wrapper != NULL) {
        if (CheckCollisionPointRec(position, wrapper->coordinates)) {
            for (int i = 0; i < HOUSES_PER_WRAPPER; i++) {
                if (wrapper->houses[i] == NULL) {
                    wrapper->houses[i] = initHouse(position.x, position.y);
                    return;
                }
            } 
        }

        if (wrapper->next == NULL) {            
            wrapper->next = malloc(sizeof(HouseWrapper));
            wrapper = wrapper->next;
            wrapper->next = NULL;
            wrapper->coordinates.x = housesWrapper->coordinates.x; // TODO
            wrapper->coordinates.y = housesWrapper->coordinates.y; // TODO
            wrapper->coordinates.width = housesWrapper->coordinates.width; // TODO
            wrapper->coordinates.height = housesWrapper->coordinates.height; // TODO
            wrapper->houses = malloc(sizeof(House*) * HOUSES_PER_WRAPPER);
            for (int i = 1; i < HOUSES_PER_WRAPPER; i++) wrapper->houses[i] = NULL;
            wrapper->houses[0] = initHouse(position.x, position.y);
            return;
        }

        wrapper = wrapper->next;
    };
}

void drawHouses(HouseWrapper *housesWrapper, Vector2 cameraPosition, Texture2D *house) {
    HouseWrapper *wrapper = housesWrapper;
    wrappersAmount = 0;
    while (wrapper != NULL) {
        if (CheckCollisionRecs(
            (Rectangle) {wrapper->coordinates.x, wrapper->coordinates.y, wrapper->coordinates.width, wrapper->coordinates.width},
            (Rectangle) {cameraPosition.x, cameraPosition.y, wrapper->coordinates.width, wrapper->coordinates.width}
        )) {
            for (int i = 0; i < HOUSES_PER_WRAPPER; i++) {
                if (wrapper->houses[i] != NULL)
                    DrawTextureEx(*house, (Vector2){wrapper->houses[i]->x, wrapper->houses[i]->y}, 0, 0.25, WHITE); 
            }
        }
        wrapper = wrapper->next;
        wrappersAmount++;
    }
}

inline HouseWrapper* isAnyColliding(HouseWrapper *housesWrapper,  Vector2 point, int houseWidth, int houseHeight) {
    return false;
}
