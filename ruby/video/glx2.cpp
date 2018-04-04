//Xorg/GLX OpenGL 2.0 driver

//note: this is a fallback driver for use when OpenGL 3.2 is not available.
//see glx.cpp for comments on how this driver operates (they are very similar.)

#if defined(DISPLAY_XORG)
  #include <GL/gl.h>
  #include <GL/glx.h>
  #ifndef glGetProcAddress
    #define glGetProcAddress(name) (*glXGetProcAddress)((const GLubyte*)(name))
  #endif
#elif defined(DISPLAY_QUARTZ)
  #include <OpenGL/gl3.h>
#elif defined(DISPLAY_WINDOWS)
  #include <GL/gl.h>
  #include <GL/glext.h>
  #ifndef glGetProcAddress
    #define glGetProcAddress(name) wglGetProcAddress(name)
  #endif
#else
  #error "ruby::OpenGL2: unsupported platform"
#endif

struct VideoGLX2 : Video {
  VideoGLX2() { initialize(); }
  ~VideoGLX2() { terminate(); }

  auto ready() -> bool { return _ready; }

  auto context() -> uintptr { return _context; }
  auto blocking() -> bool { return _blocking; }
  auto smooth() -> bool { return _smooth; }

  auto setContext(uintptr context) -> bool {
    if(_context == context) return true;
    _context = context;
    return initialize();
  }

  auto setBlocking(bool blocking) -> bool {
    if(_blocking == blocking) return true;
    _blocking = blocking;
    if(_ready && glXSwapInterval) glXSwapInterval(_blocking);
    return true;
  }

  auto setSmooth(bool smooth) -> bool {
    if(_smooth == smooth) return true;
    _smooth = smooth;
    return true;
  }

  auto clear() -> void {
    if(!ready()) return;
    memory::fill(_glBuffer, _glWidth * _glHeight * sizeof(uint32_t));
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
    if(_isDoubleBuffered) glXSwapBuffers(_display, _glXWindow);
  }

  auto lock(uint32_t*& data, uint& pitch, uint width, uint height) -> bool {
    if(!ready()) return false;
    if(width != _width || height != _height) resize(width, height);
    pitch = _glWidth * sizeof(uint32_t);
    return data = _glBuffer;
  }

  auto unlock() -> void {
    if(!ready()) return;
  }

  auto output() -> void {
    if(!ready()) return;

    XWindowAttributes parent, child;
    XGetWindowAttributes(_display, (Window)_context, &parent);
    XGetWindowAttributes(_display, _window, &child);
    if(child.width != parent.width || child.height != parent.height) {
      XResizeWindow(_display, _window, parent.width, parent.height);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _smooth ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _smooth ? GL_LINEAR : GL_NEAREST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, parent.width, 0, parent.height, -1.0, 1.0);
    glViewport(0, 0, parent.width, parent.height);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPixelStorei(GL_UNPACK_ROW_LENGTH, _glWidth);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _width, _height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, _glBuffer);

    double w = (double)_width / (double)_glWidth;
    double h = (double)_height / (double)_glHeight;
    int u = parent.width;
    int v = parent.height;

    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0); glVertex3i(0, v, 0);
    glTexCoord2f(w, 0); glVertex3i(u, v, 0);
    glTexCoord2f(0, h); glVertex3i(0, 0, 0);
    glTexCoord2f(w, h); glVertex3i(u, 0, 0);
    glEnd();
    glFlush();

    if(_isDoubleBuffered) glXSwapBuffers(_display, _glXWindow);
  }

private:
  auto initialize() -> bool {
    terminate();
    if(!_context) return false;

    _display = XOpenDisplay(0);
    _screen = DefaultScreen(_display);

    int versionMajor = 0, versionMinor = 0;
    glXQueryVersion(_display, &versionMajor, &versionMinor);
    if(versionMajor < 1 || (versionMajor == 1 && versionMinor < 2)) return false;

    XWindowAttributes windowAttributes;
    XGetWindowAttributes(_display, (Window)_context, &windowAttributes);

    int attributeList[] = {
      GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
      GLX_RENDER_TYPE, GLX_RGBA_BIT,
      GLX_DOUBLEBUFFER, True,
      GLX_RED_SIZE, 8,
      GLX_GREEN_SIZE, 8,
      GLX_BLUE_SIZE, 8,
      None
    };

    int fbCount = 0;
    auto fbConfig = glXChooseFBConfig(_display, _screen, attributeList, &fbCount);
    if(fbCount == 0) return false;

    auto vi = glXGetVisualFromFBConfig(_display, fbConfig[0]);
    _colormap = XCreateColormap(_display, RootWindow(_display, vi->screen), vi->visual, AllocNone);
    XSetWindowAttributes attributes;
    attributes.colormap = _colormap;
    attributes.border_pixel = 0;
    _window = XCreateWindow(_display, (Window)_context, 0, 0, windowAttributes.width, windowAttributes.height,
      0, vi->depth, InputOutput, vi->visual, CWColormap | CWBorderPixel, &attributes);
    XSetWindowBackground(_display, _window, 0);
    XMapWindow(_display, _window);
    XFlush(_display);

    while(XPending(_display)) {
      XEvent event;
      XNextEvent(_display, &event);
    }

    _glXContext = glXCreateContext(_display, vi, 0, GL_TRUE);
    glXMakeCurrent(_display, _glXWindow = _window, _glXContext);

    if(!glXSwapInterval) glXSwapInterval = (int (*)(int))glGetProcAddress("glXSwapIntervalMESA");
    if(!glXSwapInterval) glXSwapInterval = (int (*)(int))glGetProcAddress("glXSwapIntervalSGI");

    if(glXSwapInterval) glXSwapInterval(_blocking);

    int value = 0;
    glXGetConfig(_display, vi, GLX_DOUBLEBUFFER, &value);
    _isDoubleBuffered = value;
    _isDirect = glXIsDirect(_display, _glXContext);

    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_STENCIL_TEST);

    glEnable(GL_DITHER);
    glEnable(GL_TEXTURE_2D);

    resize(256, 256);
    return _ready = true;
  }

  auto terminate() -> void {
    _ready = false;

    if(_glTexture) {
      glDeleteTextures(1, &_glTexture);
      _glTexture = 0;
    }

    if(_glBuffer) {
      delete[] _glBuffer;
      _glBuffer = nullptr;
    }

    _glWidth = 0;
    _glHeight = 0;

    if(_glXContext) {
      glXDestroyContext(_display, _glXContext);
      _glXContext = nullptr;
    }

    if(_window) {
      XUnmapWindow(_display, _window);
      _window = 0;
    }

    if(_colormap) {
      XFreeColormap(_display, _colormap);
      _colormap = 0;
    }

    if(_display) {
      XCloseDisplay(_display);
      _display = nullptr;
    }
  }

  auto resize(uint width, uint height) -> void {
    _width = width;
    _height = height;

    if(_glTexture == 0) glGenTextures(1, &_glTexture);
    _glWidth = max(_glWidth, width);
    _glHeight = max(_glHeight, height);
    delete[] _glBuffer;
    _glBuffer = new uint32_t[_glWidth * _glHeight]();

    glBindTexture(GL_TEXTURE_2D, _glTexture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, _glWidth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _glWidth, _glHeight, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, _glBuffer);
  }

  auto (*glXSwapInterval)(int) -> int = nullptr;

  bool _ready = false;
  uintptr _context = 0;
  bool _blocking = false;
  bool _smooth = true;

  Display* _display = nullptr;
  int _screen = 0;
  Window _window = 0;
  Colormap _colormap = 0;
  GLXContext _glXContext = nullptr;
  GLXWindow _glXWindow = 0;

  bool _isDoubleBuffered = false;
  bool _isDirect = false;

  uint _width = 256;
  uint _height = 256;

  GLuint _glTexture = 0;
  uint32_t* _glBuffer = nullptr;
  uint _glWidth = 0;
  uint _glHeight = 0;
};
