#include <stdio.h>
#include <GLFW/glfw3.h>
#include "helpers.hpp"

bool load_frame(const char* filename, int* width, int* height, unsigned char** data);

int main(int argc, char** argv) {

  char* filename = argv[1];
  GLFWwindow* window;

  if(!glfwInit()) exitWithError(1, "Could not initialize GLFW");

  window = glfwCreateWindow(1280, 720, "Hello World", NULL, NULL);
  if(!window) exitWithError(1, "Could not open window");

  unsigned char* frame_data;
  int frame_height, frame_width;
  
  if(!load_frame(filename, &frame_width, &frame_height, &frame_data))
    exitWithError(1, "Could not load frame.\n");

  glfwMakeContextCurrent(window);

  printf("Frame height: %i, frame width: %i\n", frame_height, frame_width);
  GLuint tex_handle;
  glGenTextures(1, &tex_handle);
  glBindTexture(GL_TEXTURE_2D, tex_handle);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, frame_data);

  while(!glfwWindowShouldClose(window)){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set orthographic projection
    int window_height, window_width;
    glfwGetFramebufferSize(window, &window_width, &window_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, window_width, 0, frame_height, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    // Render whatever buffer on screen
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex_handle);
    glBegin(GL_QUADS);
      glTexCoord2d(0, 0); glVertex2i(0, frame_height);
      glTexCoord2d(1, 0); glVertex2i(frame_width, frame_height);
      glTexCoord2d(1, 1); glVertex2i(frame_width, 0);
      glTexCoord2d(0, 1); glVertex2i(0, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glfwSwapBuffers(window);
    glfwWaitEvents();
  }  

  return 0;
}