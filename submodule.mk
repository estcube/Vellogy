ifndef app_logging_mk
app_logging_mk := Prevent repeated "-include".

app_logging.SRC = $(wildcard $(app_logging)src/*.cpp)
app_logging.OBJ = $(patsubst $(app_logging)src/%.cpp, $(app_logging)build/obj/%.o, $(app_logging.SRC))

$(app_logging)build/liblogging.a: $(app_logging)submodule.mk $(app_logging) $(app_logging.OBJ)
	$(DIR_GUARD)
	@$(AR) rcs $@ $(app_logging.OBJ)
	@if tty -s; then tput setaf 2; echo "Data logging complete"; tput sgr0; fi

$(app_logging)build/obj/%.o: $(app_logging)submodule.mk $(app_logging) $(app_logging)src/%.cpp bal/build/inc/ecbal.h
	$(DIR_GUARD)
	@$(CPPC) $(CPPFLAGS) -DBOARD_$(BOARD) -o $@ $(word 3,$^)

endif
