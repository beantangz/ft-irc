CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -MMD -MP

SRC = Channel.cpp Client.cpp errors.cpp main.cpp Server.cpp mode.cpp

OBJ_DIR = obj
OBJ = $(SRC:%.cpp=$(OBJ_DIR)/%.o)
DEP = $(OBJ:.o=.d)

NAME = irc_serv

all: $(NAME)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEP)



