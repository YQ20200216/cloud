cloud: cloud.cpp bundle.cpp
	g++ -std=c++17 -Dopen_mode=openmode -o $@ $^ -lpthread -lstdc++fs -ljsoncpp
	# -Dopen_mode=openmode,是将open_mode定义为openmode
	# -lpthread,是线程库,配合cloud.cpp使用
	# -lstdc++fs,是文件系统库,配合util.hpp使用	
	# -ljsoncpp,是json库
.PHONY: clean
clean:
	rm -f cloud