CC = g++

TARGET = sfgen

CPPFLAGS = -std=c++11
LDFLAGS = -lpng

CPPFILES = sfgen.cpp

$(TARGET) : $(CPPFILES)
	$(CC) -o $(TARGET) $(CPPFLAGS) $(CPPFILES) $(LDFLAGS)

clean:
	rm $(TARGET)
	rm *.o
