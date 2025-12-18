NAME = ircserv
CFLAGS = -Wall -Wextra -Werror -std=c++98 -MMD -MP
CXX = c++
SRCS = Channel.cpp Client.cpp errors.cpp main.cpp Server.cpp mode.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

clean:
	rm -f $(OBJS) $(OBJS:.o=.d)

fclean: clean
	rm -f $(NAME)

re: fclean all