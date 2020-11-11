ifndef data_logging_mk
data_logging_mk := Prevent repeated "-include".

data_logging.SRC = $(wildcard $(data_logging)src/*.cpp)
data_logging.OBJ = $(patsubst $(data_logging)src/%.cpp, $(data_logging)build/obj/%.o, $(data_logging.SRC))

$(data_logging)build/liblogging.a: $(data_logging)submodule.mk $(data_logging) $(data_logging.OBJ)
	$(DIR_GUARD)
	@$(AR) rcs $@ $(data_logging.OBJ)
	@if tty -s; then tput setaf 2; echo "Data logging complete"; tput sgr0; fi

$(data_logging)build/obj/%.o: $(data_logging)submodule.mk $(data_logging) $(data_logging)src/%.cpp bal/build/inc/ecbal.h
	$(DIR_GUARD)
	@$(CPPC) $(CPPFLAGS) -DBOARD_$(BOARD) -o $@ $(word 3,$^)

endif
