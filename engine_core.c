#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

typedef struct { float x, y, z; } Vec3;
typedef struct { int v1, v2, v3; float u, v; } Face;
typedef struct { char name[128]; unsigned int tex_id; Vec3* verts; Face* faces; int vc, fc; } Asset;

Asset registry[1024];
int asset_total = 0;
float cX = 0, cY = 5, cZ = 15;

unsigned int load_tex(const char* path) {
    int w, h, ch;
    unsigned char* data = stbi_load(path, &w, &h, &ch, 4);
    unsigned int id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return id;
}

void load_obj(const char* name) {
    char p[256]; sprintf(p, "assets/%s", name);
    FILE* f = fopen(p, "r");
    if (!f) return;
    Asset* a = &registry[asset_total++];
    strcpy(a->name, name);
    a->verts = malloc(sizeof(Vec3) * 20000);
    a->faces = malloc(sizeof(Face) * 20000);
    a->vc = 0; a->fc = 0;
    char l[256];
    while (fgets(l, sizeof(l), f)) {
        if (l[0] == 'v' && l[1] == ' ') {
            sscanf(l, "v %f %f %f", &a->verts[a->vc].x, &a->verts[a->vc].y, &a->verts[a->vc].z);
            a->vc++;
        } else if (l[0] == 'f') {
            sscanf(l, "f %d %d %d", &a->faces[a->fc].v1, &a->faces[a->fc].v2, &a->faces[a->fc].v3);
            a->fc++;
        }
    }
    fclose(f);
}

void draw(const char* name, float x, float y, float z, const char* tex) {
    for (int i = 0; i < asset_total; i++) {
        if (strcmp(registry[i].name, name) == 0) {
            glPushMatrix();
            glTranslatef(x, y, z);
            if (tex) {
                glEnable(GL_TEXTURE_2D);
                // Bind specific texture here
            }
            glBegin(GL_TRIANGLES);
            for (int j = 0; j < registry[i].fc; j++) {
                Face f = registry[i].faces[j];
                glVertex3f(registry[i].verts[f.v1-1].x, registry[i].verts[f.v1-1].y, registry[i].verts[f.v1-1].z);
                glVertex3f(registry[i].verts[f.v2-1].x, registry[i].verts[f.v2-1].y, registry[i].verts[f.v2-1].z);
                glVertex3f(registry[i].verts[f.v3-1].x, registry[i].verts[f.v3-1].y, registry[i].verts[f.v3-1].z);
            }
            glEnd();
            glPopMatrix();
        }
    }
}

void run_script(const char* p) {
    FILE* f = fopen(p, "r");
    if (!f) return;
    char l[512];
    while (fgets(l, sizeof(l), f)) {
        char c[32], a[128], t[128]; float x, y, z;
        if (sscanf(l, "spawn %s %f %f %f %s", a, &x, &y, &z, t) >= 4) draw(a, x, y, z, t);
        else if (sscanf(l, "load %s", a) == 1) load_obj(a);
        else if (sscanf(l, "import %s", a) == 1) { char np[256]; sprintf(np, "src/%s", a); run_script(np); }
    }
    fclose(f);
}

int main() {
    glfwInit();
    GLFWwindow* w = glfwCreateWindow(1280, 720, "LibreSandbox", NULL, NULL);
    glfwMakeContextCurrent(w);
    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(w)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) cZ -= 0.1f;
        if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) cZ += 0.1f;
        gluLookAt(cX, cY, cZ, 0, 0, 0, 0, 1, 0);
        run_script("src/master.lbc");
        glfwSwapBuffers(w);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}