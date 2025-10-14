cloud: cloud.cpp bundle.cpp
	g++ -std=c++17 -Dopen_mode=openmode -o $@ $^ -lpthread -lstdc++fs -ljsoncpp

.PHONY: clean
clean:
	rm -f cloud