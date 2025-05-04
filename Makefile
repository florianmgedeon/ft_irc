CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iinc
NAME = ircserv
INC = inc/ircserv.hpp
SRC = srcs/main.cpp
OBJ = $(SRC:srcs/%.cpp=obj/%.o)

all: $(NAME)

$(NAME): $(OBJ) $(INC)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

obj/%.o: srcs/%.cpp $(INC)
	mkdir -p obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf obj

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re