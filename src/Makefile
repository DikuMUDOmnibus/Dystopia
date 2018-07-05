CC      := gcc
C_FLAGS := -g -pedantic -Wall -W -Wno-unused-parameter
L_FLAGS := -lz -lcrypt -lpthread -lm
O_DIR   := obj

# In case you want to enable all kinds of warnings, use
# the C_FLAGS below instead.
# C_FLAGS := -g -pedantic -Wall -W -Wno-unused-parameter -Wformat-extra-args -Wformat-security -Wchar-subscripts -Wcomment -Wno-system-headers -Wfloat-equal -Wundef -Wendif-labels -Wshadow -Wpointer-arith -Wcast-align -Wmissing-noreturn -Wmissing-format-attribute -Wredundant-decls -Wdisabled-optimization -Werror

FILES := act_comm act_info act_move act_obj act_wiz       \
         act_fight board datastruct utility shops msp     \
         fight arena mccp db xsocials interp handler olc  \
         mem olc_act olc_save descmap magic mxp kingdoms  \
         bit update const save special string comm shadow \
         giant warlock fae event artifacts webif quests

.SUFFIXES:

Dystopia: $(FILES:%=$(O_DIR)/%.o)
	@rm -f Dystopia
	@echo [`date +%T`] Linking executable ...
	@$(CC) -o Dystopia $(FILES:%=$(O_DIR)/%.o) $(L_FLAGS)
	@chmod 755 Dystopia

-include $(FILES:%=$(O_DIR)/%.d)

%.o $(O_DIR)/%.o : %.c
	@echo [`date +%T`] Compiling $< ...
	@$(CC) -c -MMD -MP $(C_FLAGS) $*.c -o $(O_DIR)/$*.o

clean:
	@echo [`date +%T`] Cleaning code...
	@rm -f $(O_DIR)/*.o
	@rm -f $(O_DIR)/*.d
	@rm -f *.*~

distclean:
	@echo [`date +%T`] Cleaning code...
	@rm -f $(O_DIR)/*.o
	@rm -f $(O_DIR)/*.d
	@rm -f *.*~
	@rm -f Dystopia
