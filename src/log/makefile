CXX = g++
CXXFLAGS = -Wall -std=c++11

TARGET = libpnlog.a

INCLUDE_PATH = -I ../../include/

SRCS := $(wildcard *.cpp) $(wildcatd ../util/*.cppp)
OBJS = $(SRCS:.cpp=.o)
DEPS := $(SRCS:.cpp=.d)

#特殊的符号$@代表目标文件，$^代表所有依赖文件
$(TARGET) : $(OBJS)
	ar crv $@ $^

%.o : %.cpp
	$(CXX) -o $@ -c $< -MMD -MP $(INCLUDE_PATH) $(CXXFLAGS)
# -MP的作用是为头文件生成单独的依赖规则，没有依赖项，也没有动作。为了使得头文件改名或者被删除后，include其的
# 源文件忘了改，make的报错信息更清晰。

-include $(DEPS)

# 通配符可以出现在模式的命令中
.PHONY : clean
clean : 
	@rm -rf $(OBJS) $(DEPS) $(TARGET); 
