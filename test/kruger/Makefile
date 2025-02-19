TARGET=main
include ../shared/Makefile.shared

$(TARGET): $(TARGET).o
	$(CXX) $(TARGET).o -lm $(SNIPER_LDFLAGS) -o $(TARGET)

run_$(TARGET):
	../../run-sniper -v -n 8 -c donuts -s stop-by-icount:50000000 -- ./main -p 1

CLEAN_EXTRA=viz
