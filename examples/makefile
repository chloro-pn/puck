CXX = g++
CXXFLAGS = -Wall -std=c++11

INCLUDE_PATH = -I ../include

SRCS := $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.out)
DEPS := $(SRCS:.cpp=.d)
LIBS := ../src/libpuck.a ../src/log/libpnlog.a
STDLIBS := -lpthread

#特殊的符号$@代表目标文件，$^代表所有依赖文件
all : $(OBJS) $(LIBS)

$(OBJS):%.out : %.cpp
	$(CXX) -o $@ $< -MMD -MP $(INCLUDE_PATH) $(LIBS) $(CXXFLAGS) $(STDLIBS)
# -MP的作用是为头文件生成单独的依赖规则，没有依赖项，也没有动作。为了使得头文件改名或者被删除后，include其的
# 源文件忘了改，make的报错信息更清晰。

../src/libpuck.a : 
	@echo "make libpuck.a."; \
	$(MAKE) -C ../src

../src/log/libpnlog.a :
	@echo "make libpnlog.a"; \
	$(MAKE) -C ../src/log

-include $(DEPS)

# 通配符可以出现在模式的命令中
.PHONY : clean
clean : 
	@rm -rf $(OBJS) $(DEPS) $(TARGET);
