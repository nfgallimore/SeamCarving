all:
	g++ seam_carver.cpp -o seamcarve -std=c++11

clean:
	rm -rf build seamcarve