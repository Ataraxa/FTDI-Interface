CXX = g++
CXXFLAGS = -std=c++17 -Iexternal/imgui -Iexternal/imgui/backends -Iexternal/implot -Iexternal/glfw/include -mwindows
LDFLAGS = -Lexternal/glfw/build/src -lglfw3 -lopengl32 -lgdi32

SRCS = main.cpp \
       external/imgui/imgui.cpp \
       external/imgui/imgui_draw.cpp \
       external/imgui/imgui_tables.cpp \
       external/imgui/imgui_widgets.cpp \
       external/imgui/imgui_demo.cpp \
       external/imgui/backends/imgui_impl_glfw.cpp \
       external/imgui/backends/imgui_impl_opengl3.cpp \
       external/implot/implot.cpp \
       external/implot/implot_items.cpp

OBJS = $(SRCS:.cpp=.o)
EXEC = minimal_gui.exe

all: $(EXEC)

$(EXEC): $(SRCS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -f *.o $(EXEC)
       