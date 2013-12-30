CC = g++
LFLAGS        = -g -Wall -Wextra
INCPATH       = -I/usr/share/qt4/mkspecs/linux-g++ -I. -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtGui -I/usr/include/qt4 -I. -I.
LINK          = g++
LIBS          = -L/usr/lib/i386-linux-gnu  -L/usr/local/lib  -lpcap -lQtGui -lQtCore -lpthread 

OBJ           =  packet.o inodeport.o main.o process.o maindialog.o
TERGET        =  all

all: $(OBJ)
	$(LINK) -o all $(OBJ) $(LIBS)  $(LFLAGS)

packet.o: packet_cap.cpp  packet_cap.h
	$(CC) -c packet_cap.cpp -o packet.o $(LFLAGS)

inodeport.o: inodeport.cpp inodeport.h
	$(CC) -c inodeport.cpp -o inodeport.o $(LFLAGS)

main.o:main.cpp 
	$(CC) -c main.cpp $(LFLAGS) $(INCPATH)

process.o: process.cpp process.h
	$(CC) -c process.cpp $(LFLAGS)

maindialog.o: maindialog.cpp maindialog.h
	$(CC) -c maindialog.cpp $(LFLAGS) $(INCPATH)

clean:
	rm -f $(OBJ) $(TERGET)
	rm -f *~
	
	





