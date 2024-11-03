NAME = Matt_daemon

SRCS = 										\
		srcs/main.cpp						\
		srcs/daemon/Matt_daemon.cpp			\
		srcs/daemon/Server.cpp				\
		srcs/reporter/Tintin_reporter.cpp	\


OBJS = $(patsubst srcs/%.cpp, objs/srcs/%.o, $(SRCS))
DEPS = $(OBJS:%.o=%.d)

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -Wshadow -fsanitize=address -g3
CXXFLAGS +=	-I inc

CPPFLAGS = -MMD

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

objs/srcs/%.o: ./srcs/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

bonus:: CXXFLAGS += -D BONUS
bonus:: re

all: $(NAME)

clean:
	@rm -rf objs

fclean: clean
	@rm -f $(NAME)

re:: fclean
re:: all

-include $(DEPS)

.PHONY: all clean fclean re bonus
