NAME = Matt_daemon

SRCS = 										\
		srcs/main.cpp						\
		srcs/daemon/Matt_daemon.cpp			\
		srcs/daemon/Server.cpp				\
		srcs/reporter/Tintin_reporter.cpp	\


OBJS = $(patsubst srcs/%.cpp, objs/srcs/%.o, $(SRCS))
DEPS = $(OBJS:%.o=%.d)

GUICLIENTSRCS = bonus/GUIClient.cpp

GUICLIENTOBJS = $(GUICLIENTSRCS:%.cpp=objs/%.o)
GUICLIENTDEPS = $(GUICLIENTOBJS:%.o=%.d)

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -Wshadow -fsanitize=address -g3
CXXFLAGS +=	-I inc

CPPFLAGS = -MMD
GUICLIENT = Ben_AFK

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(GUICLIENT): $(GUICLIENTOBJS)
	$(CXX) $(CXXFLAGS) $(GUICLIENTOBJS) -o $(GUICLIENT) -lX11

objs/srcs/%.o: ./srcs/%.cpp
	@mkdir -p $(dir $@)
  $(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
  
objs/bonus/%.o: ./bonus/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

bonus:: CXXFLAGS += -D BONUS
bonus:: re
bonus:: GUIClient

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

.PHONY: all clean fclean re bonus $(GUICLIENT)
