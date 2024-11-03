NAME = Matt_daemon

SRCS = 										\
		srcs/main.cpp						\
		srcs/daemon/Matt_daemon.cpp			\
		srcs/daemon/Server.cpp				\
		srcs/reporter/Tintin_reporter.cpp	\


OBJS = $(SRCS:%.cpp=objs/%.o)
DEPS = $(OBJS:%.o=%.d)

GUICLIENTSRCS = bonus/GUIClient.cpp

GUICLIENTOBJS = $(GUICLIENTSRCS:%.cpp=objs/%.o)
GUICLIENTDEPS = $(GUICLIENTOBJS:%.o=%.d)

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -Wshadow -fsanitize=address -g3
CXXFLAGS +=	-I inc
CPPFLAGS = -MMD
GUICLIENT = Ben_AFK

$(NAME): objs $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(GUICLIENT): objs $(GUICLIENTOBJS)
	$(CXX) $(CXXFLAGS) $(GUICLIENTOBJS) -o $(GUICLIENT) -lX11

objs:
	@mkdir -p objs/srcs/daemon objs/srcs/reporter objs/bonus

objs/%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

debug:: CXXFLAGS += -D DEBUG -g3 -fsanitize=address
debug:: re

all: $(NAME)

GUIClient: $(GUICLIENT)

clean:
	@rm -rf objs

fclean: clean
	@rm -f $(NAME)
	@rm -f $(GUICLIENT)

re:: fclean
re:: all

-include $(DEPS)

-include $(GUICLIENTDEPS)

.PHONY: all clean fclean re GUIClient