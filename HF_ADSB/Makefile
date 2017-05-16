all:HF_ADSB_Decoder

objects = HF_ADSB_Decoder.o tinyxml/tinystr.o tinyxml/tinyxml.o tinyxml/tinyxmlerror.o tinyxml/tinyxmlparser.o

$(objects): %.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@
	
HF_ADSB_Decoder: $(objects)
	$(CC)   $(CFLAGS) -o $@  $(objects) -L./ -lhfdecoder -lstdc++  -lm
	
clean:
	rm -f $(objects)
	rm -f HF_ADSB_Decoder