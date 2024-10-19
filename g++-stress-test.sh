 g++ -std=c++23 \
     -g3 \
     -O3 \
     \
     -Wall \
     -Weffc++ \
     -pedantic  \
	 -pedantic-errors \
     -Wextra \
     -Wno-aggregate-return \
     -Wcast-align \
	 -Wcast-qual \
     -Wconversion \
	 -Wdisabled-optimization \
	 -Wfloat-equal \
     -Wformat=2 \
	 -Wformat-nonliteral \
     -Wformat-security  \
	 -Wformat-y2k \
	 -Wimport \
     -Winit-self \
     -Winline \
	 -Winvalid-pch   \
	 -Wlong-long \
	 -Wmissing-field-initializers \
     -Wmissing-format-attribute   \
	 -Wmissing-include-dirs \
     -Wmissing-noreturn \
	 -Wpacked \
     -Wno-padded \
     -Wpointer-arith \
	 -Wredundant-decls \
	 -Wshadow \
     -Wstack-protector \
	 -Wstrict-aliasing=2 \
     -Wswitch-default \
	 -Wswitch-enum \
	 -Wunreachable-code \
     -Wunused \
	 -Wunused-parameter \
	 -Wvariadic-macros \
	 -Wwrite-strings \
     -Wno-inline \
     -Wno-non-virtual-dtor \
     \
	 -lspdlog \
     -lfmt \
     \
     -ISilvaCollections/ \
     \
     -DDEBUG \
     \
	 example.cpp -o example 2>&1
