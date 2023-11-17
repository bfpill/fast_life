#define GL_SILENCE_DEPRECATION
#include <iostream>
#include <thread>
#include <set>

#include "gl_utils.hpp"

float zoomScale = 1.0f;
float centerX = 0.0f;
float centerY = 0.0f;

Board global_board = {};

int num_threads = std::thread::hardware_concurrency();
std::vector<std::thread> threads(num_threads);
std::vector<Board> slices(num_threads);

void update_global_board() {
    threaded_get_next_board(global_board, threads, num_threads, slices);
    glutPostRedisplay();
}

void set_pixel(int x, int y, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

void display_window() {
    glClear(GL_COLOR_BUFFER_BIT);

    update_global_board();

    for (const auto& cell : global_board) {
        int factor = (10 * (1-zoomScale) + 1);
        int x = cell.first * factor,
            y = cell.second * factor;

        for(int i = 0; i < factor + 1; i++){
            for(int j = 0; j < factor + 1; j++){
                set_pixel(x - i, y - j, 1.0, 0.0, 0.0); // Red
            }
        }
    }

    glutSwapBuffers();
}

void updateProjection(int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(centerX - (width / 2.0) * zoomScale,
               centerX + (width / 2.0) * zoomScale,
               centerY - (height / 2.0) * zoomScale,
               centerY + (height / 2.0) * zoomScale);
    glMatrixMode(GL_MODELVIEW);
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    updateProjection(width, height);
}

void zoom(int direction) {
    const float zoomSpeed = 0.1f;
    if (direction > 0) {
        zoomScale /= (1 + zoomSpeed);
    } else {
        zoomScale *= (1 + zoomSpeed);
    }
    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);
    updateProjection(width, height);
    glutPostRedisplay();
}

void pan(int xDirection, int yDirection) {
    float panSpeed = 20.0f * zoomScale;

    centerX += xDirection * panSpeed;
    centerY += yDirection * panSpeed;

    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);
    updateProjection(width, height);

    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:
            pan(-1, 0);
            break;
        case GLUT_KEY_RIGHT:
            pan(1, 0);
            break;
        case GLUT_KEY_UP:
            pan(0, 1);
            break;
        case GLUT_KEY_DOWN:
            pan(0, -1);
            break;
    }
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case '+':
            zoom(1);
            break;
        case '-':
            zoom(-1);
            break;
    }
}


// Glut works through callbacks and runs its own loop
// So.... yeah... we give it things
void init_window(int argc, char** argv, Board start_board) {

    global_board = start_board;

    glutInit(&argc, argv);

    unsigned int window_width = 400;
    unsigned int window_height = 400;

    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("OpenGL Window");

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(0.0, window_width, window_height, 0.0);

    // These two handle zoom and that good good
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    glutDisplayFunc(display_window);
    glutMainLoop();
}
