#include <stdio.h>
#include <GLFW/glfw3.h>
#include "helpers.hpp"

bool load_frame(const char* filename, int* width, int* height, unsigned char** data);

int main(int argc, char** argv) {
  GLFWwindow* window;

  if(!glfwInit()) exitWithError(1, "Could not initialize GLFW");

  window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
  if(!window) exitWithError(1, "Could not open window");

  unsigned char* data = new unsigned char[100 * 100 *3];
  int frame_height, frame_width;
  
  if(!load_frame("/Users/marcelo/Desktop/video_sample.mp4", &frame_width, &frame_height, &data))
    exitWithError(1, "Could not load frame.\n");

  glfwMakeContextCurrent(window);

  GLuint tex_handle;
  glGenTextures(1, &tex_handle);
  glBindTexture(GL_TEXTURE_2D, tex_handle);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_REPEAT);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

  while(!glfwWindowShouldClose(window)){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set orthographic projection
    int window_height, window_width;
    glfwGetFramebufferSize(window, &window_width, &window_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, window_width, 0, window_height, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    // Render whatever buffer on screen
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex_handle);
    glBegin(GL_QUADS);
      glTexCoord2d(0, 0); glVertex2i(200, 200);
      glTexCoord2d(1, 0); glVertex2i(200 + frame_width, 200);
      glTexCoord2d(1, 1); glVertex2i(200 + frame_width, 200 + frame_height);
      glTexCoord2d(0, 0); glVertex2i(200, 200 + frame_height);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glfwSwapBuffers(window);
    glfwWaitEvents();
  }  

  return 0;
}