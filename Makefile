NAME		:= webserv

CC			:= c++
CFLAGS		:= -Wall -Wextra -Werror -std=c++98 -g
DEP_FLAGS	:= -MP -MMD

SRCS_DIRS := \
	core \
	cgi \
	utils \
	config \
	config/parser \
	http \
	http/parser

HDRS_DIRS := \
	core \
	cgi \
	utils \
	config \
	config/models \
	config/parser \
	http \
	http/models \
	http/parser

vpath %.cpp	$(SRCS_DIRS)
vpath %.hpp	$(HDRS_DIRS)

SRCS := main.cpp \
	cgi/cgiHandler.cpp \
	config/parser/parserConfig.cpp \
	core/server.cpp \
	core/HttpRequestProcessor.cpp \
	core/pageRenderer.cpp \
	http/parser/parser.cpp \
	utils/logger.cpp \
	utils/file.cpp \
	utils/time.cpp \
	utils/string.cpp \
	utils/exceptions.cpp

OBJS_DIR	:= .objects
OBJS		:= $(addprefix $(OBJS_DIR)/, \
				$(patsubst %.cpp, %.o, $(SRCS)))

DEPS		:= $(addprefix $(OBJS_DIR)/, \
				$(patsubst %.cpp, %.d, $(SRCS)))


all: $(NAME) flower
					@if [ ! -f $(NAME) ]; then $(MAKE) --no-print-directory flower; fi

$(NAME):			$(OBJS)
					@echo "$(FMT_BOLD)🌟 Building the magic...$(FMT_DEF)"
					@$(CC) $(OBJS) $(CFLAGS) -o $@
					@echo "$(FMT_GREEN_B)✨ '$(NAME)'$(FMT_BOLD) is ready to shine!$(FMT_DEF)"

$(OBJS_DIR)/%.o:	%.cpp | $(OBJS_DIR)
					@echo "🔧 Compiling $<..."
					@mkdir -p $(dir $@)
					@$(CC) $(CFLAGS) $(DEP_FLAGS) -c $< -o $@

$(OBJS_DIR):
					@mkdir -p $@
					@echo "$(FMT_BOLD)📂 Directory '$(OBJS_DIR)' has been created.$(FMT_DEF)"

clean:
					@rm -rf $(OBJS) $(DEPS) $(OBJS_DIR)
					@echo "$(FMT_WHITE)🧹 $(NAME): $(FMT_BOLD)Object files have been cleaned.$(FMT_DEF)"

fclean:				clean
					@rm -rf $(NAME)
					@echo "$(FMT_WHITE)🗑️ '$(NAME)'$(FMT_BOLD) has been cleaned.$(FMT_DEF)"

re:					fclean all

flower:
					@echo ""
					@echo "$(FLOWER_COLOR)              🌸      $(FMT_DEF)"
					@echo "$(FLOWER_COLOR)            🌸🌸🌸     $(FMT_DEF)"
					@echo "$(FLOWER_COLOR)          🌸🌸🌸🌸🌸   $(FMT_DEF)"
					@echo "$(FLOWER_COLOR)         🌸🌸🌸🌸🌸🌸  $(FMT_DEF)"
					@echo "$(FLOWER_COLOR)          🌸🌸🌸🌸🌸   $(FMT_DEF)"
					@echo "$(FLOWER_COLOR)            🌸🌸🌸     $(FMT_DEF)"
					@echo "$(FLOWER_COLOR)              🌸      $(FMT_DEF)"
					@echo "$(FMT_GREEN_B)🌼 Webserver is blooming and ready to serve! 🌼$(FMT_DEF)"
					@echo ""

.PHONY:				all clean fclean re flower

-include $(DEPS)

FMT_BOLD	:= \033[0;1m
FMT_WHITE_B	:= \033[1;37m
FMT_WHITE	:= \033[37m
FMT_DEF		:= \033[0;39m
FMT_GREEN_B	:= \033[1;32m
FLOWER_COLOR := \033[1;35m

