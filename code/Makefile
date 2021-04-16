CC	= gcc
AS	= as
CFLAGS	= -g -O2
ASFLAGS = 
LDFLAGS = -lpthread
RM	= rm

CSRCS	= server.c
ASRCS   = 
TARGET	=  simple_webserver
OBJECTS = $(CSRCS:.c=.o) $(ASRCS:.s=.o)
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)
	
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.s.o:
	$(AS) $(ASFLAGS) $< -o $@

clean:
	$(RM) -f $(OBJECTS) $(TARGET) 

