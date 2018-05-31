CC = g++

TARGET = sfgen

CPPFLAGS = -std=c++11
LDFLAGS = -lpng

$(TARGET) : 
	$(CC) $(CPPFLAGS)