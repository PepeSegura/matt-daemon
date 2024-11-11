NAME = Matt_daemon

SRCS = 										\
		srcs/main.cpp						\
		srcs/daemon/Matt_daemon.cpp			\
		srcs/daemon/Server.cpp				\
		srcs/reporter/Tintin_reporter.cpp	\


OBJS = $(patsubst srcs/%.cpp, objs/srcs/%.o, $(SRCS))
DEPS = $(OBJS:%.o=%.d)

GUICLIENTSRCS = bonus/BenAFK.cpp

GUICLIENTOBJS = $(GUICLIENTSRCS:%.cpp=objs/%.o)
GUICLIENTDEPS = $(GUICLIENTOBJS:%.o=%.d)

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -Wshadow -g3 #-fsanitize=address -g3
CXXFLAGS +=	-I inc

CPPFLAGS = -MMD
GUICLIENT = Ben_AFK

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(GUICLIENT): $(GUICLIENTOBJS)
	g++ $(CXXFLAGS) $(GUICLIENTOBJS) -o $(GUICLIENT) `pkg-config gtkmm-3.0 --cflags --libs`

objs/srcs/%.o: ./srcs/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
  
objs/bonus/%.o: ./bonus/%.cpp
	@mkdir -p $(dir $@)
	g++ $(CPPFLAGS) $(CXXFLAGS) `pkg-config gtkmm-3.0 --cflags --libs` -c $< -o $@

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
