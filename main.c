#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>

// TO HANDLE FORWARD DECLARATION
enum BuildingType {
    House,
    Lumbermill
};

enum Quadrant {
    NW,
    NE,
    SW,
    SE
};

typedef struct Building {
    int x;
    int y;
    union {
        int inhabitants;
        int workers;
    } buildingData;
    enum BuildingType type;
    int repairState;
} Building;
// FORWARD DECLARATION

#define MAX_QUAD_NODE_CAPACITY 4
typedef struct QuadNode {
    Rectangle *coords;
    struct QuadNode *parent;
    struct QuadNode *ne;
    struct QuadNode *nw;
    struct QuadNode *sw;
    struct QuadNode *se;
    // Add type?
    void** data;
} QuadNode;

QuadNode* initChildNode(QuadNode *parent, float x, float y) {
    QuadNode *node = malloc(sizeof(QuadNode));
    node->coords = malloc(sizeof(Rectangle));
    node->parent = parent;
    node->ne = node->nw = node->se = node->sw = NULL;
    node->data = NULL;
    node->coords->height = parent->coords->height / 2.f;
    node->coords->width = parent->coords->width / 2.f;
    node->coords->x = x;
    node->coords->y = y;
    node->data = malloc(sizeof(void*) * MAX_QUAD_NODE_CAPACITY);
    for (int i = 0; i < MAX_QUAD_NODE_CAPACITY; i++) {
        node->data[i] = NULL;
    }
    return node;
}

void makeNewParent(QuadNode *node, Vector2 target) {
    QuadNode *parent = malloc(sizeof(QuadNode));
    
    bool isLeft = node->coords->x > target.x;
    bool isAbove = node->coords->y > target.y;

    float x = isLeft ? node->coords->x - node->coords->width : node->coords->x + node->coords->width;
    float y = isAbove ? node->coords->y - node->coords->height : node->coords->y + node->coords->height;

    parent->coords = malloc(sizeof(Rectangle));
    parent->coords->width = node->coords->width * 2;
    parent->coords->height = node->coords->height * 2;
    parent->coords->x = x;
    parent->coords->y = y;
    parent->data = malloc(sizeof(void*) * MAX_QUAD_NODE_CAPACITY);

    /* WRONG  */
    bool nw = !isLeft && !isAbove;
    bool ne = !isLeft && isAbove;
    bool sw = isLeft && isAbove;
    bool se = !isLeft && !isAbove;

    if (nw) printf("NW\n");
    if (ne) printf("NE\n");
    if (sw) printf("SW\n");
    if (se) printf("SE\n");

    if (nw) parent->nw = node;
    else parent->nw = initChildNode(parent, parent->coords->x, parent->coords->y);

    if (ne) parent->ne = node;
    else parent->ne = initChildNode(parent, parent->coords->x + parent->coords->width / 2, parent->coords->y);
        
    if (sw) parent->sw = node;
    else parent->sw = initChildNode(parent, parent->coords->x, parent->coords->y + parent->coords->height / 2);
        
    if (se) parent->se = node;
    else parent->se = initChildNode(parent, parent->coords->x + parent->coords->width / 2, parent->coords->y + parent->coords->height / 2);
    
    // if (!CheckCollisionPointRec(target, *parent->coords)) {
    //     printf("Attempt make new parent make new parent\n");
    //     makeNewParent(parent, target);
    // }
}

void makeSubdivision(QuadNode *node) {
    node->nw = initChildNode(node, node->coords->x, node->coords->y);
    node->ne = initChildNode(node, node->coords->x + node->coords->width / 2, node->coords->y);
    node->sw = initChildNode(node, node->coords->x, node->coords->y + node->coords->height / 2);
    node->se = initChildNode(node, node->coords->x + node->coords->width / 2, node->coords->y + node->coords->height / 2);
    int nw = 0, ne = 0, sw = 0, se = 0;

    int count = 0;
    for (int i = 0; i < MAX_QUAD_NODE_CAPACITY; i++) {
        Building *b = node->data[i];
        if (b == NULL) return;
        count++;
        // if buildings y position is greater or equal to south nodes
        if (b->y >= node->sw->coords->y) {
            // if bulding is on the right side
            if (b->x >= node->se->coords->x) {
                node->se->data[se++] = b;
            } else {
                node->sw->data[sw++] = b;
            }
        } else {
            // Node is in upper two quadrants
            if (b->x >= node->ne->coords->x) {
                node->ne->data[ne++] = b;
            } else {
                node->nw->data[nw++] = b;
            }
        }
    }
    printf("Placed %i \n", count);
    free(node->data);
}

bool isLeaf(QuadNode *node) {
    // Any child node being null means every node is. For now.
    return node->ne == NULL;
}

void drawQuadTree(QuadNode *node, char dir[4]) {
    if (isLeaf(node)) {
        int count = 0;
        for (int i = 0; i < MAX_QUAD_NODE_CAPACITY; i++) {
            if (node->data[i] != NULL) {
                count++;
            } else {
                break;
            }
        }
        DrawRectangleLines(node->coords->x, node->coords->y, node->coords->width, node->coords->height, RED);
        DrawText(dir, node->coords->x + 5, node->coords->y + 4, 10, RED);
        char buff[10];
        sprintf(buff, "%i", count);
        DrawText(buff, node->coords->x + 40, node->coords->y + 4, 10, RED);
    } else {
        drawQuadTree(node->nw, "NW");
        drawQuadTree(node->ne, "NE");
        drawQuadTree(node->sw, "SW");
        drawQuadTree(node->se, "SE");
    }
}

#define MODE_2D(BLOCK) BeginMode2D(*camera);\
    BLOCK \
EndMode2D();

#define DRAW(BLOCK) BeginDrawing();\
    BLOCK \
EndDrawing();

typedef struct Textures {
    Texture2D house;
    Texture2D lumbermill;
} Textures;

typedef struct Resources {
    int gold; 
    int wood;
    int electricity;
} Resources;

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
void drawBuildings(QuadNode *root, Vector2 cameraPosition, Textures *textures);
bool addBuilding(QuadNode *root, Vector2 position, enum BuildingType type);
void input(Camera2D *camera, float delta, Message *message, QuadNode *root, enum BuildingType *type);
void update(Message *message, float delta);
void render(Camera2D *camera, Textures *textures, QuadNode *root, Message *message, enum BuildingType type);
void freeHouseWrappers(BuildingWrapper *housesWrapper);
enum Quadrant findQuadrant(QuadNode *node, Vector2 pos);
bool isAnyColliding(QuadNode *node, Rectangle box2);

int wrappersAmount = 0;
int screenWidth = 800;
int screenHeight = 450;
int debug = 0;
int houseHeight = 0;
int houseWidth = 0;
int sawmillHeight = 0;
int sawmillWidth = 0;

int main() {
    enum BuildingType buildingType = House;

    QuadNode tree = {
        malloc(sizeof(Rectangle)),
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        malloc(sizeof(Building) * MAX_QUAD_NODE_CAPACITY)
    };

    tree.coords->x = 0;
    tree.coords->y = 0;
    tree.coords->width = screenWidth;
    tree.coords->height = screenHeight;
    for (int i = 0; i < MAX_QUAD_NODE_CAPACITY; i++) {
        tree.data[i] = NULL;
    }

    InitWindow(screenWidth, screenHeight, "YARP");

    Message message;
    message.message = malloc(sizeof(char) * 256);
    message.timeRemaining = 0;

    Camera2D camera;
    camera.offset.x = camera.offset.y = 0;
    camera.rotation = 0;
    camera.target = (Vector2){0, 0};
    camera.zoom = 1;

    Textures textures;
    textures.house = LoadTexture("Assets/house.png");
    houseHeight = textures.house.height / 4;
    houseWidth = textures.house.width / 4;
    textures.lumbermill = LoadTexture("Assets/lumbermill_outline.png");
    sawmillHeight = textures.lumbermill.height / 4;
    sawmillWidth = textures.lumbermill.width / 4;

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    while (!WindowShouldClose()) {
        float delta = GetFrameTime();
        input(&camera, delta, &message, &tree, &buildingType);
        update(&message, delta);
        render(&camera, &textures, &tree, &message, buildingType);
    }
    CloseWindow();
    return 0;
}

void update(Message *message, float delta) {
    if (message->timeRemaining > 0) message->timeRemaining -= delta;
}

void render(Camera2D *camera, Textures *textures, QuadNode *root, Message *message, enum BuildingType type) {
    DRAW(
        ClearBackground((Color){70, 149, 75});

        MODE_2D(
            drawBuildings(root, (Vector2){0, 0}, textures);
            if (debug) {
                drawQuadTree(root, "ROOT");
            }
        );
        Vector2 mouse = GetMousePosition(); 
        Vector2 pointerPosition = GetScreenToWorld2D(mouse, *camera);
        DrawCircle(mouse.x, mouse.y, 10, RED);


        DrawRectangle(screenWidth * 0.75f, 0, screenWidth / 4 , 30, WHITE);
        if (message->timeRemaining > 0) {
            DrawText(message->message, mouse.x, mouse.y, 16, RED);
        }
        if (debug) {
            char buff[256];
            DrawFPS(10, 10);
            sprintf(buff, "Wrappers: %i", wrappersAmount);
            DrawText(buff, 2, 2, 20, WHITE);
            sprintf(buff, "%f, %f", pointerPosition.x, pointerPosition.y);
            DrawText(buff, 10, 25, 20, WHITE);
            sprintf(buff, "Type: %i", type);
            DrawText(buff, 10, 40, 20, WHITE);
            
            enum Quadrant quadrant = findQuadrant(root, mouse);
            switch (quadrant) {
                case NW:
                    DrawText("NW", mouse.x, mouse.y, 20, WHITE);
                    break;
                case NE:
                    DrawText("NE", mouse.x, mouse.y, 20, WHITE);
                    break;
                case SW:
                    DrawText("SW", mouse.x, mouse.y, 20, WHITE);
                    break;
                case SE:
                    DrawText("SE", mouse.x, mouse.y, 20, WHITE);
                    break;
            }
        }
    );
}

void input(Camera2D *camera, float delta, Message *message, QuadNode *root, enum BuildingType *type) {
    SetMouseOffset(camera->offset.x, camera->offset.y);
    Vector2 mouse = GetMousePosition();

    Vector2 pointerPosition = GetScreenToWorld2D(mouse, *camera);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        bool added = addBuilding(root, pointerPosition, *type);
        printf(added ? "true\n" : "false\n");
        if (!added) {
            message->message = "Cannot place building. Collides with other house.";
            message->timeRemaining = 5;
        }
    }


    int scrollSpeed = IsKeyDown(KEY_LEFT_SHIFT) ? 250 : 150;
    float invertedZoomScale = 1 / camera->zoom;


    if (IsKeyDown(KEY_W)) {
        camera->target.y -= delta * scrollSpeed  * invertedZoomScale;
    } else if (IsKeyDown(KEY_S)) {
        camera->target.y += delta * scrollSpeed  * invertedZoomScale;
    }

    if (IsKeyDown(KEY_A)) {
        camera->target.x -= delta * scrollSpeed * invertedZoomScale;
    } else if (IsKeyDown(KEY_D)) {
        camera->target.x += delta * scrollSpeed * invertedZoomScale;
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
    if (camera->zoom > 3.f) camera->zoom = 3;
    else if (camera->zoom < 0.1f) camera->zoom = 0.1f;
}


inline Building* initBuilding(int x, int y, enum BuildingType type) {
    Building *building = malloc(sizeof(Building));
    building->x = x;
    building->y = y;
    building->type = type;
    building->repairState = 100;
    switch (type) {
        case House:
            building->buildingData.inhabitants = 0;
            break;
        case Lumbermill:
            building->buildingData.workers = 0;
        default:
            break;
    }
    return building;
}

enum Quadrant findQuadrant(QuadNode *node, Vector2 pos) {
    double middleX = node->coords->x + node->coords->width / 2;
    double middleY = node->coords->y + node->coords->height / 2; 

    bool isNorth = middleY > pos.y;
    bool isWest = middleX > pos.x;

    if (isNorth) {
        return isWest ? NW : NE;
    } else {
        return isWest ? SW : SE;
    }
}

bool addBuilding(QuadNode *node, Vector2 position, enum BuildingType type) {

    // if (!CheckCollisionPointRec(position, *node->coords)) {
    //     makeNewParent(node, position);
    // }

    if (isLeaf(node)) {
        Rectangle b = {position.x, position.y, type == House  ? houseWidth : sawmillWidth, type == House ? houseHeight : sawmillHeight};
        if (isAnyColliding(node, b)) return false;


        for (int i = 0; i < MAX_QUAD_NODE_CAPACITY; i++) {
            if (node->data[i] == NULL) {
                printf("adding\n");
                node->data[i] = initBuilding(position.x, position.y, type);
                return true;
            }
        }
        makeSubdivision(node);
    } 
    
    enum Quadrant quadrant = findQuadrant(node, position);
    switch (quadrant) {
        case NW:
            printf("NW\n");
            return addBuilding(node->nw, position, type);
        case NE:
            printf("NE\n");
            return addBuilding(node->ne, position, type);
        case SW:
            printf("SW\n");
            return addBuilding(node->sw, position, type);
        case SE:
            printf("SE\n");
            return addBuilding(node->se, position, type);
    }
}

void drawBuildings(QuadNode *node, Vector2 cameraPosition, Textures *textures) {
    if (isLeaf(node)) {
        for (int i = 0; i < MAX_QUAD_NODE_CAPACITY; i++) {
            if (node->data[i] != NULL) {
                Building *b = node->data[i];
                switch (b->type) {
                    case House:
                        DrawTextureEx(textures->house, (Vector2){b->x, b->y}, 0, 0.25, WHITE);  
                        break;
                    case Lumbermill:
                        DrawTextureEx(textures->lumbermill, (Vector2){b->x, b->y}, 0, 0.25, WHITE);  
                        break;
                    default:
                        DrawRectangle(b->x, b->y, 24, 24, WHITE);
                        DrawText("???", b->x, b->y, 20, BLACK);
                        break;
                    }
            } else {
                return;
            }
        }
    } else {
        drawBuildings(node->nw, cameraPosition, textures);
        drawBuildings(node->ne, cameraPosition, textures);
        drawBuildings(node->sw, cameraPosition, textures);
        drawBuildings(node->se, cameraPosition, textures);
    }
}

bool isAnyColliding(QuadNode *node, Rectangle rectangle) {
    if (!CheckCollisionRecs(*node->coords, rectangle)) return false;
    if (isLeaf(node)) {
        for (int i = 0; i < MAX_QUAD_NODE_CAPACITY; i++) {
            Building *b = node->data[i];
            if (b == NULL) return false;
            enum BuildingType t = b->type;
            Rectangle rec = { b->x, b->y, t == House  ? houseWidth : sawmillWidth, t == House ? houseHeight : sawmillHeight};
            if (CheckCollisionRecs(rec, rectangle)) return true;
        }
        return false;
    } else {
        return (
            isAnyColliding(node->nw, rectangle) ||
            isAnyColliding(node->ne, rectangle) ||
            isAnyColliding(node->sw, rectangle) ||
            isAnyColliding(node->se, rectangle)
        );
    }
}