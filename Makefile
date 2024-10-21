NAME = Matt_daemon

SRCS = 										\
		srcs/main.cpp						\
		srcs/daemon/Matt_daemon.cpp			\
		srcs/daemon/Server.cpp				\
		srcs/reporter/Tintin_reporter.cpp	\


OBJS = $(SRCS:%.cpp=objs/%.o)
DEPS = $(OBJS:%.o=%.d)

CXX = c++

CXXFLAGS =	-Wall -Wextra -Werror -Wshadow -fsanitize=address -g3
CXXFLAGS +=	-I inc
CPPFLAGS = -MMD

$(NAME): objs $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

objs:
	@mkdir -p objs/srcs/daemon objs/srcs/reporter

objs/%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

debug:: CXXFLAGS += -D DEBUG -g3 -fsanitize=address
debug:: re

all: $(NAME)

clean:
	@rm -rf objs

fclean: clean
	@rm -f $(NAME)

re:: fclean
re:: all

-include $(DEPS)

.PHONY: all clean fclean re